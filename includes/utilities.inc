<?php


/**
 * @file
 * Contains utility functions.
 */

/**
 * Removes, replaces or adds XML elements from an XML string.
 *
 * Replace will add if the element is not found.
 *
 * @param $patch mixed
 *   Array of changes of the format.
 *   $change = array(
 *     'ns' => array('prefix' => 'uri'),
 *     'changes' => array(
 *        'replace' => array('old xpath', 'parent xpath', 'new xml'),
 *        'remove' => array('old xpath'),
 *        'add' => array('parent xpath', 'new xml'),
 *     ),
 *   );
 *
 * @param $original_xml string
 *   The original XML string.
 *
 * @return string
 *   The altered XML.
 */
function manidora_xml_patcher($patch, $original_xml) {

  $mods = new DOMDocument();
  $mods->loadXML($original_xml);
  if (!$mods) {
    watchdog('Manidora', 'manidora_mods_patcher failed to parse XML');
    return FALSE;
  }

  $xpath = new DOMXpath($mods);
  $namespaces = array();

  if (isset($patch['ns']) && is_array($patch['ns'])) {
    foreach ($patch['ns'] as $ns => $uri) {
      $xpath->registerNamespace($ns, $uri);
      $namespaces[$ns] = $uri;
    }
  }
  foreach ($patch['changes'] as $k => $p) {
    if (!is_int($k)) {
      __manidora_xml_patcher_iterateGrouped($mods, $xpath, $k, $p, $namespaces);
    }
    else {
      foreach ($p as $type => $change) {
        __manidora_xml_patcher_makeXMLChange($mods, $xpath, $type, $change, $namespaces);
      }
    }
  }

  return $mods->saveXML();
}

/**
 * Sometimes the add keys are merged and moved up a level.
 *
 * @param $mods DOMDocument
 *   the XML to alter
 * @param $xpath DOMXPath
 *   the namespace enhanced DOMXpath
 * @param $type string
 *   type of XML change [add, remove, replace]
 * @param $changes mixed
 *   array of the change details
 * @param $namespaces array
 *   array of namespaces (prefix => URI)
 */
function __manidora_xml_patcher_iterateGrouped(&$mods, $xpath, $type, $changes, $namespaces) {
  foreach ($changes as $change) {
    __manidora_xml_patcher_makeXMLChange($mods, $xpath, $type, $change, $namespaces);
  }
}

/**
 * Make the actual change to the XML document
 *
 * @param $mods DOMDocument
 *   The XML to alter
 * @param $xpath DOMXPath
 *   The DOMXPath object with any needed namespaces.
 * @param $type string
 *   type of XML change [add, remove, replace]
 * @param $changes mixed
 *   array of the change details
 * @param $namespaces array
 *   array of namespaces (prefix => URI)
 */
function __manidora_xml_patcher_makeXMLChange(&$mods, $xpath, $type, $change, $namespaces) {
  switch ($type) {
    case 'replace':
      list($remove, $parent, $add) = $change;
      $hits = $xpath->query($remove);
      if (is_array($hits) && count($hits) > 0) {
        $add = __manidora_xml_patcher_insert_namespace($add, $namespaces);
        foreach ($hits as $hit) {
          $parent = $hit->parentNode;
          $replace = dom_import_simplexml(simplexml_load_string($add));
          $replace = $mods->importNode($replace, TRUE);
          $parent->replaceChild($replace, $hit);
        }
      }
      else {
        // No element found to replace, instead add to the parent.
        __manidora_xml_patcher_makeXMLChange($mods, $xpath, 'add', array($parent, $add), $namespaces);
      }
      break;

    case 'add':
      list($parent, $add) = $change;
      $add = __manidora_xml_patcher_insert_namespace($add, $namespaces);
      $hits = $xpath->query($parent);
      foreach ($hits as $hit) {
        $new_node = dom_import_simplexml(simplexml_load_string($add));
        $new_node = $mods->importNode($new_node, TRUE);
        $hit->appendChild($new_node);
      }
      break;

    case 'remove':
      $hits = $xpath->query($change);
      $dom_to_remove = array();
      foreach ($hits as $hit) {
        $dom_to_remove[] = $hit;
      }
      foreach ($dom_to_remove as $hit) {
        $hit->parentNode->removeChild($hit);
      }
      break;

  }
}

/**
 * Insert namespace URI in element.
 *
 * Insert all namespaces not already defined in the XML
 * from the original XML document to ensure this XML
 * snippet will load.
 *
 * @param $element string
 *   XML snippet
 * @param $namespaces array
 *   array of prefix => namespace URI
 * @return string
 *   modified XML snippet
 */
function __manidora_xml_patcher_insert_namespace($element, $namespaces) {
  $pos = strpos($element, '>');
  if (strpos(substr($element, 0, $pos), '/') !== FALSE) {
    // We may have an empty element, parse to see if the
    // backslash is inside an attribute.
    for ($x = $pos; $x > 0; $x -= 1) {
      if (substr($element, $x, 1) == "/") {
        $pos = $x;
        break;
      }
      elseif (substr($element, $x, 1) == "\"" ||
        substr($element, $x, 1) == "'") {
          break;
      }
    }
  }
  $add_string = "";
  foreach ($namespaces as $prefix => $uri) {
    if (strpos($element, "xmlns:$prefix") === FALSE) {
      $add_string .= " xmlns:$prefix=\"$uri\"";
    }
  }
  return substr($element, 0, $pos) . $add_string . substr($element, $pos);
}

/**
 * Constructs the text used in the header of the PDF or page.
 *
 * @param string $pid
 *   A PID representing a Fedora object.
 *
 * @return string
 *   Formatted string for output display.
 */
function manidora_retrieve_header_text($pid) {
  $object = islandora_object_load($pid);
  if (isset($object['MODS']) && strlen($mods = $object['MODS']->content) > 0) {
    // $mods = $object['MODS']->content;
    $xml = new SimpleXMLElement($mods);

    $title = '';
    $date = '';
    $page = '';

    if (in_array('islandora:newspaperPageCModel', $object->models) && isset($xml->relatedItem)) {
      if (isset($xml->relatedItem->titleInfo)) {
        if (isset($xml->relatedItem->titleInfo->title)) {
          $title = (string) $xml->relatedItem->titleInfo->title;
        }
      }
    }

    if (empty($title) && isset($xml->titleInfo)) {
      if (isset($xml->titleInfo->title)) {
        $title = (string) $xml->titleInfo->title;
        $title = preg_replace('/:(.*)/', '', $title);
      }
    }
    if (empty($title)) {
      $title = t('Unknown');
    }

    if (isset($xml->relatedItem)) {
      if (isset($xml->relatedItem->part)) {
        if (isset($xml->relatedItem->part->date)) {
          $date = $xml->relatedItem->part->date;
        }
        if (isset($xml->relatedItem->part->extent)) {
          if (isset($xml->relatedItem->part->extent->start)) {
            $page = $xml->relatedItem->part->extent->start;
          }
        }
      }
    }

    if (empty($date)) {
      $date = t('Unknown');
    }

    if (empty($page)) {
      $page = t('Unknown');
    }
    $text = $title . ', ' . $date . ' (Page ' . $page . ')';
    return $text;
  }
  else {
    return FALSE;
  }
}

/**
 * Get an array of excluded PIDs.
 *
 * @param bool $children
 *    Whether to get objects that are excluded or their children are.
 * @return array
 *    Pids to be excluded
 */
function manidora_collection_get_excluded($children=False) {
  $query = 'select pid from {manidora_omit_collections} where exclude_object = 1';
  if ($children) {
    $query = 'select pid from {manidora_omit_collections} where exclude_children = 1';
  }
  $results = db_query($query);
  $pids = [];
  foreach ($results as $result) {
    $pids[] = $result->pid;
  }
  return $pids;
}
<?php


/**
 * @file
 * Contains utility functions.
 */

/**
 * Replaces or adds XML elements from an XML string.
 * 
 * @param $patch mixed
 *   Array of changes of the format.
 *   $change = array(
 *     'ns' => array('prefix' => 'uri'), 
 *     'changes' => array(
 *        'replace' => array('old xpath', 'new xml'),
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

  $mods = DOMDocument::loadXML($original_xml);
  if (!$mods) {
    watchdog('Manidora', 'manidora_mods_patcher failed to parse XML');
    return FALSE;
  }

  $xpath = new DOMXpath($mods);

  if (isset($patch['ns']) && is_array($patch['ns'])) {
    foreach ($patch['ns'] as $ns => $uri) {
      $xpath->registerNamespace($ns, $uri);
    }
  }
  foreach ($patch['changes'] as $k => $p) {
    if (!is_int($k)) {
      __iterateGrouped($mods, $xpath, $k, $p);
    }
    else {
      foreach ($p as $type => $change) {
        __makeXMLChange($mods, $xpath, $type, $change);
      }
    }
  }

  return $mods->saveXML();
}

/**
 * Sometimes the add keys are merged and moved up a level.
 *
 * @param $type string
 *   type of XML change [add, remove, replace]
 * @param $changes mixed
 *   array of the change details
 */
function __iterateGrouped(&$mods, $xpath, $type, $changes) {
  foreach ($changes as $change) {
    __makeXMLChange($mods, $xpath, $type, $change);
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
 */
function __makeXMLChange(&$mods, $xpath, $type, $change) {
  switch ($type) {
    case 'replace':
      list($remove, $add) = $change;
      $hits = $xpath->query($remove);
      foreach ($hits as $hit) {
        $parent = $hit->parentNode;
        $replace = dom_import_simplexml(simplexml_load_string($add));
        $replace = $mods->importNode($replace, TRUE);
        $parent->replaceChild($hit, $replace);
      }
      break;

    case 'add':
      list($parent, $add) = $change;
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
<?php
/**
 * @file
 * Metadata related functions and forms
 */

/**
 * Fetches MODS metadata as form encoded markup.
 *
 * @param AbstractObject $object
 *   The object you wish to retrive formatted metadata from
 */
function manidora_retrieve_metadata_form(AbstractObject $object) {
  drupal_set_title($object->label . " Metadata");
  module_load_include('inc', 'islandora', 'includes/breadcrumb');
  drupal_set_breadcrumb(islandora_get_breadcrumbs($object));
  $output = '';
  $form = array();
  // Video and audio objects get PBCORE.
  if (array_intersect(array('islandora:sp_videoCModel', 'islandora:sp-audioCModel'), $object->models) && isset($object['PBCORE'])) {
    // $output = manidora_pbcore_metadata($object);
    $output = manidora_pbcore_transform($object);
  }
  // Grab MODS otherwise.
  else {
    $output = manidora_mods_transform($object);
  }
  if (!empty($output)) {
    $form = array(
      '#type' => 'item',
      '#markup' => $output,
      '#weight' => 10,
    );
  }
  else {
    drupal_set_message(t('A transformation error has occured!'), 'error');
  }
  return $form;
}

/**
 * Utility function that runs an xslt on MODS metadata.
 *
 * The transform was straight jacked from U of M's D6 site.
 *
 * @param IslandoraObject $object
 *   The IslandoraObject whose MODS metadata you wish to transform.
 */
function manidora_mods_transform($object) {
  // To get the islandora functions.
  module_load_include('inc', 'islandora', 'includes/utilities');

  $mods_text = $object->repository->api->a->getDatastreamDissemination($object->id, 'MODS');
  $parent_collections = islandora_get_parents_from_rels_ext($object);

  if ($parent_collections) {
    $collections = "";
    foreach ($parent_collections as $c) {
      $collections .= l($c->label, 'islandora/object/' . $c->id) . "<br />";
    }
  }

  if ($mods_text) {
    if (in_array('islandora:bookCModel', $object->models)) {
      $rows = array();
      $mods_xml = new SimpleXMLElement($mods_text);

      $mods_xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');

      // Title info.
      $title_results = $mods_xml->xpath('/mods:mods/mods:titleInfo/mods:title');
      $title = (string) reset($title_results);

      if (!empty($title)) {
        $rows[] = array('Field' => t('Title'), 'Value' => $title);
      }

      if (!empty($collections)) {
        $rows[] = array(t('Collections'), $collections);
      }

      $description_results = $mods_xml->xpath('/mods:mods/mods:abstract');
      $description = (string) reset($description_results);

      if (!empty($description)) {
        $rows[] = array(t('Description'), $description);
      }

      $format_res = $mods_xml->xpath('/mods:mods/mods:typeOfResource');
      $format = (string) reset($format_res);
      if (!empty($format)) {
        $rows[] = array(t('Format'), $format);
      }

      $date_res = $mods_xml->xpath('/mods:mods/mods:subject/mods:temporal');
      $date = (string) reset($date_res);
      if (!empty($date)) {
        $rows[] = array(t('Date'), $date);
      }

      $subtitle_results = $mods_xml->xpath('/mods:mods/mods:titleInfo/mods:subTitle');
      $subtitle = (string) reset($subtitle_results);

      if (!empty($subtitle)) {
        $rows[] = array(t('Subtitle'), $subtitle);
      }

      $authors = array();

      // Authors.
      foreach ($mods_xml->name as $name_xml) {
        $name_parts = array();
        $role = (string) @$name_xml->role->roleTerm;
        if ($role) {
          if ($role == 'author') {
            foreach ($name_xml->namePart as $name_part) {
              if ((string) $name_part != '') {
                // Strip periods off the end.
                $np = (string) rtrim($name_part, '.');
                if ($name_part['type'] == 'given') {
                  $name_parts['first_name'] = (string) $name_part;
                }
                if ($name_part['type'] == 'family') {
                  $name_parts['last_name'] = $np;
                }
              }
            }
          }
          if (isset($names_parts['last_name']) && isset($name_parts['first_name'])) {
            $authors[] = $name_parts['first_name'] . ' ' . $name_parts['last_name'];
          }
        }
      }
      $author_markup = '';
      foreach ($authors as $author) {
        if (empty($author)) {
          $author_markup = $author;
        }
        else {
          $author_markup .= '<br/>' . $author;
        }
      }

      if (!empty($author_markup)) {
        $rows[] = array(t('Author'), $author_markup);
      }

      // Origin Info.
      $place_results = $mods_xml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
      $place = (string) reset($place_results);

      if (!empty($place)) {
        $rows[] = array(t('Place'), $place);
      }

      $publisher_results = $mods_xml->xpath('/mods:mods/mods:originInfo/mods:publisher');
      $publisher = (string) reset($publisher_results);

      if (!empty($publisher)) {
        $rows[] = array(t('Publisher'), $publisher);
      }

      $date_results = $mods_xml->xpath('/mods:mods/mods:originInfo/mods:dateIssued');
      $date = (string) reset($date_results);

      if (!empty($date)) {
        $date_format = new DateTime($date);
        $rows[] = array(t('Date Issued'), date_format($date_format, 'Y-m-d'));
      }
      // Physical Description.
      $physical_results = $mods_xml->xpath('/mods:mods/mods:physicalDescription/mods:extent');
      $physical = (string) reset($physical_results);

      if (!empty($physical)) {
        $rows[] = array(t('Physical Description'), $physical);
      }

      // Record Info.
      $record_id_results = $mods_xml->xpath('/mods:mods/mods:recordInfo/mods:recordIdentifier');
      $record_id = (string) reset($record_id_results);

      if (!empty($record_id)) {
        $rows[] = array(t('Record ID'), $record_id);
      }

      $content_source_results = $mods_xml->xpath('/mods:mods/mods:recordInfo/mods:recordContentSource');
      $content_source = (string) reset($content_source_results);

      if (!empty($content_source)) {
        $rows[] = array(t('Content Source'), $content_source);
      }

      $language_results = $mods_xml->xpath('/mods:mods/mods:recordInfo/mods:languageofCataloging');
      $language = (string) reset($language_results);

      if (!empty($language)) {
        $rows[] = array(t('Language'), $language);
      }

      // Identifier.
      $identifier_results = $mods_xml->xpath('/mods:mods/mods:identifier');
      $identifier = (string) reset($identifier_results);

      if (!empty($identifier)) {
        $rows[] = array(t('Identifier'), $identifier);
      }

      // Table markup.
      $headers = array('Field' => t('Field'), 'Value' => t('Value'));
      $markup = theme_table(array(
        'header' => $headers,
        'rows' => $rows,
        'attributes' => array(),
        'caption' => '',
        'colgroups' => array(),
        'sticky' => TRUE,
        'empty' => t('No MODS present!')));
      return $markup;
    }
    else {
      $xslt_processor = new XSLTProcessor();
      $xsl = new DOMDocument();
      $file = drupal_get_path('module', 'manidora') . '/xsl/manidora_mods2html.xsl';
      $xsl->load($file);
      $input = new DOMDocument();

      $did_load = $input->loadXML($mods_text);

      if ($did_load) {
        global $base_url;
        // Using PHP functions to inject the collections HTML into the XSL.
        $xslt_processor->registerPHPFunctions();
        $xslt_processor->importStylesheet($xsl);
        $param_array = array(
          'islandoraUrl' => $base_url,
        );
        if (isset($collections)) {
          $param_array['collections'] = '<span>' . $collections . '</span>';
        }
        $xslt_processor->setParameter('', $param_array);
        $mods_transformed = $xslt_processor->transformToXml($input);
        return manidora_print_insert_link('print/islandora/object/' . $object->id . "/manitoba_metadata") . $mods_transformed;
      }
      else {
        return FALSE;
      }
    }
  }
  else {
    return FALSE;
  }
}

/**
 * Display custom look to PB Core metadata.
 */
function manidora_pbcore_transform($object) {
  // To get the islandora functions.
  module_load_include('inc', 'islandora', 'includes/utilities');

  $pbcore_text = $object->repository->api->a->getDatastreamDissemination($object->id, 'PBCORE');
  $parent_collections = islandora_get_parents_from_rels_ext($object);

  if ($parent_collections) {
    $collections = "";
    foreach ($parent_collections as $c) {
      $collections .= l($c->label, 'islandora/object/' . $c->id) . "<br />";
    }
  }
  if ($pbcore_text) {
    $xslt_processor = new XSLTProcessor();
    $xsl = new DOMDocument();
    $file = drupal_get_path('module', 'manidora') . '/xsl/manidora_pbcore2html.xsl';
    $xsl->load($file);
    $input = new DOMDocument();

    $did_load = $input->loadXML($pbcore_text);

    if ($did_load) {
      global $base_url;
      // Using PHP functions to inject the collections HTML into the XSL.
      $xslt_processor->registerPHPFunctions();
      $xslt_processor->importStylesheet($xsl);
      $param_array = array(
        'islandoraUrl' => $base_url,
      );
      if (isset($collections)) {
        $param_array['collections'] = '<span>' . $collections . '</span>';
      }
      $xslt_processor->setParameter('', $param_array);
      $mods_transformed = $xslt_processor->transformToXml($input);
      return manidora_print_insert_link('print/islandora/object/' . $object->id . "/manitoba_metadata") . $mods_transformed;
    }
  }
  else {
    return FALSE;
  }
}


/**
 * Used to insert the Collections into the mods2html.xsl stylesheet.
 *
 * This is called from the mods2html.xsl XSLT and passed a string of HTML.
 */
function manidora_return_collection_nodeset($i) {
  $doc = new DOMDocument();
  $doc->loadXml($i);
  return $doc;
}

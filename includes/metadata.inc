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
  elseif (isset($object['MODS'])) {
    $output = manidora_mods_transform($object);
  }
  $form = array(
    '#type' => 'item',
    '#markup' => $output,
    '#weight' => 10,
  );
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
            'pid' => $object->id,
        );
        if (isset($collections)) {
        $param_array['collections'] = '<span>' . $collections . '</span>';
        }
        $xslt_processor->setParameter('', $param_array);
        $mods_transformed = $xslt_processor->transformToXml($input);
        return $mods_transformed;
    }
    else {
        return FALSE;
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
        'pid' => $object->id,
      );
      if (isset($collections)) {
        $param_array['collections'] = '<span>' . $collections . '</span>';
      }
      $xslt_processor->setParameter('', $param_array);
      $mods_transformed = $xslt_processor->transformToXml($input);
      return $mods_transformed;
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

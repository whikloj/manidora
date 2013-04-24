<?php

/**
 * @file
 * Book batch ingest script for University of Manitoba.
 */

/**
 * Implements hook_drush_command().
 */
function manidora_drush_command() {
  $items = array();

  $items['manidora-book-batch'] = array(
    'options' => array(
      'timeout' => array(
        'description' => 'The duration to run for (in seconds), before stopping. Default is to run until completion.',
        'value' => 'optional',
      ),
      'target' => array(
        'description' => 'The target directory for preprocessing.',
        'value' => 'optional',
      ),
    ),
    'drupal dependencies' => array(
      'islandora_book_batch',
      'manidora',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['manidora_generate_hq_jpegs'] = array(
      'callback' => 'manidora_derive_hq_jpeg_datastreams',
      'description' => 'Derives the HQ_JPG datastream for downloading pdfs of newspaper pages for those pages who missed having these derivatives created upon ingest.',
      'examples' => array(
        'drush -u 1 -l http://mysite manidora_derive_hq_jpegs'
      ),
      'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    );

  return $items;
}

/**
 * Drush command callback.
 *
 * Triggers the ingest process. Attempts to ingest, preprocess, and ingest, in
 * that order.
 */
function drush_manidora_book_batch() {
  $t = 'manidora_book_batch_timeout';
  $timeout = drush_get_option('timeout', NULL);
  timer_start($t);

  $ops = array(
    'timeout' => $timeout,
  );

  drush_invoke_process('@self', 'islandora_batch_ingest', array(), $ops);

  $remaining = ($timeout * 1000) - timer_read($t);

  if ($timeout === NULL || $remaining > 0) {
    drush_invoke_process('@self', 'islandora_book_batch_preprocess', array(), array(
      'type' => 'directory',
      'target' => drush_get_option('target', '/usr/local/fedora/staging'),
      'parent' => 'uofm:rarebooks',
      'directory_dedup' => TRUE,
      'wait_for_metadata' => TRUE,
      'email_admin' => TRUE,
      'create_pdfs' => TRUE,
    ));

    $remaining = ($timeout * 1000) - timer_read($t);

    if ($timeout === NULL || $remaining > 0) {
      if ($timeout !== NULL) {
        // Timeout is in seconds, remaining is in ms.
        $ops['timeout'] = $remaining / 1000;
      }

      drush_invoke_process('@self', 'islandora_batch_ingest', array(), $ops);
    }
  }

  timer_stop($t);
}

function manidora_derive_hq_jpeg_datastreams() {
  $obj = '<info:fedora/*/OBJ>';
  $q = <<<EOQ
SELECT ?obj
FROM <#ri>
WHERE {
  ?obj <fedora-model:hasModel> <info:fedora/islandora:newspaperPageCModel> ;
  <fedora-view:disseminates> [<fedora-view:disseminationType> $obj]
}
EOQ;

  $tuque = islandora_get_tuque_connection();
  foreach($tuque->repository->ri->sparqlQuery($q) as $result) {
    $pid = $result['obj']['value'];
    $object = islandora_object_load($pid);
    if (!isset($object['HQ_JPG'])) {
      manidora_islandora_newspaperPageCModel_islandora_object_ingested($object);
    }
  }
}

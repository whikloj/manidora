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
      'drush -u 1 -l http://mysite manidora_derive_hq_jpegs',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['manidora-wipe-hocr-doctypes'] = array(
    'options' => array(
      'before' => array(
        'description' => 'HOCR datastreams from before the given date with be transformed. If not provided, all HOCR streams will be processed.',
        'value' => 'optional',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'drupal dependencies' => array(
      'islandora_ocr',
      'islandora',
      'manidora',
    ),
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

/**
 * Drush command callback.
 *
 * Generate HQ_JPG derivatives.
 */
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
  foreach ($tuque->repository->ri->sparqlQuery($q) as $result) {
    $pid = $result['obj']['value'];
    $object = islandora_object_load($pid);
    if (!isset($object['HQ_JPG'])) {
      manidora_islandora_newspaperpagecmodel_islandora_object_ingested($object);
    }
  }
}

/**
 * Batch operation.
 */
function manidora_batch_wipe_hocr_doctypes($before_date, &$context) {
  if ($before_date) {
    $before_date_filter = "FILTER(?datastream_date < '$before_date'^^<http://www.w3.org/2001/XMLSchema#dateTime>)";
  }
  elseif (isset($context['results']['first_altered_date'])) {
    $before_date_filter = "FILTER(?datastream_date < '{$context['results']['first_altered_date']}'^^<http://www.w3.org/2001/XMLSchema#dateTime>)";
  }
  else {
    $before_date_filter = '';
  }

  $query = <<<EOQ
SELECT ?object ?datastream_date
WHERE {
  ?object <fedora-view:disseminates> ?datastream ;
          <fedora-model:state> <fedora-model:Active> .
  ?datastream <fedora-model:state> <fedora-model:Active> ;
              <fedora-view:disseminationType> <info:fedora/*/HOCR> ;
              <fedora-view:lastModifiedDate> ?datastream_date .
  $before_date_filter
}
ORDER BY ?datastream_date
LIMIT 50
EOQ;

  $connection = islandora_get_tuque_connection();

  $results = $connection->repository->ri->sparqlQuery($query);

  // If there were no results, we're finished.
  $context['finished'] = empty($results);

  $temp_file = drupal_tempnam('temporary://', 'hocr-dtd');
  foreach ($results as $result) {
    $object = islandora_object_load($result['object']['value']);
    $datastream = $object['HOCR'];
    $datastream->getContent($temp_file);
    HOCR::stripDoctypeFromFile($temp_file);
    $datastream->setContentFromFile($temp_file);
    if (!$before_date && !isset($context['results']['first_altered_date'])) {
      // The resource index calls it "lastModifiedDate"...  Looks like objects
      // call it "createdDate"...  Hurray inconsistencies!
      $context['results']['first_altered_date'] = (string) $datastream->createdDate;
    }
    $context['message'] = dt('Wiped DOCTYPE from HOCR in @pid. New lastModified/created date "@date".', array(
      '@pid' => $object->id,
      '@date' => (string) $datastream->createdDate,
    ));
  }
  file_unmanaged_delete($temp_file);
}

/**
 * Drush callback.
 */
function drush_manidora_wipe_hocr_doctypes() {
  $batch = array(
    'operations' => array(
      array(
        'manidora_batch_wipe_hocr_doctypes', array(
          drush_get_option('before', FALSE),
        ),
      ),
    ),
    'title' => dt('Wiping HOCR DOCTYPE declarations...'),
  );

  batch_set($batch);
  drush_backend_batch_process();
}

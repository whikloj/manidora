<?php

/**
 * @file
 * Custom drush commands for the University of Manitoba.
 *
 * Includes the book batch ingest script, generate high quality
 * jpegs, fix HOCR datastreams, and regenerate compound item
 * sequence numbers.
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
  $items['manidora-aggregate-compound-ocr'] = array(
    'options' => array(
      'before' => array(
        'description' => 'Compound parents from before the given date with be transformed. If not provided, all compound object will be processed.',
        'value' => 'optional',
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
    'drupal dependencies' => array(
      'islandora_compound_object',
      'islandora',
      'manidora',
    ),
  );
  $items['manidora_regenerate_sequence_nums'] = array(
    'aliases' => array('uom_compound_fix'),
    'callback' => 'manidora_regenerate_sequence_batch_setup',
    'description' => 'Regenerates isSequenceNumberOf<PID> for compound images from mods:identifier@local if it exists',
    'options' => array(
      'pid' => array(
        'description' => 'The compound image PID to run against. If not provided all islandora:compoundCModel objects are run',
        'value' => 'optional',
      ),
    ),
    'examples' => array(
      'drush -u 1 uom_compound_fix' => 'Run against all objects with islandora:compoundCModel type',
      'drush -u 1 uom_compound_fix --pid="uofm:1234"' => 'Run ONLY against uofm:1234',
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

/**
 * Batch operation.
 */
function manidora_batch_aggregate_compound_ocr($before_date, &$context) {
  if ($before_date) {
    $before_date_filter = "FILTER(!bound(?datastream) || ?datastream_date < '$before_date'^^<http://www.w3.org/2001/XMLSchema#dateTime>)";
  }
  elseif (isset($context['results']['first_altered_date'])) {
    $before_date_filter = "FILTER(!bound(?datastream) || ?datastream_date < '{$context['results']['first_altered_date']}'^^<http://www.w3.org/2001/XMLSchema#dateTime>)";
  }
  else {
    $before_date_filter = '';
  }
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
  $query = <<<EOQ
SELECT DISTINCT ?object ?datastream_date
WHERE {
  {
    ?subject <fedora-rels-ext:$rels_predicate> ?object .
  }
  UNION
  {
    ?object <fedora-model:hasModel> <info:fedora/islandora:compoundCModel> .
  }
  ?object <fedora-model:state> <fedora-model:Active> .
  OPTIONAL {
    ?object <fedora-view:disseminates> ?datastream .
    ?datastream <fedora-model:state> <fedora-model:Active> ;
                <fedora-view:disseminationType> <info:fedora/*/OCR> ;
                <fedora-view:lastModifiedDate> ?datastream_date .
  }
  $before_date_filter
}
ORDER BY ?datastream_date
LIMIT 10
EOQ;

  $connection = islandora_get_tuque_connection();

  $results = $connection->repository->ri->sparqlQuery($query);

  // If there were no results, we're finished.
  $context['finished'] = empty($results);

  module_load_include('inc', 'manidora', 'includes/compound_ocr_aggregation');
  foreach ($results as $result) {
    $object = islandora_object_load($result['object']['value']);
    manidora_aggregate_child_ocr($object);
    $datastream = $object['OCR'];
    if (!$before_date && !isset($context['results']['first_altered_date'])) {
      // The resource index calls it "lastModifiedDate"...  Looks like objects
      // call it "createdDate"...  Hurray inconsistencies!
      $context['results']['first_altered_date'] = (string) $datastream->createdDate;
    }
    $context['message'] = dt('Aggregated OCR in @pid.', array(
      '@pid' => $object->id,
    ));
  }
}

/**
 * Drush callback.
 */
function drush_manidora_aggregate_compound_ocr() {
  $batch = array(
    'operations' => array(
      array(
        'manidora_batch_aggregate_compound_ocr', array(
          drush_get_option('before', FALSE),
        ),
      ),
    ),
    'title' => dt('Aggregating OCR from compound children into parents...'),
  );

  batch_set($batch);
  drush_backend_batch_process();
}

/**
 * Start the process to regenerate compound item sequence numbers.
 */
function manidora_regenerate_sequence_batch_setup() {
  $pid = drush_get_option('pid', FALSE);

  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  if ($pid !== FALSE) {
    $results[]['object']['value'] = $pid;
  }
  else {
    $query = 'select $object $label from <#ri> where (
    $object <fedora-model:label> $label and
    $object <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:compoundCModel> and
    $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>)
    order by $label';
    $results = $connection->repository->ri->itqlQuery($query, 'unlimited');
  }
  $operations = array();
  foreach ($results as $compound_object) {
    array_push($operations, array("manidora_regenerate_sequence_set_rels_ext",
      array($compound_object['object']['value']),
      )
    );
  }

  $batch = array(
    'operations' => $operations,
    'finished' => 'manidora_regenerate_sequence_finished',
    'title' => t('Recreating Sequence Numbers for Compound Images'),
    'init_message' => t('Batch is starting.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Batch has encountered an error.'),
    'file' => drupal_get_path('module', 'manidora') . '/manidora.drush.inc',
  );
  batch_set($batch);
  $batch =& batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();
}

/**
 * Process the compound object and set the sequence number if neccessary.
 */
function manidora_regenerate_sequence_set_rels_ext($compound_object, &$context) {
  $context['message'] = 'Setting sequence numbers from MODS record for object: ' . $compound_object;
  $parts = islandora_compound_object_get_parts($compound_object);
  if (is_array($parts) && count($parts) > 0) {
    foreach ($parts as $part) {
      $good = TRUE;
      $escaped_pid = str_replace(':', '_', $compound_object);
      $child_object = islandora_object_load($part);
      $sequence = $child_object->relationships->get(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_pid");
      if (is_array($sequence) && count($sequence) == 1) {
        drush_log("Child ($part) of $compound_object already has a sequence number, skipping", "ok");
      }
      else {
        $mods_text = $child_object->repository->api->a->getDatastreamDissemination($child_object->id, 'MODS');
        if ($mods_text) {
          $mods_xml = new SimpleXMLElement($mods_text);
          $mods_xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');
          // Title info.
          $mods_id_res = $mods_xml->xpath('/mods:mods/mods:identifier[@type="local"]');
          $mods_id = (string) reset($mods_id_res);
          if (!empty($mods_id)) {
            $local_id = intval(substr($mods_id, strrpos($mods_id, '_') + 1));
            if ($local_id == 0) {
              drush_log("Object ($part) has sequence invalid sequence 0 parsed from ($mods_id), not setting", 'error');
            }
            else {
              drush_log("Setting sequence number for ($part) of ($compound_object) to ($local_id)", "ok");
              $child_object->relationships->add(ISLANDORA_RELS_EXT_URI, "isSequenceNumberOf$escaped_pid", $local_id, TRUE);
            }
          }
          else {
            // Missed one, need to report to check manually.
            $good = FALSE;
          }
        }
        else {
          // Missed one, need to report to check manually.
          $good = FALSE;
        }
      }
      if (!$good) {
        drush_log("Part ($part) of ($compound_object) has no local identifier", "warning");
      }
    }
  }
  else {
    drush_log("Object ($compound_object) is not a compound or has 0 children", 'warning');
  }

}

/**
 * Completed sequence number regeneration.
 */
function manidora_regenerate_sequence_finished($success, $results, $operations) {
  // Print finished message to user.
  drush_log('Finished updating compound object sequence numbers.', 'ok');
}

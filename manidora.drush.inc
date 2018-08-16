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
  $items['parse_collection_json'] = array(
    'aliases' => array('parse_collection_json'),
    'description' => 'parse a collections json to find orphan items',
    'drupal_dependencies' => array('islandora', 'manidora'),
    'options' => array(
      'json_file' => array(
        'description' => 'file containing json',
        'required' => TRUE,
      ),
      'patch_file' => array(
        'description' => 'patch file containing compounding info',
        'required' => TRUE,
      ),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_LOGIN,
  );
  $items['patch_items'] = array(
    'aliases' => array('patch_items'),
    'description' => 'patch items based on a file',
    'drupal_dependencies' => array('islandora', 'manidora'),
    'options' => array(
      'patch_file' => array(
        'description' => 'patch file containing compounding info',
        'required' => TRUE,
      ),
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
      'parent' => drush_get_option('parent', 'uofm:rarebooks'),
      'namespace' => 'uofm',
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
    $query = 'select $object $label where (
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

function drush_manidora_patch_items() {
  drupal_static_reset('islandora_get_tuque_connection');
  $connection = islandora_get_tuque_connection();
  #$connection = new IslandoraRepositoryConnection(FEDORA_URL,FEDORA_USER,FEDORA_PWD);
  #$connection->reuseConnection = TRUE;
  #$connection->repository = new FedoraRepository(new FedoraApi($connection), new SimpleCache());

  $patchfile = drush_get_option('patch_file');
  $patches = file_get_contents($patchfile);
  $patches = explode("\n",$patches);
  drush_log($patches, "ok");
  foreach ($patches as $patch) {
    $tok = explode("\t",$patch);
    //[0] should be the parent pid
    //[1] should be the child pid
    //[2] should be the sequence index
    //module_load_include('inc','islandora','includes/compound_ocr_aggregation');
    $parent = islandora_object_load($tok[0]);
    if ($parent !== false){
      $child = islandora_object_load($tok[1]);
      if ($child !== false){
        $seq = intval($tok[2]);
        $items = islandora_compound_object_get_parts($tok[0]);
        if (count($items) > $tok[2]){
          // re-indexing needed
          //grab the existing items
          //$parts = islandora_compound_object_get_parts($tok[0]);
          $escaped_pid = str_replace(':','_',$tok[0]);
          foreach ($items as $tmpitem){
            //check the seq number. < tok[2] ? ignore; > tok[2] ? increment that number
            $item = islandora_object_load($tmpitem);
            if ($item !== FALSE){
              //get the sequence number
              $seqnum = $item->relationships->get(ISLANDORA_RELS_EXT_URI,"isSequenceNumberOf$escaped_pid",NULL,NULL);
              $seqnumint = intval($seqnum);
              if ($seqnumint > $seq){
                $seqnumint = $seqnumint+1;
                $item->relationships->remove(ISLANDORA_RELS_EXT_URI,"isSequenceNumberOf$escaped_pid",NULL,NULL);
                $item->relationships->add(ISLANDORA_RELS_EXT_URI,"isSequenceNumberOf$escaped_pid",$seqnumint,RELS_TYPE_PLAIN_LITERAL);
                $item->relationships->remove(FEDORA_RELS_EXT_URI,"isConstituentOf",NULL,NULL);
                $item->relationships->add(FEDORA_RELS_EXT_URI,"isConstituentOf",$tok[0]);
              }
            }
          }
        }
        //adding items onto the end of a sequence
        $escaped_pid = str_replace(':','_',$tok[0]);
        $child->relationships->remove(ISLANDORA_RELS_EXT_URI,"isSequenceNumberOf$escaped_pid",NULL,NULL);
        $child->relationships->add(ISLANDORA_RELS_EXT_URI,"isSequenceNumberOf$escaped_pid",$tok[2],RELS_TYPE_PLAIN_LITERAL);
        $child->relationships->remove(FEDORA_RELS_EXT_URI,"isConstituentOf",NULL,NULL);
        $child->relationships->add(FEDORA_RELS_EXT_URI,"isConstituentOf",$tok[0]);
      }
    }
  }
}


function drush_manidora_parse_collection_json() {
  $jsonfilename = drush_get_option('json_file');
  $patchfile = drush_get_option('patch_file');
  // read in the file, convert the utf8 (as apparently it "works better"), convert to json arrays
  $jsonfile = file_get_contents($jsonfilename);
  $jsonfile = utf8_encode($jsonfile);
  $results = json_decode($jsonfile,TRUE);
  // sort the array of objects based on the local identifier
  usort( $results, function( $a, $b ) {
    if (strnatcmp(trim($a["local_id"]), trim($b["local_id"])) === 0) {
      return strnatcmp($a['pid'], $b['pid']);
    }
    return strnatcmp( trim($a["local_id"]), trim($b["local_id"]) );

  });

  // Pull last 4 digits off the local identifier
  $get_sequence = function($o) {
    return substr($o,-4);
  };
  // Get everything except the underscore and last 4 digits
  $get_main_identifier = function($o) {
    return substr($o,0,-5);
  };
  $is_islandora_cmodel = function($o, $v) {
    return (strpos($v, 'islandora:') === 0);
  };

  /* process the items, so forth:
  //
  // *_0001, no *_0002 - no sequence, continue
  // *_0001, *_0002, same parent - everything is as expected
  // *_0001, *_0002, different parent - log as the 0002 will have to be orphaned and compounded
  // *_0001, *_0002, 0002 no parent - add to file that will control next script to auto compound
     */
  $file = fopen($patchfile,"w");

  if ($file === FALSE) {
    drush_log("Unable to open patch file $patchfile", "error");
  }
  else {
    for ($cnt = 0; $cnt < sizeof($results) - 1; $cnt++) {

      $first = $results[$cnt];
      $second = $results[$cnt + 1];

      drush_log($first["local_id"] . " vs " . $second["local_id"], "ok");

      if ($first["local_id"] == $second["local_id"]) {
        drush_log(format_string("models\n!a\n ---\n!b", array(
          '!a' => var_export($first['models'], TRUE),
          '!b' => var_export($second['models'], TRUE),
        )), "ok");
        $models_1 = array_filter($first['models'], $is_islandora_cmodel);
        $models_2 = array_filter($second['models'], $is_islandora_cmodel);
        drush_log(format_string("models\n!a\n ---\n!b", array(
          '!a' => var_export($models_1, TRUE),
          '!b' => var_export($models_2, TRUE),
        )), "ok");

        if (count(array_diff($models_1, $models_2)) === 0 &&
          count(array_diff($models_2, $models_1)) === 0) {
          drush_log(format_string("\t!pid1 has same local id as !pid2", array(
            '!pid1' => $first['pid'],
            '!pid2' => $second['pid'],
          )), "ok");
        }
        continue;
      }

      $parent1 = $first["relationships"]["isConstituentOf"];
      $parent2 = $second["relationships"]["isConstituentOf"];
      $id = call_user_func($get_sequence, $first["local_id"]);
      $itempre = call_user_func($get_main_identifier, $first["local_id"]);
      $itempost = $id;
      $nxtitempre = call_user_func($get_main_identifier, $second["local_id"]);
      $nxtitempost = call_user_func($get_sequence, $second["local_id"]);

      if (strcmp($itempre, $nxtitempre) == 0) {
        $cur = intval($nxtitempost);
        $nxt = intval($itempost) + 1;
        if ($nxt >= $cur) {
          //expected subsequent id
          /* check if the 2 items have the same parent(s) */
          for ($relcnt = 0; $relcnt < sizeof($parent1); $relcnt++) {
            if (sizeof($parent2) > 0) {
              $found = array_search($parent1[$relcnt], $parent2);
              //drush_log($results[$cnt]["local_id"]." ".$results[$cnt+1]["local_id"],"ok");
              if ($found === FALSE) {
                // items dont share a parent, check prefix
                drush_log("        adding " . $results[$cnt + 1]["local_id"] . " " . $results[$cnt + 1]["pid"], "ok");
              }
            }
            else {
              //check if item is a compound or not
              if ($nxtitempost != "0001") {
                drush_log("\tAdding " . $results[$cnt + 1]["local_id"] . " " . $results[$cnt + 1]["pid"] . " to " . $parent1[0], "ok");
                //add to patch file
                //format
                //[parent_pid]    [child_pid]     [sequence#]

                if ($file !== FALSE) {
                  $cmpitem = $parent1[0];
                  $patchstr = sprintf("%s\t%s\t%s\r\n", $cmpitem, $results[$cnt + 1]["pid"], intval($nxtitempost));
                  fwrite($file, $patchstr);
                }
              }
            }
          }
        }
      }
    }
    fclose($file);
  }
}

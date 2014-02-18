<?php

/**
 * @file
 * Functions pertaining to the aggregation of OCR in compounds.
 */

/**
 * Islandora derivative callback.
 *
 * Rebuild OCR in all compound parents of the current object.
 */
function manidora_aggregate_ocr_in_parents(AbstractObject $object, $force = FALSE) {
  // Get all compound parents.
  $rels_predicate = variable_get('islandora_compound_object_relationship', 'isConstituentOf');
  $parent_relationships = $object->relationships->get('info:fedora/fedora-system:def/relations-external#', $rels_predicate);

  $map_to_pid = function ($value) {
    return $value['object']['value'];
  };
  $parent_pids = array_map($map_to_pid, $parent_relationships);
  $parent_pids = array_unique($parent_pids);

  // For each parent, aggregate children.
  $output = array(
    'success' => TRUE,
    'messages' => array(),
  );
  foreach ($parent_pids as $parent_pid) {
    try {
      $parent = islandora_object_load($parent_pid);
      manidora_aggregate_child_ocr($parent);
      $output['messages'][] = array(
        'message' => t('Aggregated OCR in compound parent "@pid".'),
        'message_sub' => array(
          '@pid' => $parent_pid,
        ),
        'type' => 'dsm',
        'severity' => 'status',
      );
    }
    catch (Exception $e) {
      $output['success'] = FALSE;
      $output['messages'][] = array(
        'message' => t('Failed to aggregate OCR for compound parent "@pid"'),
        'message_sub' => array(
          '@pid' => $parent_pid,
        ),
        'type' => 'dsm',
        'severity' => 'error',
      );
    }
  }
  return $output;
}

/**
 * Gather OCR from all compound children of the current object.
 *
 * Might be made more efficient if we were to track which objects were already
 * accounted for in the OCR, instead of building it up from scratch everytime.
 * Would still have to go from scratch when deleting, though.
 *
 * @param AbstractObject $object
 *   An AbstractObject for which we will aggregate the OCR from its compound
 *   children.
 */
function manidora_aggregate_child_ocr(AbstractObject $object) {
  if (isset($object['OCR'])) {
    $datastream = $object['OCR'];
  }
  else {
    $datastream = $object->constructDatastream('OCR', 'M');
    $datastream->mimeType = 'text/plain';
    $datastream->label = 'Aggregated child OCR';
  }

  $datastream->relationships->autoCommit = FALSE;
  $datastream->relationships->remove(ISLANDORA_RELS_INT_URI, 'manidora-aggregated-from');

  // Get children.
  $child_pids = islandora_compound_object_get_parts($object->id);

  // For each child, append OCR to temp file.
  $temp_file = drupal_tempnam('temporary://', 'compound_ocr');

  // XXX: The file has to contain at least one character, as the
  // modifyDatastream fails to set the datastream to be empty when an empty file is used.
  file_put_contents($temp_file, " ");
  foreach ($child_pids as $child_pid) {
    $child_object = islandora_object_load($child_pid);
    if (!isset($child_object['OCR'])) {
      continue;
    }

    file_put_contents($temp_file, $child_object['OCR']->content . "\n", FILE_APPEND);
    $datastream->relationships->add(ISLANDORA_RELS_INT_URI, 'manidora-aggregated-from', $child_object->id . '/OCR');
  }
  $datastream->relationships->commitRelationships();

  // Add file of aggregated OCR to object.
  $datastream->setContentFromFile($temp_file);

  if (!isset($object['OCR'])) {
    $object->ingestDatastream($datastream);
  }
  file_unmanaged_delete($temp_file);
}

/**
 * Regenerate any aggregated OCR where content from this PID might be located.
 *
 * @param string $pid
 *   A string indicating a PID, which may not (and in fact likely does not)
 *   actually exist in Fedora... It's probably just been purged, so we're gonna
 *   use our funky RELS-INT to track down stuff which was using it.
 */
function manidora_scrub_aggregated_ocr($pid) {
  $tuque = islandora_get_tuque_connection();

  $rels_int = ISLANDORA_RELS_INT_URI;
  $query = <<<EOQ
PREFIX islandora-rels-int: <$rels_int>
SELECT DISTINCT ?obj
WHERE {
  ?obj <fedora-view:disseminates> ?datastream .
  ?datastream <fedora-view:disseminationType> <info:fedora/*/OCR> ;
              islandora-rels-int:manidora-aggregated-from <info:fedora/$pid/OCR> .
}
EOQ;
  $results = $tuque->repository->ri->sparqlQuery($query);

  $map_to_pids = function ($value) {
    return $value['obj']['value'];
  };
  $pids = array_map($map_to_pids, $results);
  $objects = array_filter(array_map('islandora_object_load', $pids));
  array_map('manidora_aggregate_child_ocr', $objects);
}

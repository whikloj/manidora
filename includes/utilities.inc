<?php

/**
 * @file
 * Miscellaneous helper functions.
 */

/**
 * Get pids of all sub-collections of supplied collection pid.
 *
 * @param string $collection_pid
 *   pid of ancestor collection
 */
function manidora_get_descendant_collections($collection_pid) {
  $tuque = islandora_get_tuque_connection();

  $dirs_to_process = array($collection_pid);
  $dirs_to_search = array($collection_pid);

  while (count($dirs_to_process) > 0) {
    $where = manidora_build_query($dirs_to_process);
    $dirs_to_process = array();
    $ri_query = "select \$pid
                 from <#ri>
                 $where
                   and  \$pid <fedora-model:state> <info:fedora/fedora-system:def/model#Active>
                   and (\$pid <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:collectionCModel>
                        or \$pid <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:newspaperCModel>
                        or \$pid <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:newspaperIssueCModel>
                        or \$pid <info:fedora/fedora-system:def/model#hasModel> <info:fedora/islandora:bookCModel>)";
    $objects = $tuque->repository->ri->itqlQuery($ri_query, 'unlimited');
    foreach ($objects as $object) {
      $dirs_to_search[] = $object['pid']['value'];
      $dirs_to_process[] = $object['pid']['value'];
    }
  }
  return $dirs_to_search;
}

/**
 * Builds itql string from collection pids.
 *
 * @param array $collection_pids
 *   Collection PID.
 *
 * @return string
 *   Query string.
 */
function manidora_build_query($collection_pids) {
  $first_pid = array_shift($collection_pids);
  $string = "\$pid  <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/$first_pid>
             or \$pid  <info:fedora/fedora-system:def/relations-external#isMemberOf> <info:fedora/$first_pid>";
  foreach ($collection_pids as $collection_pid) {
    $string .= " or \$pid  <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/$collection_pid>";
    $string .= " or \$pid  <info:fedora/fedora-system:def/relations-external#isMemberOf> <info:fedora/$collection_pid>";
  }
  $where_string = "where($string)";
  return $where_string;
}

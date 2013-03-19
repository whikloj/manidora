<?php
/**
 * Get pids of all sub-collections of supplied collection pid
 * @param string $collection_pid
 *  pid of ancestor collection
 *
 */
function manidora_get_descendant_collections($collection_pid) {
  $fedora_object = islandora_object_load('islandora:root');

  $dirs_to_process = array($collection_pid);
  $dirs_to_search = array($collection_pid);


  while (count($dirs_to_process) > 0) {
    $where = manidora_build_query($dirs_to_process);
    $dirs_to_process = array();
    $ri_query = "select \$pid from <#ri>
  $where
    and  \$pid <fedora-model:state> <info:fedora/fedora-system:def/model#Active>";
    $objects = $fedora_object->repository->ri->itqlQuery($ri_query, 'unlimited'); // for itql
    foreach ($objects as $object) {
      $dirs_to_search[] = $object['pid']['value'];
      $dirs_to_process[] = $object['pid']['value'];
    }
  }
  return $dirs_to_search;
}

/**
 * Builds itql string from collection pids
 * @param array $collection_pids
 * @return string
 */
function manidora_build_query($collection_pids) {
  $first_pid = array_shift($collection_pids);
  $string = "\$pid  <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/$first_pid>";
  while (count($collection_pids) > 0) {
    $next_pid = array_shift($collection_pids);
    $string .= " or \$pid  <info:fedora/fedora-system:def/relations-external#isMemberOfCollection> <info:fedora/$first_pid>";
  }
  $where_string = "where($string)";
  return $where_string;
}
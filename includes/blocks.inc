<?php

/**
 * @file
 * Contains funtions for blocks
 */

/**
 * Implements hook_block_info().
 */
function manidora_block_info() {
  $blocks['manidora_simple_search'] = array(
    'info' => t('Manidora simple search'),
  );
  return $blocks;
}

/**
 * Implements hook_block_view().
 */
function manidora_block_view($delta = '') {
  $block = array();
  switch ($delta) {
    case 'manidora_simple_search':
      $block['subject'] = t('Manidora simple search');
      $block['content'] = drupal_get_form('manidora_simple_search_form');
      break;
  }
  return $block;
}

/**
 * Manidora simple search form.
 *
 * Based on the Islandora Solr simple search block, this search box adds a
 * feature to search within the current collection or newspaper.
 */
function manidora_simple_search_form($form, &$form_state) {
  $params = $_GET;
  $hidden_filter = NULL;
  $rels_ext = array(
    'RELS_EXT_isMemberOfCollection_uri_ms',
    'RELS_EXT_isMemberOf_uri_ms',
  );
  // Find rels ext in parameters.
  if (isset($params['f'])) {
    foreach ($params['f'] as $filter) {
      $filter_arr = explode(':', $filter);
      foreach ($rels_ext as $rels) {
        if (strpos($filter_arr[0], $rels) === 0) {
          $hidden_filter = $filter;
          break;
        }
      }
    }
  }
  $options = array(
    0 => t('All collections'),
    1 => t('This collection'),
  );
  $form['simple'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array(
        'container-inline',
      ),
    ),
  );
  $form['simple']['hidden_filter'] = array(
    '#type' => 'hidden',
    '#value' => $hidden_filter,
  );
  if (!empty($hidden_filter)) {
    $form['simple']['collection_select'] = array(
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => (!empty($hidden_filter)) ? 1 : 0,
    );
  }
  $form['simple']["islandora_simple_search_query"] = array(
    '#size' => '15',
    '#type' => 'textfield',
    '#title' => '',
    // @TODO: should this be the searched value?
    '#default_value' => '',
  );
  $form['simple']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('search'),
  );
  return $form;
}

/**
 * Islandora Solr simple search form submit.
 */
function manidora_simple_search_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $search_string = $form_state['values']['islandora_simple_search_query'];
  // Replace the slash so url doesn't break.
  $search_string = str_replace('/', '~slsh~', $search_string);
  $hidden_filter = urldecode($form_state['values']['hidden_filter']);
  $collection_pid = str_replace('RELS_EXT_isMemberOfCollection_uri_ms:', '', $hidden_filter);
  $collection_pid = str_replace(array('"', "'"), '', $collection_pid);
  if (isset($form_state['values']['collection_select'])) {
    $collection_select = $form_state['values']['collection_select'];
  }

  // Using edismax by default for manidora.

  $query = array('type' => 'edismax');
  if (!empty($hidden_filter) && isset($collection_select) && $collection_select == 1) {
    $collection_pids = manidora_get_descendant_collections($collection_pid);
    $current_pid = array_shift($collection_pids);
    $pid_string = '"' . $current_pid . '"';
    $pid_string2 = '"info:fedora/' . $current_pid . '"';
    if (!empty($collection_pids)) {
      foreach ($collection_pids as $collection_pid) {
        $pid_string .= ' OR "' . $collection_pid . '"';
        $pid_string2 .= ' OR "info:fedora/' . $collection_pid . '"';
      }
    }
    $hidden_filter = "RELS_EXT_isMemberOfCollection_uri_ms:($pid_string)";
    $hidden_filter .= " OR RELS_EXT_isMemberOf_uri_ms:($pid_string2)";
    $query['hidden_filter'][] = $hidden_filter;
  }
  drupal_goto('islandora/search/' . $search_string, array('query' => $query));
}

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
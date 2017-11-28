<?php
/**
 * @file
 *
 * Administrative forms and functions
 */

/**
 * Admin form definition.
 */
function manidora_admin($form, &$form_state) {
  $form = array();

  $form['manidora_collection_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Collection Solr Field'),
    '#description' => t("Holds the collection Solr field."),
    '#default_value' => variable_get('manidora_collection_solr_field', 'RELS_EXT_isMemberOfCollection_uri_ms'),
    '#size' => 30,
  );
  $form['manidora_secondary_collection_solr_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Secondary Membership Solr Field'),
    '#description' => t('A Solr field; assumed to be "@pid_structure".', array('@pid_structure' => 'info:fedora/<pid>')),
    '#default_value' => variable_get('manidora_secondary_collection_solr_field', 'RELS_EXT_isMemberOf_uri_ms'),
    '#size' => 30,
  );
  $form['manidora_name_role_search_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr field to search for names and roles'),
    '#description' => t('The advanced search contains a Name/Role option, this combines the text with a role and searches this field.'),
    '#default_value' => variable_get('manidora_name_role_search_field'),
    '#size' => 30,
    '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
  );
  $form['manidora_name_role_roles_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Solr field containing roles'),
    '#description' => t('The above search populates a list of roles, it is created by returning all unique values in this field.'),
    '#default_value' => variable_get('manidora_name_role_roles_field'),
    '#size' => 30,
    '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
  );

  return system_settings_form($form);
}

/**
 * Admin form for homepage thumbnails.
 */
function manidora_homepage_thumbnail_admin($form, &$form_state) {
  $form = array();
  $thumbnails = variable_get('manidora_thumbnail_pids', NULL);
  $headers = array(
    'pid' => 'PID',
    'image' => 'Preview',
    'opers' => 'Operations',
  );
  $options = array();
  if (!is_null($thumbnails) && strlen(trim($thumbnails)) > 0) {
    $thumb_array = explode('~', $thumbnails);
    foreach ($thumb_array as $uupid) {
      $pid = urldecode($uupid);
      $options[] = array(
        'pid' => $pid,
        'image' => '<img style="width:63px;height:63px;" src="' . url('islandora/object/' . $pid . '/datastream/TN/view') . '" alt="thumbnail for ' . $pid . '"/>',
        'opers' => l(t('Remove'), 'admin/islandora/manidora/homepage/' . $pid . '/remove'),
      );
    }
  }
  $form['manidora_homepage_thumbnails_add'] = array(
    '#type' => 'textfield',
    '#title' => t('Enter the new pid here'),
    '#description' => t('To add a new image to the homepage, add the pid here'),
    '#length' => 30,
    '#maxlength' => 255,
    '#default_value' => '',
  );
  $form['manidora_homepage_thumbnails_add_btn'] = array(
    '#type' => 'submit',
    '#value' => 'Add',
  );
  $form['manidora_homepage_title'] = array(
    '#markup' => '<p><b>' . t('Homepage images') . '</b></p>',
  );
  if (count($options) > 0) {
    $form['manidora_homepage_thumbnails'] = array(
      '#type' => 'tableselect',
      '#title' => t('Homepage images'),
      '#description' => t('Holds the pids to rotate on the homepage'),
      '#multiple' => TRUE,
      '#options' => $options,
      '#empty' => t('There are no PIDs currently displayed on the homepage.'),
      '#header' => $headers,
    );
  }
  else {
    $form['manidora_homepage_thumbnails'] = array(
      '#title' => t('Homepage images'),
      '#description' => t('Holds the pids to rotate on the homepage'),
      '#markup' => '<p>' . t('There are no images currently displayed on the homepage, add one below') . '</p>',
    );
  }

  return $form;
}

/**
 * Validate the PID entered to be added to the homepage.
 */
function manidora_homepage_thumbnail_admin_validate($form, &$form_state) {
  $thumbnails = variable_get('manidora_thumbnail_pids');
  $pids = array();
  if (!is_null($thumbnails)) {
    $pids = explode('~', $thumbnails);
  }
  $newpid = $form_state['values']['manidora_homepage_thumbnails_add'];
  if (strlen(trim($newpid)) > 0 && preg_match('/^\w+:\w+$/', $newpid)) {
    if (in_array(urlencode($newpid), $pids)) {
      form_error($form, t('This image is already included on the homepage'));
    }
    try {
      $fedora_object = islandora_object_load($newpid);
      if (!is_a($fedora_object, 'IslandoraFedoraObject')) {
        form_error($form, t('Cannot load this PID, please ensure it is in the Fedora Repository.'));
      }
      else {
        $tn = $fedora_object->getDatastream('TN');
        if (!$tn) {
          form_error($form, t('PID (%pid) does not have a TN datastream.'), array('%pid' => $newpid));
        }
      }
    }
    catch (Exception $e) {
      watchdog('Manidora', 'Homepage Thumbnail PID error (%e)', array('%e' => $e->getMessage));
      form_error($form, t('This PID seems invalid (%e)', array('%e' => $e->getMessage())));
    }
  }
  else {
    form_error($form, t('The PID must be of the correct format'));
  }
}
/**
 * Submit the Homepage Image addition.
 *
 * Implements hook_form_submit().
 */
function manidora_homepage_thumbnail_admin_submit($form, &$form_state) {
  $thumbnails = variable_get('manidora_thumbnail_pids', "");
  if (strlen($thumbnails) > 0) {
    $thumbnails .= '~';
  }
  $thumbnails .= urlencode($form_state['values']['manidora_homepage_thumbnails_add']);
  variable_set('manidora_thumbnail_pids', $thumbnails);
  drupal_set_message(t('PID added successfully'), 'status', FALSE);
}

/**
 * Remove PID from homepage collage.
 */
function manidora_homepage_thumbnail_remove($pid = "") {
  if (strlen($pid) > 0) {
    $thumbnails = variable_get('manidora_thumbnail_pids', "");
    if (strlen($thumbnails) > 0) {
      $thumbs = explode('~', $thumbnails);
      $new_thumbs = array_filter($thumbs, create_function('$v', '$pid = "' . $pid . '"; return ($v != urlencode($pid));'));
      $thumbnails = implode('~', $new_thumbs);
    }
    variable_set('manidora_thumbnail_pids', $thumbnails);
    drupal_set_message(t('Image %p removed', array('%p' => $pid)), 'status', FALSE);
  }
  drupal_goto('admin/islandora/manidora/homepage');
}

/**
 * Implements hook_form().
 *
 * Omitted collections admin form.
 */
function manidora_collection_list_form($form, &$form_state) {
  $header = array(
    'title' => array('data' => t('Title'), 'field' => 'title', 'sort' => 'asc'),
    'exclude_object' => array('data' => t('Exclude this object'), 'field' => 'exclude_object'),
    'exclude_children' => array('data' => t('Exclude children'), 'field' => 'exclude_children'),
    'actions' => array('data' => t('Actions')),
  );
  $query = db_select('manidora_omit_collections', 'n');
  $total = $query->countQuery()->execute()->fetchField();
  $nids = $query
    ->fields('n',array('pid', 'title', 'exclude_object', 'exclude_children'))
    ->extend('TableSort')
    ->orderByHeader($header)
    ->extend('PagerDefault')->limit(50)
    ->execute();

  $results = $nids->fetchAllAssoc('pid', PDO::FETCH_ASSOC);
  foreach ($results as $r) {
    $results[$r['pid']]['exclude_object'] = ($r['exclude_object'] == 1 ? 'Yes' : 'No');
    $results[$r['pid']]['exclude_children'] = ($r['exclude_children'] == 1 ? 'Yes' : 'No');
    $results[$r['pid']]['actions'] = theme('links', [
      'links' =>
      [
        [
          'title' => t('Edit'),
          'href' => format_string('admin/islandora/manidora/collections/!pid/edit', ['!pid' => $r['pid']]),
        ],
        [
          'title' => t('Remove'),
          'href' => format_string('admin/islandora/manidora/collections/!pid/delete', ['!pid' => $r['pid']]),
        ],
      ],
      'attributes' => ['class' => ['manidora-omit-collections-list']],
    ]);
  }
  $solr_field = (isset($form_state['values']['manidora_omit_collection_ancestor_field']) ?
    $form_state['values']['manidora_omit_collection_ancestor_field'] :
    variable_get('manidora_omit_collection_ancestor_field', 'RELS_EXT_isMemberOf_uri_ms'));
  $form['manidora_omit_collection_solr'] = [
    '#type' => 'fieldset',
    '#title' => t('Solr settings'),
    'manidora_omit_collection_ancestor_field' => [
      '#type' => 'textfield',
      '#title' => t('Solr ancestor field'),
      '#description' => t('Solr field which contains the ancestors of a record. ' .
        'This is used to determine if a collection is a sub-collection.'),
      '#length' => 30,
      '#default_value' => $solr_field,
      '#autocomplete_path' => 'islandora_solr/autocomplete_luke',
    ],
    'btn_save' => [
      '#type' => 'submit',
      '#value' => t('Save'),
    ],
  ];
  $form['manidora_add_collection'] = [
    '#type' => 'fieldset',
    '#title' => t('Add a collection'),
    '#description' => t('Add a collection to omit it or its children from the homepage collections tabs.'),
    'add_pid_text' => [
      '#type' => 'textfield',
      '#length' => 25,
      '#title' => t('Collection pid'),
      '#default_value' => (isset($form_state['values']['add_pid_text'])? $form_state['values']['add_pid_text'] : ""),
      '#autocomplete_path' => 'islandora/manidora/collection_autocomplete'
    ],
    'add_pid_title' => [
      '#type' => 'hidden',
      '#default_value' => '',
    ],
    'omission_options' => [
      '#type' => 'select',
      '#title' => t('Omission options'),
      '#description' => t('How/What to omit from the chosen collection.'),
      '#options' => [
        0 => t('The collection and all sub-collections'),
        1 => t('The collection only (show sub-collections)'),
        2 => t('Only all sub-collections of this collection.'),
      ],
      '#default_value' => (isset($form_state['values']['omission_options']) ? $form_state['values']['omission_options'] : 0),
    ],
    'add_pid_btn' => [
      '#type' => 'submit',
      '#value' => t('Add Collection'),
    ],
  ];
  $form['manidora_single_pids'] = array(
    '#type' => 'fieldset',
    '#title' => t('Omitted collections'),
    '#description' => t('Collections to NOT be displayed on the homepage.'),
    'collection_listing' => array(
      '#type' => 'tableselect',
      '#title' => t('Collections'),
      '#header' => $header,
      '#options' => $results,
      '#empty' => t('There are no collections omitted'),
    ),
    'pager' => array('#markup' => theme('pager')),
  );
  return $form;
}

/**
 * Implements hook_form_validate().
 *
 * Omitted collections admin form.
 */
function manidora_collection_list_form_validate($form, &$form_state) {
  if (isset($form_state['clicked_button']) && $form_state['clicked_button']['#value'] == 'Add Collection') {
    module_load_include('inc', 'islandora', 'includes/utilities');
    if (!isset($form_state['values']['add_pid_text']) || empty($form_state['values']['add_pid_text'])) {
      form_set_error('add_pid_text', 'You must enter the PID of a collection');
    }
    elseif (!islandora_is_valid_pid($form_state['values']['add_pid_text'])) {
      form_set_error('add_pid_text', 'PID is not valid');
    }
    else {
      $pid = $form_state['values']['add_pid_text'];
      $obj = islandora_object_load($pid);
      if (!$obj) {
        form_set_error('add_pid_text', 'Islandora object could not be loaded');
      }
      elseif (count(array_intersect($obj->models, ['islandora:collectionCModel'])) <= 0) {
        form_set_error('add_pid_text', 'You must enter the PID of a Collection.');
      }
      else {
        $result = db_query('SELECT m.pid from {manidora_omit_collections} m where m.pid = :pid', [':pid' => $pid]);
        if ($result) {
          foreach ($result as $res) {
            if ($res->pid == $pid) {
              form_set_error('add_pid_text', 'That PID is already in the omitted collections list. Please edit it in place.');
            }
          }
        }
        $solr = new IslandoraSolrQueryProcessor();
        $query = format_string("PID:\"!pid\"", ['!pid' => $pid]);
        $params = [
          'fl' => 'ancestors_ms',
        ];
        $solr->buildQuery($query, $params);
        $solr->solrParams['facet'] = $solr->solrParams['hl'] = 'false';
        try {
          $solr->executeQuery(FALSE);
          $solr_results = $solr->islandoraSolrResult['response']['objects'];
        }
        catch (Exception $e) {
          drupal_set_message(check_plain(t('Error searching Solr index')) . ' ' . $e->getMessage(), 'error');
          $solr_results = array();
        }
        if ($solr_results) {
          $solr_get = function($field, $result) {
            if (isset($result[$field])) {
              return $result[$field];
            }
            elseif (isset($result['solr_doc'][$field])) {
              return $result['solr_doc'][$field];
            }
            else {
              return '';
            }
          };
          foreach ($solr_results as $result) {
            $pid_list = $solr_get('ancestors_ms', $result);
          }
          $result = db_query('SELECT m.pid from {manidora_omit_collections} m where m.pid IN (:pid) AND m.exclude_children = 1', [':pid' => $pid_list]);
          if ($result) {
            foreach ($result as $res) {
              if (isset($res->pid)) {
                form_set_error('add_pid_text', 'This collection is already hidden by a parent collection.');
                break;
              }
            }
          }
        }
      }
      $form_state['values']['add_pid_title'] = $obj->label;
    }
  }
}

/**
 * Implements hook_form_submit().
 *
 * Omitted collections admin form.
 */
function manidora_collection_list_form_submit($form, &$form_state) {
  if (isset($form_state['clicked_button']) && $form_state['clicked_button']['#value'] == 'Add Collection') {
    $pid = $form_state['values']['add_pid_text'];
    $title = $form_state['values']['add_pid_title'];
    $option = $form_state['values']['omission_options'];

    $fields = ['pid' => $pid, 'title' => $title];
    if ($option == 1) {
      $fields['exclude_object'] = 1;
      $fields['exclude_children'] = 0;
    }
    elseif ($option == 2) {
      $fields['exclude_object'] = 0;
      $fields['exclude_children'] = 1;
    }
    else {
      $fields['exclude_object'] = 1;
      $fields['exclude_children'] = 1;
    }
    $res = db_insert('manidora_omit_collections')->fields($fields)->execute();
    drupal_set_message(format_string('Collection (@c) added to omit list.', ['@c' => $title], 'status'));
  }
  elseif (isset($form_state['clicked_button']) && $form_state['clicked_button']['#value'] == 'Save') {
    variable_set('manidora_omit_collection_ancestor_field', $form_state['values']['manidora_omit_collection_ancestor_field']);
    drupal_set_message('Solr settings saved.');
  }
}

/**
 * Autocomplete function to return collection names.
 *
 * @param $string
 *    The currently typed string.
 */
function manidora_autocomplete_collections($string) {
  $matches = [];
  $islandora_tuque = islandora_get_tuque_connection();

  // Build escapes as per:
  // - https://www.w3.org/TR/xmlschema-2/#dt-metac and
  // - https://www.w3.org/TR/xmlschema-2/#dt-cces1
  $meta = array(
    '\\',
    '.',
    '?',
    '*',
    '+',
    '{',
    '}',
    '(',
    ')',
    '[',
    ']',
    '|',
    '-',
    '^',
  );
  $escape_meta = function ($meta) {
    return "\\\\$meta";
  };
  $meta_replacements = drupal_map_assoc($meta, $escape_meta);

  $replacements = [
    '!model' => '<info:fedora/islandora:collectionCModel>',
    '!text' => str_replace(array_keys($meta_replacements), $meta_replacements, $string),
  ];

  $query = <<<EOQ
PREFIX fedora-model: <info:fedora/fedora-system:def/model#>
SELECT DISTINCT ?pid ?title
WHERE {
  ?pid fedora-model:hasModel !model ;
       fedora-model:label ?title .
       FILTER(regex(?title, '^!text'))
}
LIMIT 10
EOQ;
  $query = format_string($query, $replacements);
  $results = $islandora_tuque->repository->ri->sparqlQuery($query, 'unlimited');

  foreach ($results as $result) {
    $matches[$result['pid']['value']] = t('!title (!pid)', array(
      '!title' => $result['title']['value'],
      '!pid' => $result['pid']['value'],
    ));
  }
  drupal_json_output($matches);
}

/**
 * Implements hook_form().
 *
 * Form for altering options on an already omitted collection.
 */
function manidora_omit_collections_edit_form($form, &$form_state, $pid=NULL) {

  if (!isset($form_state['values']['manidora_edit_omit_collection']) && !is_null($pid)) {
    $query = db_query('SELECT m.pid, m.title, m.exclude_object, m.exclude_children from {manidora_omit_collections} m where m.pid = :pid', [':pid' => $pid]);
    $result = $query->fetchAssoc();
    if ($result) {
      $title = $result['title'];
      if ($result['exclude_object'] == 1 && $result['exclude_children'] == 0) {
        $omission_option = 1;
      }
      elseif ($result['exclude_object'] == 0 && $result['exclude_children'] == 1) {
        $omission_option = 2;
      }
      else {
        $omission_option = 0;
      }
    }
  }
  else {
    $pid = (isset($form_state['values']['manidora_edit_omit_collection']['pid']) ? $form_state['values']['manidora_edit_omit_collection']['pid'] :
      drupal_set_message('ERROR: We don\'t have object information, something went very wrong.', 'error'));
    $title = (isset($form_state['values']['manidora_edit_omit_collection']['title_holder']) ?
      $form_state['values']['manidora_edit_omit_collection']['title_holder'] : "");
    $omission_option = (isset($form_state['values']['manidora_edit_omit_collection']['omission_options']) ?
      $form_state['values']['manidora_edit_omit_collection']['omission_options'] : 0);
  }
  drupal_set_title(t('Edit settings for "@title"', ['@title' => $title]));
  $form['manidora_edit_omit_collection'] = [
    '#type' => 'container',
    '#tree' => TRUE,
    'pid' => [
      '#type' => 'hidden',
      '#value' => $pid,
    ],
    'title_holder' => [
      '#type' => 'hidden',
      '#value' => $title,
    ],
    'omission_options' => [
      '#type' => 'select',
      '#title' => t('Omission options'),
      '#description' => t('How/What to omit from the chosen collection.'),
      '#options' => [
        0 => t('The collection and all sub-collections'),
        1 => t('The collection only (show sub-collections)'),
        2 => t('Only all sub-collections of this collection.'),
      ],
      '#default_value' => $omission_option,
    ],
    'update_btn' => [
      '#type' => 'submit',
      '#value' => 'Update',
    ],
    'delete_btn' => [
      '#type' => 'submit',
      '#value' => 'Delete',
    ],
    'cancel_btn' => [
      '#type' => 'submit',
      '#value' => 'Cancel',
    ],
  ];
  return $form;
}

/**
 * Implements hook_form_submit().
 *
 * Form for altering options on an already omitted collection.
 */
function manidora_omit_collections_edit_form_submit($form, &$form_state) {
  if (isset($form_state['values']['manidora_edit_omit_collection']['pid'])) {
    $pid = $form_state['values']['manidora_edit_omit_collection']['pid'];
    if (isset($form_state['clicked_button']) && $form_state['clicked_button']['#value'] == "Cancel") {
      drupal_goto('admin/islandora/manidora/collections');
    }
    elseif (isset($form_state['clicked_button']) && $form_state['clicked_button']['#value'] == "Delete") {
      drupal_goto(format_string('/admin/islandora/manidora/collections/!pid/delete', ['!pid' => $pid]));
    }
    else {
      $option = $form_state['values']['manidora_edit_omit_collection']['omission_options'];
      $fields = [];
      if ($option == 1) {
        $fields['exclude_object'] = 1;
        $fields['exclude_children'] = 0;
      }
      elseif ($option == 2) {
        $fields['exclude_object'] = 0;
        $fields['exclude_children'] = 1;
      }
      else {
        $fields['exclude_object'] = 1;
        $fields['exclude_children'] = 1;
      }
      $num_updated = db_update('manidora_omit_collections')->fields($fields)->condition('pid', $pid)->execute();
      if ($num_updated == 1) {
        drupal_set_message("Updated collection settings.");
        drupal_goto('admin/islandora/manidora/collections');
      }
      else {
        watchdog('manidora', 'Expected 1 record updated (!pid), saw !num instead.',
          ['!pid' => $pid, '!num' => $num_updated], WATCHDOG_ALERT);
        drupal_set_message(format_string('Manidora error: Expected 1 record updated (!pid), saw !num instead.',
          ['!pid' => $pid, '!num' => $num_updated]), 'warning');
      }
    }
  }
}

/**
 * Implements hook_form().
 *
 * Remove collection from omit list confirmation form.
 */
function manidora_omit_collections_delete_form($form, &$form_state, $pid) {
  $query = db_query('SELECT m.title from {manidora_omit_collections} m where m.pid = :pid', [':pid' => $pid]);
  $result = $query->fetchAssoc();
  if ($result) {
    $title = $result['title'];
  }
  drupal_set_title(t('Remove collection "@coll"?', ['@coll' => $title]));
  $form['manidora_delete_omit_collections'] = [
    'pid' => [
      '#type' => 'hidden',
      '#value' => $pid,
    ],
    'title_holder' => [
      '#type' => 'hidden',
      '#value' => $title,
    ],
    'delete_btn' => [
      '#type' => 'submit',
      '#value' => 'Delete',
    ],
    'cancel_btn' => [
      '#type' => 'submit',
      '#value' => 'Cancel',
    ],
  ];
  return $form;
}

/**
 * Implements hook_form_submit().
 *
 * Remove collection from omit list confirmation form.
 */
function manidora_omit_collections_delete_form_submit($form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == 'Cancel') {
    drupal_goto('admin/islandora/manidora/collections');
  }
  else {
    $pid = $form_state['values']['pid'];
    $title = $form_state['values']['title_holder'];
    $num_deleted = db_delete('manidora_omit_collections')->condition('pid', $pid)->execute();
    if ($num_deleted == 1) {
      drupal_set_message(format_string("Removed collection \"@coll\".", ['@coll' => $title]));
      drupal_goto('admin/islandora/manidora/collections');
    }
    else {
      watchdog('manidora', 'Expected 1 record deleted (!pid), saw !num instead.',
        ['!pid' => $pid, '!num' => $num_deleted], WATCHDOG_ALERT);
      drupal_set_message(format_string('Manidora error: Expected 1 record deleted (!pid), saw !num instead.',
        ['!pid' => $pid, '!num' => $num_deleted]), 'warning');
    }
  }
}

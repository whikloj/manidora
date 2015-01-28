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

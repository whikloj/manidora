<?php

/**
 * @file
 * A module for testing out drupal stuffs.
 */

function manidora_creative_commons_form(&$form, &$form_state) {
  // Form options.
  $modification_options = array(
    0 => t('Yes'),
    1 => t('No'),
    2 => t('Yes, as long as others share alike'),
  );

  $countries = array(
    'international' => t('International'),
    'ar' => t('Argentina'),
    'au' => t('Australia'),
    'at' => t('Austria'),
    'be' => t('Belgium'),
    'br' => t('Brazil'),
    'bg' => t('Bulgaria'),
    'ca' => t('Canada'),
    'cl' => t('Chile'),
    'cn' => t('China Mainland'),
    'co' => t('Colombia'),
    'cr' => t('Costa Rica'),
    'hr' => t('Croatia'),
    'cz' => t('Czech Republic'),
    'dk' => t('Denmark'),
    'ec' => t('Ecuador'),
    'eg' => t('Egypt'),
    'ee' => t('Estonia'),
    'fi' => t('Finland'),
    'fr' => t('France'),
    'de' => t('Germany'),
    'gr' => t('Greece'),
    'gt' => t('Guatemala'),
    'hk' => t('Hong kong'),
    'hu' => t('Hungary'),
    'in' => t('India'),
    'ie' => t('Ireland'),
    'il' => t('Israel'),
    'it' => t('Italy'),
    'jp' => t('Japan'),
    'lu' => t('Luxembourg'),
    'mk' => t('Macedonia'),
    'my' => t('Malaysia'),
    'mt' => t('Malta'),
    'mx' => t('Mexico'),
    'nl' => t('Netherlands'),
    'nz' => t('New Zealand'),
    'no' => t('Norway'),
    'pe' => t('Peru'),
    'ph' => t('Philippines'),
    'pl' => t('Poland'),
    'pt' => t('Portugal'),
    'pr' => t('Puerto Rico'),
    'ro' => t('Romania'),
    'rs' => t('Serbia'),
    'sg' => t('Singapore'),
    'si' => t('Slovenia'),
    'za' => t('South Africa'),
    'kr' => t('South Korea'),
    'es' => t('Spain'),
    'se' => t('Sweden'),
    'ch' => t('Switzerland'),
    'tw' => t('Taiwan'),
    'th' => t('Thailand'),
    'uk' => t('UK: England & Wales'),
    'scotland' => t('Uk: Scotland'),
    'ug' => t('Uganda'),
    'us' => t('United states'),
    'vn' => t('Vietnam'),
  );

  $commercial_options = array(
    0 => t('Yes'),
    1 => t('No'),
  );

  // Form elements.
  $form['license_fieldset'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => FALSE,
    '#title' => t('License'),
  );

  $form['license_fieldset']['allow_modifications'] = array(
    '#type' => 'radios',
    '#title' => t('Allow modifications of your work?'),
    '#options' => $modification_options,
    '#default_value' => 0,
    '#ajax' => array(
      'wrapper' => 'license-output',
      'callback' => 'manidora_on_form_input_change',
    ),
  );

  $form['license_fieldset']['allow_commercial'] = array(
    '#type' => 'radios',
    '#title' => t('Allow commercial uses of your work?'),
    '#options' => $commercial_options,
    '#default_value' => 0,
    '#ajax' => array(
      'wrapper' => 'license-output',
      'callback' => 'manidora_on_form_input_change',
    ),
  );

  $form['license_fieldset']['license_jurisdiction'] = array(
    '#type' => 'select',
    '#title' => t('License Jurisdiction'),
    '#options' => $countries,
    '#ajax' => array(
      'wrapper' => 'license-output',
      'callback' => 'manidora_on_form_input_change',
    ),
  );

  if (isset($form_state['triggering_element'])) {
    $derivatives = '';

    if ($form_state['values']['license_fieldset']['allow_modifications'] == 0) {
      $derivatives = 'y';
    }
    elseif ($form_state['values']['license_fieldset']['allow_modifications'] == 1) {
      $derivatives = 'n';
    }
    elseif ($form_state['values']['license_fieldset']['allow_modifications'] == 2) {
      $derivatives = 'sa';
    }

    $commercial = '';

    if ($form_state['values']['license_fieldset']['allow_commercial'] == 0) {
      $commercial = 'y';
    }
    elseif ($form_state['values']['license_fieldset']['allow_commercial'] == 1) {
      $commercial = 'n';
    }

    $jurisdiction = $form_state['values']['license_fieldset']['license_jurisdiction'];
    $data = '&derivatives=' . $derivatives . 'commercial=' . $commercial . '&jurisdiction=' . $jurisdiction;
    $response = simplexml_load_file('http://api.creativecommons.org/rest/1.5/license/standard/get?commercial=' . $commercial . '&derivatives=' . $derivatives . '&jurisdiction=' . $jurisdiction);

    $license = '';
    $count = 1;
    foreach ($response->html->a as $a) {
      if ($count == 2) {
        $license = $a;
      }

      $count++;
    }

    $license_markup = '<strong>' . t('Selected license:') . '</strong> ' . $license;
  }

  $form['license_fieldset']['license_output'] = array(
    '#prefix' => '<div id="license-output">',
    '#markup' => isset($license_markup) ? $license_markup : '<strong>' . t('Selected license:') . '</strong> Attribution 3.0 Unported',
    '#suffix' => '</div>',
  );

  $form['next']['#submit'][] = array('manidora_creative_commons_form_submit');
  $form['next']['#weight'] = 100;
}

function manidora_on_form_input_change($form, &$form_state) {
  return $form['license_fieldset']['license_output'];
}

function manidora_creative_commons_form_submit($form, &$form_state) {
  dsm($form_state);
}
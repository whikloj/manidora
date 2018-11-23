<?php

class ManidoraContextReactionMusicCompound extends context_reaction {

  private $track_unavailable_label = 'This track is not available.';

  function options_form($context) {
    $settings = $this->fetch_from_context($context);
    $label = isset($settings['track_unavailable_label']) ? $settings['track_unavailable_label'] : '';
    return array(
      'track_unavailable_label' => array(
        '#type' => 'textfield',
        '#length' => 200,
        '#title' => t('Track not available message'),
        '#description' => t('Message to display if the audio/video OBJ is missing. Defaults to "This track is not available"'),
        '#default_value' => $label,
      ),
      'enabled' => array('#type' => 'value', '#value' => TRUE),
    );
  }

  function options_form_submit($values) {
    if (isset($values['track_unavailable_label']) &&
      empty($values['track_unavailable_label'])) {
      unset($values['track_unavailable_label']);
    }
    return $values;
  }

  function execute($object) {
    $markup = array();
    foreach ($this->get_contexts() as $context) {
      if ($context->reactions['manidora_context_reaction_alter_music']['enabled']) {
        if (isset($context->reactions['manidora_context_reaction_alter_music']['track_unavailable_label'])) {
          $this->track_unavailable_label = $context->reactions['manidora_context_reaction_alter_music']['track_unavailable_label'];
        }
        $parts = islandora_compound_object_get_parts($object);
        $markup['manidora_context_reaction_music_compound'] = array();
        foreach ($parts as $part) {
          $object = islandora_object_load($part);
          if (in_array('islandora:sp_pdf', $object->models)) {
            $markup['manidora_context_reaction_music_compound'][] = $this->makePdf($object);
          }
          elseif (in_array('islandora:sp-audioCModel', $object->models)) {
            $markup['manidora_context_reaction_music_compound'][] = $this->makeAudioPlayer($object);
          }
          elseif (in_array('islandora:sp_videoCModel', $object->models)) {
            $markup['manidora_context_reaction_music_compound'][] = $this->makeVideoPlayer($object);
          }
        }
      }
    }
    return $markup;
  }

  private function makePdf(AbstractObject $object) {
    module_load_include('inc', 'manidora', 'includes/metadata');
    $metadata = manidora_get_correct_metadata($object);

    $link = array(
      '#type' => 'link',
      '#title' => t('View Program'),
      '#href' => url(format_string('islandora/object/!pid/datastream/!dsid/content',
        array(
          '!pid' => $object->id,
          '!dsid' => 'OBJ',
        )), array('absolute' => TRUE)),
    );

    $admin = drupal_render($link);
    $this->makeAdminLink($object, $admin);

    return array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'manidora-music-compound-wrapper',
          'manidora-music-compounds-pdf-link',
        ),
      ),
      '#attached' => array('js' => array('misc/collapse.js', 'misc/form.js')),
      'program' => array(
        '#type' => 'html_tag',
        '#tag' => 'p',
        '#attributes' => array(
          'class' => array(
            'manidora-music-compound-pdf-label',
            'manidora-music-compound-label',
            ),
        ),
        '#value' => $admin,
      ),
      'details' => array(
        '#type' => 'fieldset',
        '#title' => t('Details'),
        '#attributes' => array('class' => array('collapsible', 'collapsed')),
        'content' => array(
          '#markup' => $metadata,
        ),
      ),
    );
  }

  private function makeAudioPlayer(AbstractObject $object) {
    module_load_include('inc', 'manidora', 'includes/metadata');
    $metadata = manidora_get_correct_metadata($object);
    // Possible DSIDs in order of preference
    $possible_dsids = array(
      'PROXY_MP3',
      'OBJ',
    );

    $pid = $object->id;
    $source = $this->getSources($object, $possible_dsids);

    $value = t($object->label);
    $this->makeAdminLink($object, $value);
    $form = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'manidora-music-compound-wrapper',
          'manidora-music-compound-audio-wrapper',
        ),
      ),
      '#attached' => array('js' => array('misc/collapse.js', 'misc/form.js')),
      'label' => array(
        '#type' => 'html_tag',
        '#tag' => 'p',
        '#attributes' => array(
          'class' => array(
            'manidora-music-compound-audio-label',
            'manidora-music-compound-label',
          ),
        ),
        '#value' => $value,
      ),
    );

    if (!empty($source)) {
      // theme_html_tag does not render children.
      $form['audio_player'] = array(
        '#type' => 'html_tag',
        '#tag' => 'audio',
        '#attributes' => array(
          'preload' => 'metadata',
          'controls' => 'controls',
          'id' => drupal_html_id($pid),
          'class' => array('manidora-context', 'manidora-music-compound-audio'),
          'controlsList' => 'nodownload',
        ),
        '#value' => $source . 'Your browser does not support the HTML 5 audio element.',
      );
    }
    else {
      $form['audio'] = array(
        '#type' => 'html_tag',
        '#tag' => 'p',
        '#attributes' => array(
          'class' => array('manidora-music-unavailable'),
        ),
        '#value' => t($this->track_unavailable_label),
      );
    }
    $form['details'] = array(
      '#type' => 'fieldset',
      '#title' => t('Details'),
      '#attributes' => array('class' => array('collapsible', 'collapsed')),
      'content' => array(
        '#markup' => $metadata,
      ),
    );
    return $form;
  }

  private function makeVideoPlayer(AbstractObject $object) {
    module_load_include('inc', 'manidora', 'includes/metadata');
    $metadata = manidora_get_correct_metadata($object);
    // Possible DSIDs in order of preference
    $possible_dsids = array(
      'MP4',
      'MKV',
      'OBJ',
    );

    $pid = $object->id;
    $source = $this->getSources($object, $possible_dsids);

    $value = t($object->label);
    $this->makeAdminLink($object, $value);
    $form = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'manidora-music-compound-wrapper',
          'manidora-music-compound-video-wrapper',
        ),
      ),
      '#attached' => array('js' => array('misc/collapse.js', 'misc/form.js')),
      'label' => array(
        '#type' => 'html_tag',
        '#tag' => 'p',
        '#attributes' => array(
          'class' => array(
            'manidora-music-compound-video-label',
            'manidora-music-compound-label',
          ),
        ),
        '#value' => $value,
      ),
    );

    if (!empty($source)) {
      // theme_html_tag does not render children.
      $form['video_player'] = array(
        '#type' => 'html_tag',
        '#tag' => 'video',
        '#attributes' => array(
          'preload' => 'metadata',
          'controls' => 'controls',
          'id' => drupal_html_id($pid),
          'class' => array(
            'manidora-context',
            'manidora-music-compound-video',
          ),
          'controlsList' => 'nodownload',
        ),
        '#value' => $source . 'Your browser does not support the HTML 5 video element.',
      );
    }
    else {
      $form['video'] = array(
        '#type' => 'html_tag',
        '#tag' => 'p',
        '#attributes' => array(
          'class' => array('manidora-music-unavailable'),
        ),
        '#value' => t($this->track_unavailable_label),
      );
    }

    $form['details'] = array(
      '#type' => 'fieldset',
      '#title' => t('Details'),
      '#attributes' => array('class' => array('collapsible', 'collapsed')),
      'content' => array(
        '#markup' => $metadata,
      ),
    );

    return $form;
  }

  /**
   * Make and add an admin link to the value string.
   *
   * @param \AbstractObject $object
   *   The object to administer.
   * @param $value
   *   The current label string

   * @throws \Exception
   *   When calling theme().
   */
  private function makeAdminLink(AbstractObject $object, &$value) {
    if (islandora_object_manage_access_callback(array(
      ISLANDORA_MANAGE_PROPERTIES,
      ISLANDORA_METADATA_EDIT,
      ISLANDORA_ADD_DS,
      ISLANDORA_PURGE,
      ISLANDORA_INGEST,
    ), $object)) {
      $value .= " " .
        theme('html_tag', array(
          'element' => array(
            '#type' => 'html_tag',
            '#tag' => 'span',
            '#attributes' => array(
              'class' => array('manidora-music-compound-admin-part'),
            ),
            '#value' => l(
              'Edit this object',
              format_string('islandora/object/!pid/manage', array(
                '!pid' => $object->id,
              ))
            ),
          )));
    }
  }

  /**
   * Get the any desired source datastreams and return rendered <source> tags.
   *
   * @param \AbstractObject $object
   *   The object.
   * @param array $valid_dsids
   *   The desired DSIDs
   *
   * @return string
   *   The <source> tags if any
   * @throws \Exception
   *   On theming.
   */
  private function getSources(AbstractObject $object, array $valid_dsids) {
    $sources = '';
    foreach ($valid_dsids as $dsid) {
      if (isset($object[$dsid])) {
        $url = url(format_string('islandora/object/!pid/datastream/!dsid/content',
          array(
            '!pid' => $object->id,
            '!dsid' => $dsid,
          )), array('absolute' => TRUE));
        $mimetype = $object[$dsid]->mimetype;
        $sources .= theme('html_tag', array(
          'element' => array(
            '#type' => 'html_tag',
            '#tag' => 'source',
            '#attributes' => array(
              'mimetype' => $mimetype,
              'src' => $url,
            ),
            '#value' => '',
          ),
        ));
      }
    }
    return $sources;
  }

}

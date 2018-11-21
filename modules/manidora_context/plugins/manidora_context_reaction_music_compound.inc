<?php

class ManidoraContextReactionMusicCompound extends context_reaction {

  function options_form($context) {
    return array('display_as_music_compound' => array('#type' => 'value', '#value' => TRUE));
  }

  function options_form_submit($values) {
    return array('display_as_music_compound' => 1);
  }

  function execute($object) {
    $markup = array();
    foreach ($this->get_contexts() as $context) {
      if ($context->reactions['manidora_context_reaction_alter_music']['display_as_music_compound']) {
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
    $admin = "";
    $this->makeAdminLink($object, $admin);
    return array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array(
          'manidora-music-compound-wrapper',
          'manidora-music-compounds-pdf-link',
        ),
      ),
      'program_link' => array(
        '#type' => 'link',
        '#title' => t('View Program'),
        '#href' => url(format_string('islandora/object/!pid/datastream/!dsid/content',
          array(
            '!pid' => $object->id,
            '!dsid' => 'OBJ',
          )), array('absolute' => TRUE)),
      ),
      'admin_link' => array(
        '#markup' => $admin,
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
    $url = NULL;
    foreach ($possible_dsids as $dsid) {
      if (isset($object[$dsid])) {
        $url = url(format_string('islandora/object/!pid/datastream/!dsid/content',
          array(
            '!pid' => $pid,
            '!dsid' => $dsid,
          )), array('absolute' => TRUE));
        $mimetype = $object[$dsid]->mimetype;
      }
    }

    if (!is_null($url)) {
      // theme_html_tag does not render children.
      $source = theme('html_tag', array(
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

      $value = t($object->label);
      $this->makeAdminLink($object, $value);

      return array(
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
            'class' => array('manidora-music-compound-audio-label'),
          ),
          '#value' => $value,
        ),
        'audio_player' => array(
          '#type' => 'html_tag',
          '#tag' => 'audio',
          '#attributes' => array(
            'controls' => 'controls',
            'id' => drupal_html_id($pid),
            'class' => array('manidora-context', 'manidora-music-compound-audio'),
            'controlsList' => 'nodownload',
          ),
          '#value' => $source . 'Your browser does not support the HTML 5 audio element.',
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
    else {
      return array();
    }
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
    $sources = array();
    foreach ($possible_dsids as $dsid) {
      if (isset($object[$dsid])) {
        if (!array_key_exists($object[$dsid]->mimetype, $sources)) {
          $sources[$object[$dsid]->mimetype] =  url(
            format_string('islandora/object/!pid/datastream/!dsid/content',
              array(
                '!pid' => $pid,
                '!dsid' => $dsid,
              )), array('absolute' => TRUE));
        }
      }
    }

    if (count($sources) > 0) {
      $output = array(
        '#type' => 'html_tag',
        '#tag' => 'video',
        '#attributes' => array(
          'controls' => 'controls',
          'id' => drupal_html_id($pid),
          'class' => array(
            'manidora-context',
            'manidora-music-compound-video',
          ),
          'controlsList' => 'nodownload',
        ),
        '#value' => 'Your browser does not support the HTML 5 video element.',
      );
      foreach ($sources as $mimetype => $url) {
        // theme_html_tag does not handle children, so render separately.
        $output['#value'] .= theme('html_tag', array(
          '#type' => 'html_tag',
          '#tag' => 'source',
          '#attributes' => array(
            'mimetype' => $mimetype,
            'src' => $url,
          ),
        ));
      }
      $value = t($object->label);
      $this->makeAdminLink($object, $value);
      return array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('manidora-music-compound-wrapper', 'manidora-music-compound-video-wrapper'),
        ),
        '#attached' => array('js' => array('misc/collapse.js', 'misc/form.js')),
        'label' => array(
          '#type' => 'html_tag',
          '#tag' => 'p',
          '#attributes' => array(
            'class' => array('manidora-music-compound-video-label'),
          ),
          '#value' => $value,
        ),
        'video_player' => $output,
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
    else {
      return array();
    }
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
}

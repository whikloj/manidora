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
    return array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => 'manidora-music-compounds-pdf-link',
      ),
      'program_link' => array(
        '#type' => 'link',
        '#title' => t('View Program'),
        '#href' => url(format_string('islandora/object/!pid/datastream/!dsid/content',
          array(
            '!pid' => $object->id,
            '!dsid' => 'OBJ',
          )), array('absolute' => TRUE)),
      )
    );
  }

  private function makeAudioPlayer(AbstractObject $object) {
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

      return array(
        '#type' => 'container',
        'label' => array(
          '#type' => 'html_tag',
          '#tag' => 'p',
          '#attributes' => array(
            'class' => array('manidora-music-compound-audio-label'),
          ),
          '#value' => t($object->label),
        ),
        'audio_player' => array(
          '#type' => 'html_tag',
          '#tag' => 'audio',
          '#attributes' => array(
            'controls' => 'controls',
            'id' => drupal_html_id($pid),
            'class' => array('manidora-context', 'manidora-music-compound-audio'),
          ),
          '#value' => $source . 'Your browser does not support the HTML 5 audio element.',
        ),
      );
    }
    else {
      return array();
    }
  }

  private function makeVideoPlayer(AbstractObject $object) {
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
          'class' => array('manidora-context', 'manidora-music-compound-video'),
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
      return $output;
    }
    else {
      return array();
    }
  }

}
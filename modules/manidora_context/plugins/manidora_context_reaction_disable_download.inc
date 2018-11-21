<?php


class ManidoraContextReactionDisableDownload extends context_reaction {

  function options_form($context) {
    return array('dont_display_download_tab' => array('#type' => 'value', '#value' => TRUE));
  }

  function options_form_submit($values) {
    return array('dont_display_download_tab' => 1);
  }

  function execute() {
    foreach ($this->get_contexts() as $context) {
      if ($context->reactions['manidora_context_reaction_alter_music']['display_as_music_compound']) {
        return False;
      }
    }
    return NULL;
  }
}
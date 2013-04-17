/**
 * @file
 * Integrate with Colorbox.
 */

(function ($) {
  Drupal.behaviors.manidora = {
    attach: function (context, settings) {
      $('.manidora-colorbox').colorbox({inline:true});
    }
  };
})(jQuery);

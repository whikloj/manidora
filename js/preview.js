/**
 * @file
 * Javascript function to move thumbnail on data input forms.
 */
jQuery(document).ready(function() {
  // setTimeout because if the image is not loaded it will mess up the following css calculation
  window.setTimeout(function(){
    jQuery('#block-manidora-manidora-preview-image').css({'right': 'auto', 'left': (jQuery(window).width() - jQuery('#block-manidora-manidora-preview-image').width() - 20) + 'px'}).draggable();
  }, 1000);
});
/**
 * @file
 * Javascript function for tracking Google Analytic clicks.
 */
var manidora_trackDownloadLink = function(ID) {
  if (typeof(ga) != undefined) {
    ga('send', 'event', 'download', 'click', { 'eventLabel' : ID, 'page' : manidora_trackdownload_page});
  }
}
/**
 * This is needed to track downloaded file types
 */
var manidora_trackDownloadLink = function(ID) {
  <?php print "var the-page = \"" . current_path() . "\";"; ?>
  ga('send', 'event', 'download', 'click', ID, { 'page' : the-page} );
}
<?php
/**
 * @file
 * Template for What's New items
 *
 *  $PID => object PID,
 *  $title => dc.title,
 *  $image => linked thumbnail,
 *  $link => linked title,
 *  $timestamp => DateTime added to Solr
 */
?>
<div class="manidora-whats-new-row">
  <div class="manidora-whats-new-thumb">
    <?php print $image; ?>
  </div>
  <div class="manidora-whats-new-info">
    <?php print $link; ?><br />
    <span class="manidora-whats-new-timestamp date"><?php print $timestamp->format('M j, Y'); ?></span>
  </div>
</div>

<?php
/**
 * @file
 * Hooks and things.
 */

/**
 * Alter access to the downloads tab, can only make more restrictive.
 *
 * @param \AbstractObject $object
 *   The Islandora Object to check.
 *
 * @return null|boolean
 *   Null if no choice, False to restrict, True is disregarded.
 */
function hook_manidora_download_access(AbstractObject $object) {
  // Restrict download for basic images
  if (in_array('islandora:sp_basic_image', $object->models)) {
    return FALSE;
  }
  return NULL;
}

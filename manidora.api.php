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

/**
 * Essentially for calling contexts.
 *
 * @param \AbstractObject $object
 *   The islandora object.
 */
function hook_manidora_detail_view(AbstractObject $object) {
  // Do something when you are viewing the Details tab for this object.
}

/**
 * Essentially for calling contexts conditions against $object.
 *
 * @param \AbstractObject $object
 *   The islandora object.
 */
function hook_manidora_download_view(AbstractObject $object) {
  // Do something when you are viewing the Download tab for this object.
}

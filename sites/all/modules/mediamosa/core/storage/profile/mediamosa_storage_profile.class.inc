<?php
/**
 * @file
 * storage_profile module.
 */

class mediamosa_storage_profile {
  // ------------------------------------------------------------------- Consts.
  /**
   * The ID of the main storage profile is always 1.
   */
  const DEFAULT_STORAGE_PROFILE_ID = 1;
  // ---------------------------------------------------------------- Functions.
  /**
   * Get storage profile.
   *
   * @param $storage_profile_id
   *   Profile to get.
   *
   * @return array
   *   An associative array with all profile attributes.
   */
  public static function get($storage_profile_id) {
    return mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp')
      ->fields('sp')
      ->condition(mediamosa_storage_profile_db::ID, $storage_profile_id)
      ->execute()
      ->fetchAssoc();
  }

  /**
   * Get the default storage profile.
   *
   * @return array
   *   An associative array with all profile attributes.
   *
   * @see mediamosa_storage_profile::get()
   */
  public static function get_default() {
    // Default is always profile 1.
    return self::get(self::DEFAULT_STORAGE_PROFILE_ID);
  }

  /**
   * Get listing storage profiles for app.
   *
   * @param int $app_id
   *   (optional) Application ID.
   * @param array $fields
   *   (optional) Custom column selection.
   *
   * @return array
   *   Array with storage profiles
   */
  public static function get_all($app_id = NULL, array $fields = array()) {
    $query = mediamosa_db::db_select(mediamosa_storage_profile_db::TABLE_NAME, 'sp');

    if (count($fields)) {
      foreach ($fields as $key => $field) {
        $query->addField('sp', $field, (is_numeric($key) ? $field : $key));
      }
    }
    else {
      $query->fields('sp');
    }

    if (isset($app_id)) {
      $query->condition(mediamosa_storage_profile_db::APP_ID, array(0, $app_id), 'IN');
    }

    // Order from app_id 0 and higher.
    $query->orderBy(mediamosa_storage_profile_db::APP_ID, 'ASC');
    return $query->execute();
  }

  /**
   * Get the options of the storage profile.
   *
   * @param array $storage_profile_id
   *   The storage profile ID.
   *
   * @return array
   *   The storage options.
   */
  public static function get_storage_options($storage_profile_id) {
    // Create the storage profile.
    return self::create_storage_instance_by_profile_id($storage_profile_id)->get_storage_options();
  }

  /**
   * Will test if storage profile exits, will throw exception when fail.
   *
   * @param int $storage_profile_id
   *   The storage profile ID.
   * @param int $app_id
   *   (optional) Must exists in combination with application ID.
   */
  public static function must_exists($storage_profile_id, $app_id = NULL) {
    if (isset($app_id)) {
      return mediamosa_db::db_must_exists(mediamosa_storage_profile_db::TABLE_NAME, array(mediamosa_storage_profile_db::APP_ID => array(0, $app_id), mediamosa_storage_profile_db::ID => $storage_profile_id));
    }

    return mediamosa_db::db_must_exists(mediamosa_storage_profile_db::TABLE_NAME, array(mediamosa_storage_profile_db::ID => $storage_profile_id));
  }

  /**
   * Create storage instance based on a storage profile ID.
   *
   * @param int $storage_profile_id
   *   The storage profile ID.
   *
   * @return mediamosa_storage
   *   MediaMosa storage object.
   *
   * @throws mediamosa_exception_error
   * @throws mediamosa_exception_program_error
   */
  public static function create_storage_instance_by_profile_id($storage_profile_id) {
    // For re-use.
    $storage_profiles = &drupal_static(__FUNCTION__, array());

    // Already loaded?
    if (isset($storage_profiles[$storage_profile_id])) {
      return $storage_profiles[$storage_profile_id];
    }

    // Get the storage_profile ID.
    $storage_profile = self::get($storage_profile_id);
    if (!$storage_profile) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_PROFILE_NOT_FOUND, array('@profile_id' => $storage_profile_id));
    }

    $storage_profiles[$storage_profile_id] = self::create_storage_instance_by_profile($storage_profile);
    return $storage_profiles[$storage_profile_id];
  }

  /**
   * Create storage instance based on a storage profile.
   *
   * @param array $storage_profile_id
   *   The storage profile.
   *
   * @return mediamosa_storage
   *   MediaMosa storage object.
   *
   * @throws mediamosa_exception_error
   * @throws mediamosa_exception_program_error
   */
  public static function create_storage_instance_by_profile(array $storage_profile) {

    // Get type.
    $type = $storage_profile[mediamosa_storage_profile_db::TYPE];

    // Create class.
    $class = 'mediamosa_storage_' . $type;
    if (!class_exists($class)) {
      throw new mediamosa_exception_error(mediamosa_error::ERRORCODE_STORAGE_CLASS_NOT_FOUND, array('@class' => $class));
    }

    if (!in_array('mediamosa_storage', class_parents($class))) {
      throw new mediamosa_exception_program_error("Class '@class' is invalid.", array('@class' => $class));
    }

    // Create object using the $storage_profile array.
    return new $class($storage_profile);
  }
}

<?php

/**
 * @file
 * Groups portal feed import commands.
 *
 */

/**
 * Implementation of hook_drush_help()
 */
function groups_feeds_drush_help($section) {
  switch ($section) {
    case 'meta:dsd:title':
      return dt('Groups portal feed import commands');
    case 'meta:dsd:summary':
      return dt('Import feeds programatically.');
  }
}

/**
 * Implementation of hook_drush_command()
 */
function groups_feeds_drush_command() {
  $items['import-meetup-events'] = array(
    'description' => 'Import meetup.com user group events.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(),
  );
  $items['import-static-pages'] = array(
    'description' => 'Import static pages.',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(),
  );
  return $items;
}

/**
 * Trigger meetup event import. Fetch all meetup urls
 * from group node's resource links, and import the ical
 * calendar into event entitites.
 */
function drush_groups_feeds_import_meetup_events() {
  $meetup_links = groups_feeds_get_meetup_links();
  foreach ($meetup_links as $meetup_link) {
    $url = $meetup_link['url'].'/events/ical/';
    $feedSource = feeds_source('ical_feed_importer');
    $config = $feedSource->getConfig();
    $config['FeedsHTTPFetcher']['source'] = $url;
    $feedSource->setConfig($config);
    try {
      while (FEEDS_BATCH_COMPLETE != $feedSource->import());
      $feedSource->log('import', 'Import meetup event: !url', array('!url' => $url), WATCHDOG_INFO);
    } catch (Exception $e) {
      echo 'Failed to import feed: '.$url."\n";
    }
  }
}

/**
 * Trigger static pages import.
 */
function drush_groups_feeds_import_static_pages() {
  $feedSource = feeds_source('static_page_import');
  $config = $feedSource->getConfig();
  $directory = variable_get('groups_feeds_markdown_directory', NULL);
  if (empty($directory)) {
    watchdog('drush_groups_feeds', 'Variable groups_feeds_markdown_directory missing.', array(), WATCHDOG_ERROR);
    return FALSE;
  }
  $config['feeds_fetcher_directory_fetcher']['source'] = $directory;
  $config['feeds_fetcher_directory_fetcher']['feed_files_fetched'] = array();
  $feedSource->setConfig($config);
  try {
    while (FEEDS_BATCH_COMPLETE != $feedSource->import());
    $feedSource->log('import', 'Import static page: !source', array('!directory' => $directory), WATCHDOG_INFO);
  } catch (Exception $e) {
    watchdog_exception('Groups Feeds', $e);
  }
}
<?php
/**
 * @file
 * Provides the functionality for the Zoo IUCN module
 */

/**
 * Implements hook_form()
 * Creates the IUCN update form
 */
function zoo_iucn_update_form($form, &$form_state) {
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Pull new data from IUCN (warning: takes a while)'),
  );
  return $form;
}

/**
 * Pulls data from IUCN
 */
function zoo_iucn_update_form_submit($form, &$form_state) {
  $animals = get_animals();
  // dpm($animals);
  // $animals = array_values($animals);
  $items_per_batch = 10;
  $batch = array(
    'title' => t('Importing IUCN data'),
    'init_message' => t('Beginning IUCN import'),
    'progress_message' => t('Fetched @percentage% of animal data so far.'),
    'error_message' => t('IUCN import failed.'),
    'file' => drupal_get_path('module', 'zoo_iucn') . '/includes/zoo_iucn.controller.inc',
  );

  $gs_array = array();

  $i = 0;
  foreach ($animals as $nid => $animal) {
    $gs_array[$i][$nid] = $animal->field_genus_and_species['und'][0]['value'];
    if (count($gs_array[$i]) == $items_per_batch) {
      $batch['operations'][] = array('iucn_process', array($gs_array[$i]));
      $i++;
    }
  }
    batch_set($batch);
}


/**
 * Processes newly arrived IUCN data, stores it in cache
 */
function iucn_process($gs_array) {
  foreach ($gs_array as $nid => $gs) {
    $result = iucn_call($gs);
    // dpm($result);
    if (!empty($result['result'])) {
      $final_result = array(
          'status' => $result['result'][0]['category'],
          'genus_species' => $result['result'][0]['scientific_name']
        );
    }
    else {
      $final_result = array(
          'status' => NULL,
          'genus_species' => 'Unable to find species at IUCN.'
        );
    }
    // dpm($final_result);
    cache_set($nid, $final_result, 'cache_iucn');
  }
}


/**
 * Takes species name, makes call to IUCN API and returns data on that species
 */
function iucn_call($gs) {
  $token = &drupal_static(__FUNCTION__);
  if (!isset($token)) {
    $token = variable_get('zoo_iucn_settings_api_token');
  }
  $url = 'http://apiv3.iucnredlist.org/api/v3/species/' . rawurlencode($gs) . '?token=' . $token;
  $result = drupal_http_request($url);
  $result = drupal_json_decode($result->data);
  return $result;
}


/**
 * Gets list of animals from content type
 */
function get_animals() {
  //get current animal data from db
  $animal_bundle = variable_get('zoo_iucn_settings_animal_ct', '');
  if ($animal_bundle) {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', $animal_bundle)
          //limit the number of animals queried.
          // ->range(0, 10)
          ;
    $result = $query->execute();
    $nids = array_keys($result['node']);
    $animals = node_load_multiple($nids);
    return $animals;
  }
}

/**
 * Performs update operations on animal nodes
 */
function zoo_iucn_update_animal($operation = '', $node = '') {
  $value = cache_get($node->nid, 'cache_iucn');
  $value = $value->data;

  //get the version of the IUCN status that the website uses
  $iucn_status_map = get_iucn_status_map();
  $value['status'] = $iucn_status_map[$value['status']];
  switch ($operation) {
    case 'gs':
      // dpm($animal->field_genus_and_species[0]->value());
      $node->field_genus_and_species['und'][0]['value'] = $value['genus_species'];
      node_save($node);
      break;
    case 'status':
      $node->field_iucn_designation['und'][0]['value'] = $value['status'];
      node_save($node);
      break;
    default:
      drupal_set_message(t('Invalid operation.'), 'error', FALSE);
  }
  // take user back to data table page
  drupal_goto('admin/content/iucn');
}


function zoo_iucn_node_update($node) {
  $animal_bundle = variable_get('zoo_iucn_settings_animal_ct', '');
  if ($node->type == $animal_bundle) {
  // whenever an animal node is updated, including by this module, we need to update the cache entry
  iucn_process(array($node->nid => $node->field_genus_and_species['und'][0]['value']));
  }
}

/**
 * Builds an array that maps IUCN conservation status values to the website's conservation list field's keys
 */
function get_iucn_status_map() {
  $status_field =  &drupal_static(__FUNCTION__);
  if (!isset($status_field)) {
    $status_field = variable_get('zoo_iucn_settings_iucn_status');
  }
  //get status array
  if ($status_field) {
    $status_field = field_info_field($status_field);
    $status_field = list_allowed_values($status_field);

    //get array of status field keys, since this is what we want to map to
    $status_field_keys = array_keys($status_field);

    // dpm($status_field);

    // Set up array of IUCN status value => spelled out version, then modify so that the end result is: [iucn value] => [website list field key].
    // This will be used to map values to how the website does IUCN statuses
    $iucn_status_map = array(
      'LC' => 'Least Concern',
      'NT' => 'Near Threatened',
      'VU' => 'Vulnerable',
      'EN' => 'Endangered',
      'CR' => 'Critically Endangered',
      'EW' => 'Extinct in the Wild',
      'EX' => 'Extinct',
      'DD' => 'Data Deficient',
      'NE' => 'Not Evaluated'
    );

    //get a lower-cased version for searching
    $status_field_keys_lower = array_map("strtolower", $status_field_keys);

    //Figure out how the website stores its status data
    foreach ($iucn_status_map as $k => $v) {

      //first search for the abbreviated version
      if ($status_key = array_search( strtolower($k), array_map("strtolower", $status_field_keys_lower) )) {
        $iucn_status_map[$k] = $status_field_keys[$status_key];
      }

      //then search for the spelled-out version
      elseif ($status_key = array_search( strtolower($v), array_map("strtolower", $status_field_keys_lower) )) {
        $iucn_status_map[$k] = $status_field_keys[$status_key];
      }
    }
    return $iucn_status_map;
  }
}

/**
 * Creates the data review table.
 */
function zoo_iucn_data_view() {
  $iucn_status_map = get_iucn_status_map();

  // get animal list
  $animals = get_animals();

  // create table
  $rows = array();
  foreach ($animals as $nid => $animal) {
    $gs = $animal->field_genus_and_species['und'][0]['value'];
    $status = $animal->field_iucn_designation['und'][0]['value'];
    // if any animal does not exist in cache, don't print any of them, because the user should pull fresh data
    if ( $iucn = cache_get($nid, 'cache_iucn') ) {
      $iucn = $iucn->data;
    }
    else {
      return '';
    }

    //prepare the genus/species operation
    if ($iucn['status'] === NULL) {
      $operation_gs = l(t('Google'), 'https://www.google.com/search?q=iucn+' . $gs, array('attributes' => array('target' => '_blank')));
    }
    else {
      $operation_gs = l(t('update'), 'iucn-update/gs/' . $animal -> nid);
    }


    //Take the abbreviated status that came in from IUCN and get the version used by the website's status field
    if (array_key_exists($iucn['status'], $iucn_status_map)) {
      $iucn['status'] = $iucn_status_map[$iucn['status']];

      //we now know that the IUCN status value corresponds with an option value in the website's status field, so we prepare the status operation
      $operation_status = l(t('update'), 'iucn-update/status/' . $animal -> nid);

      // //and we change the status value to the website's version so that it's ready to be written to the node
      // cache_set($nid, $iucn, 'cache_iucn');
    }

    //if IUCN genus species matches NZP genus species, remove it because there isn't a problem with it, else make bold and add operation
    $iucn['genus_species'] = ($iucn['genus_species'] == $gs) ? NULL : '<strong>' . $iucn['genus_species'] . '</strong> ' . $operation_gs;


    //if IUCN status matches NZP status, remove it because there isn't a problem with it.
    if (strtolower($iucn['status']) == strtolower($status)) {
      $iucn['status'] = NULL;
    }

    //if the IUCN status is still there, bold it and add the operation
    if (!is_null($iucn['status'])) {
      $iucn['status'] = '<strong>' . $iucn['status'] . '</strong> ' . $operation_status;
    }

    //clear the operations for the next animal
    $operation_gs = $operation_status = '';

    //only add an animal to the table if an issue has been found with either its status or gs
    if ($iucn['status'] || $iucn['genus_species']) {
    // kpr($animal);
      $rows[] =
        array(
          l($animal->title, $animal->path['alias'], array('attributes' => array('target' => '_blank'))),
          t($gs),
          t($iucn['genus_species']),
          t($status),
          t($iucn['status'])
          // ,'<a href="/node/' . $animal->nid . '/edit">Edit</a>'
        );
      }
    }

  //build the table
  $table = array(
  'header' => array(
    t('Common Name'),
    t('NZP Genus/Species'),
    t('IUCN Genus/Species'),
    t('NZP Conservation Status'),
    t('IUCN Conservation Status')
    ),
  'rows' => $rows
  );

  return theme('table', $table);
}

/**
 * Creates the data review page.
 */
function zoo_iucn_data_page_view() {
  $page = '';

  // conduct tests to check if settings are configured properly. If not, don't render 'fetch data' button. Display error message
  $settings_error = '';

  // First, test if can connect to IUCN API with panda scientific name. This tests the validity of the API token.
  $test_call = iucn_call('Ailuropoda melanoleuca');
  if (isset($test_call['message']) || isset($test_call['error'])) {
    $settings_error = TRUE;
  }

  //Now test that an animal bundle is selected, as well as a scientific name field and a conservation status field
  $animal_bundle = variable_get('zoo_iucn_settings_animal_ct');
  $status_field = variable_get('zoo_iucn_settings_iucn_status');
  $scientific_name = variable_get('zoo_iucn_settings_field_scientific_name');
  if (!isset($status_field) || !isset($status_field) || !isset($status_field)) {
    $settings_error = TRUE;
  }

  if ($settings_error) {
    drupal_set_message(t('There is something wrong with your IUCN API Settings. You cannot import until you <a href="/admin/content/iucn/settings">correct</a> them.'),
    'warning', FALSE);
  }
  else {
    $form = drupal_get_form('zoo_iucn_update_form');
    $page = drupal_render($form);
    $page .= zoo_iucn_data_view();
  }
  return $page;
}

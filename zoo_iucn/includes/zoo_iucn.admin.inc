<?php
/**
 * @file
 * Defines the settings form for the Zoo IUCN module
 */

 /**
  * Implements hook_form()
  * Creates the IUCN settings form
  */
 function zoo_iucn_settings_form($form, &$form_state) {
   $form['zoo_iucn_settings_api_token'] = array(
     '#type' => 'textfield',
     '#title' => t('IUCN API Token'),
     '#size' => 128,
     '#maxlength' => 128,
     '#required' => TRUE,
     '#default_value' =>  variable_get('zoo_iucn_settings_api_token'),
     // '#suffix' => ,
     #weight' => 5,
     // '#collapsible' => TRUE,
   );

   $form['zoo_iucn_settings_animal_ct'] = array(
     '#type' => 'select',
        '#title' => t('Your Animal Content Type'),
        '#options' => node_type_get_names(),
        '#default_value' => variable_get('zoo_iucn_settings_animal_ct', ''),
        '#description' => t('Select your animal content type. It should contain a field for "scientific name" and a field for "genus and species".'),
        '#required' => TRUE,
        '#attributes' => array('onChange' => 'this.form.submit();'),
   );

   // get the list of fields for the chosen animal bundle
   $animal_bundle = variable_get('zoo_iucn_settings_animal_ct', '');

   //only add the scientific name and conservation status fields if animal bundle as been chosen
   if ($animal_bundle) {
     $field_list = field_info_instances('node', $animal_bundle);
     $fields = array();
     foreach ($field_list as $field) {
       if ($field['widget']['type'] == 'text_textfield') {
         $fields[] = $field['field_name'];
       }
     }


     $form['zoo_iucn_settings_field_scientific_name'] = array(
       '#type' => 'select',
          '#title' => t('Your Scientific Name Field'),
          '#options' => $fields,
          '#default_value' => variable_get('zoo_iucn_settings_field_scientific_name'),
          '#description' => t('Select your scientific name field. It must be a text field.'),
          '#required' => TRUE,
     );

     $fields = array();
     foreach ($field_list as $field) {
       if ($field['widget']['type'] == 'options_select') {
         $fields[] = $field['field_name'];
       }
     }

     $form['zoo_iucn_settings_iucn_status'] = array(
       '#type' => 'select',
          '#title' => t('Your IUCN status field'),
          '#options' => drupal_map_assoc($fields),
          '#default_value' => variable_get('zoo_iucn_settings_iucn_status'),
          '#description' => t('Select your IUCN status field. It must be a select field.'),
          '#required' => TRUE,
     );
   }

   return system_settings_form($form);
 }

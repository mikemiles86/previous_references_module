<?php
  /**
   * @file previous_references.admin.inc
   * Admin functions for the previous_references module
   */

/**
 * systems settings form for default settings
 */
function previous_references_admin_settings($form,&$form_state){
	$form['defaults'] = array(
		'#type'	=> 'fieldset',
		'#title'	=> 'Default Settings',
	);
	
	$form['defaults']['previous_references_default_max'] = array(
		'#type'	=> 'select',
		'#title'	=> 'Number of Selections',
		'#description'	=> 'select how many selections to display',
		'#options'	=> range(0,10),
		'#default_value'	=> variable_get('previous_references_default_max',3),
	);
	
	$form['defaults']['previous_references_default_type'] = array(
		'#type'		=> 'select',
		'#title'	=> 'Selection Order',
		'#description'	=> 'select how the previous selections will be ordered',
		'#options'	=> array(
			'none'	=> 'Do not display',
			'last'	=> 'Last selected',
			'most popular'	=> 'Most popular selections',
			'newest'	=> 'Newest selections',
		),
		'#default_value'	=> variable_get('previous_references_default_type','none'),
	);
	
	$form = system_settings_form($form);
	$form['actions']['clear-cache'] = array(
		'#type'	=> 'button',
		'#id'		=> 'clear-caches',
		'#value'	=> t('Clear Caches'),
	);
	
	$form['#validate'][] = 'previous_references_admin_settings_validate';

	return $form;
}

/**
 * Validation function for settings form
 */
function previous_references_admin_settings_validate(&$form,&$form_state){
	//if clear cahce button was pressed, then clear the caches
	if ($form_state['triggering_element']['#id'] == 'clear-caches'){
		cache_clear_all('previous_references_data','cache');
		drupal_set_message('All previous reference caches cleared');
	}
}
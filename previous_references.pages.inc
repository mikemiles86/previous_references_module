<?php
	  /**
   * @file previous_references.pages.inc
   * Additional functions used by the previous_references module
   */
	 
	/**
	 * create field key for a field
	 *
	 * @param string $field_name
	 *	the machine name of the field
	 * @param string $entity_type
	 *	the entity type for this field instance
	 * @param string $bundle
	 *	the bundle for this field instance
	 *
	 * @return string
	 *	a concatted value of the provided arguments
	 */
	function previous_references_get_field_key($field_name,$entity_type,$bundle){
		return $field_name.':'.$entity_type.':'.$bundle;
	}
	
	/**
	 * explode a field key into its given parts
	 * 
	 * @param string $field_key
	 *	the field key for a field instance
	 *
	 * @return array
	 *	an array of the different field_key parts
	 */
	function previous_references_explode_field_key($field_key){
		$key_parts = explode(':',$field_key);
		return array(
			'field_name'	=> $key_parts[0],
			'entity_type'	=> $key_parts[1],
			'bundle'			=> $key_parts[2],
		);
	
	}

	/**
	 * get the cached data for previous references
	 */
	function previous_references_get_cache(){
		if ($data = cache_get('previous_references_data','cache')){
			return $data->data;
		}
		return false;
	}
	
	/**
	 * save the cache for all previous references
	 */
	function previous_references_set_cache($data){
		cache_clear_all('previous_references_data','cache');
		cache_set('previous_references_data',$data,'cache');
	}
	
	/**
	 * retrieve the settings for a specific field
	 *
	 * @param string $field_key
	 *	the field key of a field instance
	 * @param boolean $use_default
	 *	weather to use the defaults or not
	 *
	 * @return array
	 *	returns an array of settings for the given field
	 *  or false if not found.
	 */
	function previous_references_get_settings($field_key,$use_default=false){
		$previous_references_settings = variable_get('previous_references_settings',array());
		
		if (isset($previous_references_settings[$field_key])){
			return $previous_references_settings[$field_key];
		}else if ($use_default){
			return array(
				'max'		=> variable_get('previous_references_default_max',3),
				'type'	=> variable_get('previous_references_default_type','last'),
			);
		}
		
		return false;
	}
	
	/**
	 * Save the settings for a particular field instance
	 *
	 * @param string $field_key
	 *	the field key of a field instance
	 * @param array $settings
	 *	an array of settings values to save
	 */
	function previous_references_set_settings($field_key,$settings){
		$previous_references_settings = variable_get('previous_references_settings',array());
		$previous_references_settings[$settings_key] = $settings;
		variable_set('previous_references_settings',$previous_references_settings);
	}
	
	/**
	 * Retrieve the previous reference selections for a field instance
	 * 
	 * @param string $field_key
	 * 	the key of a field instance to retrieve
	 *
	 * @return array
	 *	return an array of previous selectsion or boolean false
	 */
	function previous_references_get_previous($field_key){
		$previous = false;
		//has a chace been stored?
		if ($cache = previous_references_get_cache()){
			//does this field have cached values?
			if (isset($cache[$field_key])){
				//return teh cached values
				return $cache[$field_key];
			}
		}
		
		//not found in cache so calculate previous selections from DB
		if ($previous = previous_references_calculate_previous($field_key)){
			//save these previous to cache for next time
			previous_references_set_previous($field_key,$previous);
			return $previous;
		}
		
		//nothing found, return false
		return false;
	}
	
	/**
	 * Store the previous selections to cache for a field instance
	 *
	 * @param string $field_key 
	 *	the key of a particular field instance
	 * @param array $previous
	 *	array of previous selections
	 */
	function previous_references_set_previous($field_key,$previous){
		//does cache exist?
		if ($cache = previous_references_get_cache()){
			//nothing
		}else{
			//no cache to set to an array
			$cache = array();
		}
		//add/replace field data in cache
		$cache[$field_key] = $previous;
		//save cache
		previous_references_set_cache($cache);
	}
	
	/**
	 * Delete previous references for one or many fields
	 *
	 * @param array $field_keys
	 *	an array of field_keys
	 */
	function previous_references_delete_previous($field_keys){
		//able to retrieve a cache? (cant delete what is not there)
		if ($cache = previous_references_get_cache()){
			//loop through the field keys
			foreach ($field_keys as $field_key){
				//if the field key is in cache
				if (isset($cache[$field_key])){
					//remove it from cache
					unset($cache[$field_key]);
				}
			}
		}
		//save the changed cache.
		previous_references_set_cache($cache);
	}
	
	/**
	 * Calculate the previous selecctions for a field instance
	 *
	 * @param string $field_key
	 *	the key of a particular field instance
	 *
	 * @return array
	 *	return array of pervious selctions or boolean false
	 */
	function previous_references_calculate_previous($field_key){
		//does this field have settings? (if not, will not know what to retrieve)
		if ($settings = previous_references_get_settings($field_key)){
			//if set to none, then do not retrieve anything
			if ($settings['type']=='none'){
				return false;
			}else{
				//get the different parts of the field key
				$key_parts = previous_references_explode_field_key($field_key);
				//query the database to get the values we are looking for
				$query = db_select('field_data_'.$key_parts['field_name'],'rf');
				$query->join('node','n','n.nid = rf.'.$key_parts['field_name'].'_target_id');
				$query->fields('n',array('title'));
				$query->fields('rf',array($key_parts['field_name'].'_target_id'));
				$query->range(0,$settings['max']);
				$query->distinct();
				
				//query will change depending on teh settings
				switch ($settings['type']){
					case 'most popular':
						//for most popular we need to group and count teh results
						$query->addExpression('COUNT(rf.'.$key_parts['field_name'].'_target_id)','ref_count');
						$query->groupBy('rf.'.$key_parts['field_name'].'_target_id');
						$query->orderBy('ref_count','DESC');
						break;
					case 'last':
						$query->orderBy('n.changed','DESC');
						break;
					case 'newest':
						$query->orderBy('n.created','DESC');
						break;
				}
				
				$results = $query->execute();
				
				$previous = array();
				//loop through the results and build an array
				while ($result = $results->fetchAssoc()){
					$previous[$result[$key_parts['field_name'].'_target_id']] = trim($result['title']);
					if ($settings['type'] == 'most popular'){
						$previous[$result[$key_parts['field_name'].'_target_id']].= '['.$result['ref_count'].']';
					}
				}
				//if have results return them, else return false
				return sizeof($previous)>0 ? $previous:false;
			}
		}
		return false;
	}
	
	/**
	 * Render the HTML of the previous links to display
	 *
	 * @param string $field_key
	 *	the key of a particular field instance
	 *
	 * @param array $previous_references
	 *	array of previous reference selections
	 *
	 * @return string 
	 *	return block of HTML
	 */
	function previous_references_render_previous($field_key,$previous_references){
		//get settings for field (to display title)
		$settings = previous_references_get_settings($field_key);
		//break the key into its parts
		$key_parts = previous_references_explode_field_key($field_key);

		$html = '<div class="description">';
		//for the title, if number of retrieve vlaues is less then max, then display the acutal count
		// title will look like 'The [N] [type] selections'
		$html.='The '.($settings['max']>sizeof($previous_references) ? sizeof($previous_references):$settings['max']).' '.$settings['type'].' selections<br />';
		$html.='<ul class="tips">';
		//loop through the slections to build links
		foreach ($previous_references as $entity_id=>$title){
			//build link attributes
			$attributes = array(
				//class, so link can be trageted by js
				'class' => array('previous-reference-option'),
				//target field data attribute for js
				'data-target-field' => 'pr-'.$key_parts['field_name'],
				//field value data attribute for js
				'data-field-value'  => preg_replace('/\[\\d+\]$/','',$title).' ('.$entity_id.')',
			);
			//add link to html
			$html .='<li>'.l($title,'#',array('attributes'=>$attributes)).'</li>';
		}
		$html.='</ul></div>';
		//return the html
		return $html;
	}

	
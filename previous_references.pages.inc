<?php
	
	function previous_references_get_field_key($field_name,$entity_type,$bundle){
		return $field_name.':'.$entity_type.':'.$bundle;
	}
	
	function previous_references_get_field_name($field_key){
		$key_parts = explode(':',$field_key);
		return array_shift($key_parts);
	}

	function previous_references_get_cache(){
		if ($data = cache_get('previous_references_data','cache')){
			return $data->data;
		}
		return false;
	}
	
	function previous_references_set_cache($data){
		cache_clear_all('previous_references_data','cache');
		cache_set('previous_references_data',$data,'cache');
	}
	
	function previous_references_get_settings($settings_key,$use_default=false){
		$previous_references_settings = variable_get('previous_references_settings',array());
		
		if (isset($previous_references_settings[$settings_key])){
			return $previous_references_settings[$settings_key];
		}else if ($use_default){
			return array(
				'max'		=> variable_get('previous_references_default_max',3),
				'type'	=> variable_get('previous_references_default_type','last'),
			);
		}
		
		return false;
	}
	
	function previous_references_set_settings($settings_key,$settings){
		$previous_references_settings = variable_get('previous_references_settings',array());
		$previous_references_settings[$settings_key] = $settings;
		variable_set('previous_references_settings',$previous_references_settings);
	}
	
	function previous_references_get_previous($field_key){
		$previous = false;
		if ($cache = previous_references_get_cache()){
			if (isset($cache[$field_key])){
				$previous = $cache[$field_key];
			}
		}
		
		if ($previous){
			return $previous;
		}else if ($previous = previous_references_calculate_previous($field_key)){
			previous_references_set_previous($field_key,$previous);
			return $previous;
		}
		
		return false;
	}
	
	function previous_references_set_previous($field_key,$previous){
		if ($cache = previous_references_get_cache()){
			//nothing
		}else{
			$cache = array();
		}
		
		$cache[$field_key] = $previous;
		
		previous_references_set_cache($cache);
	}
	
	function previous_references_delete_previous($field_keys){
		if ($cache = previous_references_get_cache()){
			foreach ($field_keys as $field_key){
				if (isset($cache[$field_key])){
					unset($cache[$field_key]);
				}
			}
		}
		
		previous_references_set_cache($cache);
	}
	
	function previous_references_calculate_previous($field_key){
		if ($settings = previous_references_get_settings($field_key)){
			if ($settings['type']=='none'){
				return false;
			}else{
				$field_name = previous_references_get_field_name($field_key);
				//query the database to get the values we are looking for
				$query = db_select('field_data_'.$field_name,'rf');
				$query->join('node','n','n.nid = rf.'.$field_name.'_target_id');
				$query->fields('n',array('title'));
				$query->fields('rf',array($field_name.'_target_id'));
				$query->range(0,$settings['max']);
				$query->distinct();
				
				
				switch ($settings['type']){
					case 'most popular':
						$query->addExpression('COUNT(rf.'.$field_name.'_target_id)','ref_count');
						$query->groupBy('rf.'.$field_name.'_target_id');
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
				
				while ($result = $results->fetchAssoc()){
					$previous[$result[$field_name.'_target_id']] = trim($result['title']);
					if ($settings['type'] == 'most popular'){
					//	$previous[$result[$field_name.'_target_id']].= '['.$result['ref_count'].']';
					}
				}
				return sizeof($previous)>0 ? $previous:false;
			}
		}
		return false;
	}
	
	function previous_references_render_previous($field_key,$previous_references){
		//render a list of links to add
		$settings = previous_references_get_settings($field_key);
		$field_name = previous_references_get_field_name($field_key);
		
		$html = '<div class="description">';
		$html.='The '.($settings['max']>sizeof($previous_references) ? sizeof($previous_references):$settings['max']).' '.$settings['type'].' selections<br />';
		$html.='<ul class="tips">';
		foreach ($previous_references as $entity_id=>$title){
			$attributes = array(
				'class' => array('previous-reference-option'),
				'data-target-field' => $field_name, //NEED TO GET ACTUAL ID
				'data-field-value'  => preg_replace('/\[\\d+\]$/','',$title).' ('.$entity_id.')',
			);
		
			$html .='<li>'.l($title,'#',array('attributes'=>$attributes)).'</li>';
		}
		$html.='</ul></div>';
		
		return $html;
	}

	
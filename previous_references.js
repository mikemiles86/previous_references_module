Drupal.behaviors.previousReferences = function (context){
	$('.previous-reference-option').click(function(e){
		e.preventDefault();
		var target_field = $(this).attr('data-target-field');
		var field_value	= $(this).attr('data-field-value');
		$('#'+target_field).value = field_value;
	});
}
(function ($) {
//add a drupal behavior
  Drupal.behaviors.previousReferences = {
    attach: function(context, settings) {
		//for all links that have class of .previoud-refernce-option
      $('.previous-reference-option').click(function(e){
			//stop the default link option
			e.preventDefault();
			//get the target field and the value
			var target_field = $(this).attr('data-target-field');
			var field_value	= $(this).attr('data-field-value');
			//get the field type
			var field_type = $('input.'+target_field).attr('type');
			switch (field_type){
				case 'radio':
					//need to find the one that has the value we are loking for
					$('input.'+target_field).each(function(){
						if ($(this).val() == field_value){
							$(this).attr('checked','checked');
						}
					});
					break;
				default:
					//set target field to value
					$('.'+target_field).val(field_value);
					break;
			}
		});
    }
  }
})(jQuery);
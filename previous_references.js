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
			//set target field to value
			$('.'+target_field).val(field_value);
		});
    }
  }
})(jQuery);
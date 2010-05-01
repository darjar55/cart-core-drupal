// $Id: uc_aac.js,v 1.1.2.1 2009/11/10 17:25:00 antoinesolutions Exp $

function uc_aac_calculate(element) {
  var form = $(element).parents('form');
  form.ajaxSubmit({
    url: Drupal.settings.base_path + "?q=uc_aac",
    dataType: 'json',
    success: function(data) {
      // Replace HTML elements with new values.
      var node = $('#node-' + data.nid);
      for (var i in data.replacements) {
        var replacement = $(data.replacements[i]);
        $(node).find('.' + i).after(replacement).remove();
      }

      // Update the add to cart form.
      if (data.form) {
        var action = form.attr('action');
        $(form).after(data.form).next().attr('action', action).uc_aac_attach();
        form.remove();
      }
    }
  });
}

jQuery.fn.uc_aac_attach = function() {
  $(this).find('select[@name^=attributes]').change(function() {
    uc_aac_calculate(this);
  });
  $(this).find('input:radio[@name^=attributes], input:checkbox[@name^=attributes]').click(function() {
    uc_aac_calculate(this);
  });
}

$(function() {
  $('.add-to-cart').uc_aac_attach();
});

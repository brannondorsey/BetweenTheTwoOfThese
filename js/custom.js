$(document).ready(function() {

    function setAspectRatio() {
      jQuery('#youtube-embed').each(function() {
        jQuery(this).css('height', jQuery(this).width() * 9/16);
      });
    }

    setAspectRatio();   
    jQuery(window).resize(setAspectRatio);
});
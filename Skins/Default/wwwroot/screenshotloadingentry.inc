var ##ID## = new Image();
$(##ID##).load(function () {
	//$(this).css('display', 'none'); // .hide() doesn't work in Safari when the element isn't on the DOM already
	$(this).hide();
	$('###SSID##').removeClass('loading').append(this);
	$(this).fadeIn();
	}).error(function () {
		$('###SSID##').removeClass('loading')
		$('###SSID##').addClass('bgFailed')
	}).attr({
		src : '/##SCREENSHOT##',
		id : '##ID2##',
		name : '##SSID##',
		width : '640',
		height : '360'
	});


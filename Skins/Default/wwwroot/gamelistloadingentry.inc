var ##GAMEID## = new Image();
$(##GAMEID##).load(function () {
//$(this).css('display', 'none'); // .hide() doesn't work in Safari when the element isn't on the DOM already
$(this).hide();
$('###GAMECONTENTID##').removeClass('loading').append(this);
$(this).fadeIn();}).error(function (){
$('###GAMECONTENTID##').removeClass('loading');
$('###GAMECONTENTID##').addClass('iconFailed');
}).attr('src', '##GAMEICON##');


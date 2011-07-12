var totalSlides = 0;
var currentSlide = 1;
var contentSlides = "";

$(document).ready(function(){
  $("#slideshow-previous").click(showPreviousSlide);
  $("#slideshow-next").click(showNextSlide);
  
  var totalWidth = 0;
  contentSlides = $(".slideshow-content");
  contentSlides.each(function(i){
    totalWidth += this.clientWidth;
    totalSlides++;
  });
  $("#slideshow-holder").width(totalWidth);
  $("#slideshow-scroller").attr({scrollLeft: 0});
  updateButtons();
});

function showPreviousSlide()
{
  currentSlide--;
  updateContentHolder();
  updateButtons();
}

function showNextSlide()
{
  alert("ShowNextSlide");
  currentSlide++;
  updateContentHolder();
  updateButtons();
}

function updateContentHolder()
{
  var scrollAmount = 0;
  contentSlides.each(function(i){
    if(currentSlide - 1 > i) {
      scrollAmount += this.clientWidth;
    }
  });
  $("#slideshow-scroller").animate({scrollLeft: scrollAmount}, 1000);
}

function updateButtons()
{
  if(currentSlide < totalSlides) {
    $("#slideshow-next").show();
  } else {
    $("#slideshow-next").hide();
  }
  if(currentSlide > 1) {
    $("#slideshow-previous").show();
  } else {
    $("#slideshow-previous").hide();
  }
}

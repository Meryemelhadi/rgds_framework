//
// Set of browser independent functions for measuring page sizes
// NB. copied from Mozilla.org

function Is ()
{   // convert all characters to lowercase to simplify testing
	var agt=navigator.userAgent.toLowerCase()

	// --- BROWSER VERSION ---
	this.major = stringToNumber(navigator.appVersion)
	this.minor = parseFloat(navigator.appVersion)

	this.nav  = ((agt.indexOf('mozilla')!=-1) && ((agt.indexOf('spoofer')==-1)
				&& (agt.indexOf('compatible') == -1)))
	this.nav2 = (this.nav && (this.major == 2))
	this.nav3 = (this.nav && (this.major == 3))
	this.nav4 = (this.nav && (this.major == 4))
	
	//Netscape 6
	this.nav5 =	(this.nav && (this.major == 5))
	this.nav6 = (this.nav && (this.major == 5))
	this.gecko = (this.nav && (this.major >= 5))

	this.ie   = (agt.indexOf("msie") != -1)
	this.ie3  = (this.ie && (this.major == 2))
	this.ie4  = (this.ie && (this.major == 3))
	this.ie5  = (this.ie && (this.major == 4))


	this.opera = (agt.indexOf("opera") != -1)
	 
	this.nav4up = this.nav && (this.major >= 4)
	this.ie4up  = this.ie  && (this.major >= 4)
}

/* Returns width of current window content area in pixels. */

function getCurrentWinWidth() 
{ 
  if (is.ie4up) return(document.body.clientWidth);
  return(window.innerWidth);
}

/* Returns height of current window content area in pixels. */
function getCurrentWinHeight()
{ 
  if (is.ie4up) return(document.body.clientHeight);
  return window.innerHeight;
}

var is = new Is();

function stringToNumber(s)
{
		return parseInt(('0' + s), 10)
}
/* Returns width of elt in pixels. */
function getEltWidth(elt) {

  if (is.nav4) {
	if (elt.document.width)
	  return elt.document.width;
	else
	  return elt.clip.right - elt.clip.left;
  }
  if (is.ie4up) {
	if (elt.style.pixelWidth)
	  return elt.style.pixelWidth;

	else
	  return elt.offsetWidth;
  }
  if (is.gecko) {
	if (elt.style.width)
	  return stringToNumber(elt.style.width);
	else
	  return stringToNumber(elt.offsetWidth);
  }
  return -1;
}

/* Returns height of elt in pixels. */
function getEltHeight(elt) {
  if (is.nav4) {
	if (elt.document.height)
	  return elt.document.height;
	else
	  return elt.clip.bottom - elt.clip.top;
  }
  if (is.ie4up) {
	if (elt && elt.style && elt.style.pixelHeight)
	  return elt.style.pixelHeight;
	else
	  return elt.clientHeight;
  }
  if (is.gecko) {
	if (elt.style.height)
	  return stringToNumber(elt.style.height);
	else
	  return stringToNumber(elt.offsetHeight);
  }
  return -1;
}

function getElt () 
{ if (is.nav4)
  {
	var currentLayer = document.layers[getElt.arguments[0]];
	for (var i=1; i<getElt.arguments.length && currentLayer; i++)
	{   currentLayer = currentLayer.document.layers[getElt.arguments[i]];
	}
	return currentLayer;
  } 
  else if(document.getElementById && document.getElementsByName)
  { 
	var name = getElt.arguments[getElt.arguments.length-1];
	if(document.getElementById(name))                      //First try to find by id
	   return document.getElementById(name);
	else if (document.getElementsByName(name))             //Then if that fails by name
	   return document.getElementsByName(name)[0];
  }
  else if (is.ie4up) {
	var elt = eval('document.all.' + getElt.arguments[getElt.arguments.length-1]);
	return(elt);
  }
}

//
// Class for writing a content box in the middle of the screen.
// Copyright: NxFrontier Ltd. 2003
//
function NxJS_CenteredBox(boxW,boxH,spacer,outColor,inColor)
{
	this.H = getCurrentWinHeight();
	this.W = getCurrentWinWidth();
	this.boxH = boxH;
	this.boxW = boxW;
	this.margH = Math.floor((this.H - boxH)/2);
	this.margW = Math.floor((this.W - boxW)/2);
	this.spacer = spacer;
	if (outColor)
		this.outColor = ' class="box_frame" bgcolor="'+outColor+'"';
	else
		this.outColor = ' class="box_frame"';

	if (inColor)
		this.inColor = ' class="box_content" bgcolor="'+inColor+'"';
	else
		this.inColor = ' height="100%" class="box_content"';
		
	
}

NxJS_CenteredBox.prototype.writeHeader = function()
{
	var posX= document.body.clientWidth/2 - this.boxW/2;
	if (posX<0)
		posX = 0;
	
	var posY = document.body.clientHeight/2 - this.boxH/2;
	if (posY<0)
		posY = 0;
	
	document.write(
		'<div id="boxID" style="position:absolute;left:'+posX+'px;top:'+posY+'px;width:'+this.boxW+';height:'+this.boxH+';" class="box_content">');
}

NxJS_CenteredBox.prototype.posY = function(boxH)
{
	var res = document.body.clientHeight/2 - boxH/2;
	
	if (res < 0)
		res=0;
		
	return res;
}

NxJS_CenteredBox.prototype.posX = function(boxW)
{
	var res = document.body.clientWidth/2 - boxW/2;
	
	if (res < 0)
		res=0;
		
	return res;
}

NxJS_CenteredBox.prototype.writeFooter = function()
{
	with(this)
	{
		document.write('</div>');
		boxID.style.setExpression("left","NxJS_CenteredBox.prototype.posX("+this.boxW+")");
	    boxID.style.setExpression("top","NxJS_CenteredBox.prototype.posY("+this.boxH+")");
	}

	// reload page when it is resized to keep the content centered
	// setTimeout("window.onresize = NxJS_CenteredBox.prototype.reloadPage", 2000);
}

NxJS_CenteredBox.prototype.reloadPage = function()
{
  window.location.reload();
}


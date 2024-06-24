// ------ NxHtmlElt ------
function NxHtmlElt(id) {
	this.attachId(id);
}

NxHtmlElt.prototype.attachId=function(id) {
	if (typeof(id)=="string")
	{
		var e = document.getElementById(id);
		this.attachElement(e);
	}
}

// create new DOM elt, assign attributes from attrs list (if attr name = text => add text)
NxHtmlElt.prototype.createElement=function(tag,attrs) {
	var e = document.createElement(tag);

	if (typeof(attrs)=="object")
		for(a in attrs) {
			if (a=="text")
				e.appendChild(document.createTextNode(attrs[a]));
			else
				e.setAttribute(a, attrs[a]);
		}
	this.attachElement(e);
}

// create html element and insert inline in body
NxHtmlElt.prototype.createInlineElt=function(tag,attrs) {
	var e = this.createElement(tag,attrs);
	this.attachElement(e);
	alert(e.innerHTML);
//    document.body.appendChild(e);
}

// create html element and add to parent NxHtmlElt
NxHtmlElt.prototype.addChildElt=function(tag,attrs,handlerClass) {
	// create Javascript object
	eval("var obj = new handlerClass();");

	// create html DOM Node
	obj.createElement(tag,attrs);

	// add new Node to parentr Node
    this.e.appendChild(obj.e);

	return obj;
}

// attach html element to object
NxHtmlElt.prototype.attachElement=function(e) {
	this.e=e;
	e.handler=this;
}

NxHtmlElt.prototype.attachEvent=function(ev,cb) {
	this.e.attachEvent(ev,cb);
}

NxHtmlElt.prototype.setCSSClass=function(c) {
	this.e.className=c;
}

NxHtmlElt.prototype.getSize=function() {
	return {w:this.e.offsetWidth,h:this.e.offsetHeight};
}

// ------ NxSelect -------
function NxInput(id) {	this.attachId(id); }

// inherit from NxHtmlElt
NxInput.prototype= new NxHtmlElt();

NxInput.prototype.getValue=function() {
	return this.e.value;
}

// ------ NxSelect -------
function NxSelect(id) {	this.attachId(id); }

// inherit from NxHtmlElt
NxSelect.prototype= new NxHtmlElt();

NxSelect.prototype.createInline=function(list) {
	// create select
	this.createElement("select");

	// add options
	var l=list.length;
	for(var i = 0; i < l ; i++) {
		var opt = list[i];
		// display option
		this.addChildElt("option",list[i],NxHtmlElt);
	}

	document.write(this.e.outerHTML);
}

NxSelect.prototype.select=function(index) {
	this.e.selectedIndex=index;
}

// select first option with given value
NxSelect.prototype.selectByValue=function(v) {
	if (typeof(this.e)=="undefined")
		return false;

	var opts=this.e.options;
	var l=opts.length;
	for(var i = 0; i < l ; i++) {
		if (opts[i].value==v)
		{
			this.e.selectedIndex=i;
			return;
		}
	}
	return true;
}

NxSelect.prototype.getSelectedIndex=function() {
	return this.e.selectedIndex;
}

NxSelect.prototype.getValue=function() {
	return this.e.options[this.e.selectedIndex].value;
}

// ------ NxImage class ------
function NxImage(id) {this.attachId(id);}

// inherit from NxHtmlElt
NxImage.prototype= new NxHtmlElt();

NxImage.prototype.resize=function(w,h) {
	this.e.width=w;
	this.e.height=h;
}

NxImage.prototype.setDefaultSize=function(w,h) {
	this.defaultSize= {w:w, h:h};
}

NxImage.prototype.resetSize=function() {
	this.resizeBox(this.defaultSize.w,this.defaultSize.h);
}

// fit in box
NxImage.prototype.resizeBox=function(w,h) {
	// define ratios
	var w0=this.e.width;
	var h0=this.e.height;
	var rw=w/w0;
	var rh=h/h0;

	// get smallest ratio
	var r = (rw<rh)?rw:rh;

	// resize image according to ratio
	this.box={w:w,h:h};
	this.resize(w0*r,h0*r);
}

NxImage.prototype.getBox=function() {
	return this.box;
}

NxImage.prototype.getUrl=function() {
	return this.e.src;
}

NxImage.prototype.setTitle=function(msg) {
	this.e.title=msg;
}

// preload image (static function)
NxImage.prototype.preload=function(url) {
//	var im = new Image(); 	im.src = url; 
}

// set new image
NxImage.prototype.loadUrl=function(url) {
	var im = new Image();
	im.src = url;

	this.e.src=url;
	this.e.width=im.width;
	this.e.height=im.height;

	// just in case...
	this.attachElement(this.e);
}

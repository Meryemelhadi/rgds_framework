// class Image Box
// process box descriptors
function ImageBox(desc) {
	var dim=desc.split("x");
	if (typeof(dim[1])!="undefined" && dim[1]!="0")
	{
		this.w=dim[0];
		this.h=dim[1];
		this.desc=desc;
		this.label=desc;
		this.path=desc+"/";
	}
	else
	{
		this.w=0;
		this.h=0;
		this.desc="";
		this.label="original";
		this.path="";
	}
}

// where to store singletons
var globals=new Object();

// class ImageList
// description: manage thumbnail list and image preview
function ImageList(imageList,thumbBox,previewBox,sizeList) {
	// keep display boxes
	this.thumbBox=new ImageBox(thumbBox);
	this.previewBox=new ImageBox(previewBox);

	// format list
	this.list = this.process(imageList);

	// declare as singleton
	globals.ImageList=this;

	// keep a ref on size list
	this.sizeList=sizeList;
	this.sizeList.selectBox(previewBox);
}

// process url list, define thumbnail and preview urls
//	from {url:"images/wdoctor1.jpg"},
//  to   {path:"images/", name:"wdoctor1.jpg",url:"images/wdoctor1.jpg"}
ImageList.prototype.process=function(list) {
	var l=list.length;
	var list2=new Array();
	for(var i = 0; i < l ; i++) {
		var im=list[i];
		// split full path into dir and name
		var pathArr=im.url.split(/[\/]/);
		var name=pathArr.pop();
		var path=pathArr.join("/")+"/";

		list2.push({url:im.url,name:name,path:path,index:i});
	}
	return list2;
}

ImageList.prototype.display=function() {
	this.displayList(this.list);
}

ImageList.prototype.displayList=function(list) {
	var l=list.length;
	
	// display list header
	var s=this.displayListHeader(list);
	
	for(var i = 0; i < l ; i++) {
		// display image
		s += this.displayThumb(list[i],i);
	}
	
	// display list footer
	s+=this.displayListFooter(list);

	document.write(s);
}

ImageList.prototype.getImageDesc=function(index) {
	return this.list[index];
}

ImageList.prototype.displayListHeader=function(list) {
//	return '<div class="thumbnail">';
	return '';
}

ImageList.prototype.displayListFooter=function(list) {
	return '';
//	return "</div>";
}

// create image thumbnail
ImageList.prototype.displayThumb=function(im,index) {
	// get box
	var box=this.thumbBox;

	// get url (dependent on box)
	var url=im.path+box.path+im.name;

	// preload image
	NxImage.prototype.preload(url);

	// display html
	return '<img class="thumbnail" onload="new Thumbnail(this,'+box.w+','+box.h+','+index+')" src="'+url+'"><br>';
}

// create image preview
ImageList.prototype.displayPreview=function(index) {
	// image descriptor
	var im=this.list[index];
	this.currentIndex=index;

	// get box
	var box=this.sizeList.getBox();
	var url=im.path+box.path+im.name;

	// preload image
	NxImage.prototype.preload(url);

	s='<img id="preview" class="preview" onload="new ImagePreview(this,'+box.w+','+box.h+','+index+')" src="'+url+'"><br>';
	document.write(s);
}

// replace by new image
ImageList.prototype.updatePreview=function(index) {
	// change image
	this.currentIndex=index;
	var im=this.list[index];

	// get box
	var box=this.sizeList.getBox();

	// update preview pane
	globals.ImagePreview.update(im,box);
}

// change preview size (reload new image)
ImageList.prototype.resizePreview=function(box) {
	index=this.currentIndex;
	var im=this.list[index];

	// update preview pane
	globals.ImagePreview.update(im,box);
}


//--------- Thumbnail class ---------
function Thumbnail(e,w,h,index) {
	this.attachElement(e);
	e.onload=null;
	box=this.box=new ImageBox(""+w+"x"+h);
	if (0+e.width>0)
	{
		mytrace("can't load thumbnail:"+e.src);
	}
	if (box.w>0 && box.h>0 && (""+e.width!=""))
	{
	//	this.resizeBox(w,h);
	}

	this.index=index;
	this.attachEvent('onclick', this.onClick);
	this.setTitle("image #"+index+" w="+e.width+" h="+e.height+ " url="+e.src);
}

// inherit from NxImage
Thumbnail.prototype= new NxImage();

// onclick event: update preview image
Thumbnail.prototype.onClick=function() {
	var e=window.event.srcElement;
	var handler=e.handler;

	// change image
	globals.ImageList.updatePreview(handler.index);

	// globals.SizeList.onChange();
	// alert("3) update done on :"+globals.ImagePreview.id);
}

//--------- Image preview class ---------
function ImagePreview(e,w,h,index) {
	// attach to html element
	mytrace("loaded preview:"+e.src+" w="+e.width);
	mytrace("event="+e.onload);
	e.onload=ImagePreview.prototype.onReload;
	mytrace("killed event");

	this.attachElement(e);

	if (""+e.width=="")
	{
		mytrace("can't load preview:"+e.src);
		this.loadUrl(e.src);
	}

	// get image info from list
	this.info=globals.ImageList.getImageDesc(index);
	this.index=index;
	this.setTitle("image #"+index+" w="+e.width+" h="+e.height+ " url="+e.src);
	this.setDefaultSize(w,h);

	// in case the image is larger than expected, fit it in the box
	// this.resizeBox(w,h);

	// declare as singleton (nb. for some unknown reason, IE duplicates this object when url changes..)
	globals.ImagePreview=this;
}

// inherit from NxImage
ImagePreview.prototype= new NxImage();

// update image (url or size)
ImagePreview.prototype.update=function(info,box) {
	// update info
	this.info=info;
	this.index=info.index;
	this.box=box;

	// load image
	var url=info.path+box.path+info.name;
	this.loadUrl(url);

	// just in case, resize it (if not original size)
	/*
	if (box.w>0 && box.h>0)
		this.resizeBox(box.w,box.h);
	else
		this.resetSize();
	*/

	// update tooltip
	with(this)
		this.setTitle("image #"+index+" w="+e.width+" h="+e.height+ " url="+e.src);
}

ImagePreview.prototype.onReload=function() {
	var e = window.event.srcElement;
	var me=e.handler;
	var box=me.box;

	mytrace("loaded:"+e.src+" w="+e.width);

	// just in case, resize it (if not original size)
	if (box.w>0 && box.h>0 && e.width>0)
	{
		mytrace("resize box: h="+box.h);
		// me.resizeBox(box.w,box.h);
	}
	else
		mytrace("resize box: original size");

	// update tooltip
	with(me)
		setTitle("image #"+index+" w="+e.width+" h="+e.height+ " url="+e.src);
}

// ---------- Size box choice -----------
function SizeList(l) {
	// process list
	this.list=this.process(l);

	// declare as singleton
	globals.SizeList=this;

	// default box is first option
	if (this.list.length>0)
		desc=this.list[0].box;
	else
		desc="";
	this.box=new ImageBox(desc);
}

// inherit from NxSelect
SizeList.prototype= new NxSelect();

// build size object
//	from  {text:"100x100",value:"100x100"},
// 	to	  {w:100,h:100,text:"100x100",box:"100x100"},
SizeList.prototype.process=function(list) {
	var l=list.length;
	var list2=new Array();
	var v,text;
	for(var i = 0; i < l ; i++) {
		v=list[i].value;

		// label
		if (typeof(list[i].text))
			text=list[i].text;
		else
			text=v;

		if (v=="")
			// original size
			list2.push({w:0,h:0, text:text,box:""});
		else
		{
			// get sizes
			var dim=v.split("x");
			if (typeof(dim[1])!="undefined")
				list2.push({w:dim[0],h:dim[1],text:text,box:v});
		}
	}
	return list2;
}

// select box
SizeList.prototype.selectBox=function(desc) {
	// select the right option (if exist)
	if (this.selectByValue(desc))
		// use current value (in case requested box doesnt exist)
		this.box=new ImageBox(this.getValue());
	else
		// store box for display
		this.box=new ImageBox(desc);
}

SizeList.prototype.getBox=function() {
	return this.box;
}

SizeList.prototype.display=function() {
//	this.createInline(this.list);
	document.write(this.displayList(this.list));
	var e=document.getElementById('SizeListE');
	this.attachElement(e);

	// in case selected before being alive...
	this.selectByValue(this.box.desc);

	// if box didnt exist, reset it
	this.box=new ImageBox(this.getValue());

// IE bug: cant attach onchange through DOM...
//	this.e.attachEvent('onchange',SizeList.prototype.onChange);
	//this.attachEvent('onchange',SizeList.prototype.onChange);
}

SizeList.prototype.displayList=function(list) {
	var s = '<select id="SizeListE" class="sizeOptions" onchange="globals.SizeList.onChange()">';
	var l=list.length;
	for(var i = 0; i < l ; i++) {
		// display image
		s += this.displaySize(this.list[i],i);
	}
	s+= "</select>";
	return s;
}

SizeList.prototype.reset=function() {
	this.select(0);
}

SizeList.prototype.displaySize=function(obj,index) {
	return '<option value="'+obj.w+'x'+obj.h+'">'+obj.text+'</option>';
}

SizeList.prototype.getSize=function() {
	var index = this.getSelectedIndex();
	return this.list[index];
}

SizeList.prototype.onChange=function() {
	// change the box desc
	this.box=new ImageBox(this.getValue());

	// update image (NB. cant call the preview singleton directly because IE clones it.. bug?)
	globals.ImageList.resizePreview(this.box);
}

function mytrace(msg) {
	if (typeof(traceW)!="undefined")
		traceW.innerHTML += msg+"<br>";
}

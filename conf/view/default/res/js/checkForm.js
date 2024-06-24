// CheckForm
// version 1.10
//	fy: added support for file database 13/02/2006
//  1.12: added fix for fck
//  1.13: added NxSubmit(f)

//---  browser dependent functions ----
function NxBrowser() {
	// NS 4
	if (document.all) 
	{
		this.name    = "IE";
		this.isNN4   = false;
		this.isIE    = true;
		this.isGecko = false;
	}
	else if (document.layers) 
	{
		this.name    = "NN4";
		this.isNN4   = true;
		this.isIE    = false;
		this.isGecko = false;
	}
	else if (document.getElementById) 
	{
		this.name    = "Gecko";
		this.isNN4   = false;
		this.isIE    = false;
		this.isGecko = true;
	}
}

var nxbrowser = new NxBrowser();

Date.prototype.getFullYear = function()
{
	var y=this.getYear();
	if (y<1000)
		y+=1900;
	return y;
	
	if (nxbrowser.isIE)
		return this.getYear();
	else
		return this.getYear() + 1900;
}

// class NxQueryString
// parse a query string and returns parameters
function NxQueryString(desc) {
	this.params=this.parseQueryString(desc);
}

NxQueryString.prototype.parseQueryString=function(q) {
	var keyValuePairs = new Array();
	var params = {};
	var pairs=q.split("&");

	for(var j=0; j < pairs.length; j++) 
	{
		if (pairs[j].length > 0)
		{
			var pair=pairs[j].split("=");
			if (pair.length>1)
			{
				params[pair[0]]=unescape(pair[1]);
			}
		}
	}
	return params;
}

NxQueryString.prototype.getParam=function(n,deft) {
	if (typeof(this.params[n])!="undefined")
		return this.params[n];

	return deft;
}
// /class NxQueryString

//---------- field check -----------------------
function isEmptyString(s) {
	return (s == null || /^	*$/.test(s));
}

function isIntegerRange(v,min,max)
{
	v.match(/[\d]/);

	if (v.match(/[\d]/) == null)
		return false;

	i = parseInt(v);

	if (i<min || i > max)
		return false

	return true;
}

function isValidDate(d,m,y)
{
	m = parseInt(m)-1;
	y = parseInt(y);
	if (y < 1000)
	{
		y += 2000;
	}

	var date = new Date(y,m,d);
	if (date.getDate()==d && date.getMonth()==m && date.getFullYear()==y)
		return true;
	else
		return false;
}

// ------- basic form field utilities ---------------
function getDDIndex(sValue, oDropDown)
{
	for(var i=0;i<oDropDown.options.length;i++)
		if(oDropDown.options[i].value==sValue)
			return i;
	return 0;
}

function getDDValue(oDropDown)
{
	if (oDropDown.selectedIndex>=0)
		return oDropDown.options[oDropDown.selectedIndex].value;
	return '';
}

function getRadioValue(oRadioBtn)
{
	var sValue="";
	if(isNaN(oRadioBtn.length)){
		return oRadioBtn.value;
	}
	else{
		for(var x=0;x<oRadioBtn.length;x++){
			if(oRadioBtn[x].checked){
				sValue=oRadioBtn[x].value;
				break;
			}
		}
		return sValue;
	}
}

function getValue(oElement){
	if((oElement.type == "text")|| (oElement.type == "password") || 
		(oElement.type == "textarea") || (oElement.type == "hidden")){
		return oElement.value;
	}
	else if((oElement.type == "select-one")|| (oElement.type == "select-multiple")){
		return getDDValue(oElement);
	}
	else if((oElement.type == "radio")|| (oElement[0].type == "radio")){
		return getRadioValue(oElement);
	}
	else if((oElement.type == "checkbox")|| (oElement[0].type == "checkbox")){
		return getRadioValue(oElement);
	}
	else{
		// alert("error: " + oElement.name + " is not a text or select object");
		return null;
	}
}

// Dialog API
window.dlg_handlers=[];
function openDialog(url,args,iWidth,iHeight,target){

	// store handler ref and get its ID
	var id=window.dlg_handlers.length;
	window.dlg_handlers[id]=args;

	// complete url with id
	if (id)
	{
		if (url.search(/[?]/)>=0)
			url +='&';
		else
			url+='?';
	}

	// set target
	if (typeof(target)=="undefined")
		target=null;
		
	var w=window.open(url,target,"height=" + iHeight + ",width=" + iWidth + ",resizable=yes,status=no,toolbar=no,menubar=no,location=no");
	w.focus();
	return w;
}

// Dialog handler: to be used with dialog that use wdialog.js API 
function DialogHandler() {
}

// method: InputChooseFile::open 
DialogHandler.prototype.open = function(url,w,h,target) {
	this.w=w;
	this.h=h;
	openDialog(url,this,w,h,target,"scrollbars=1");
}

// abstract method: InputChooseFile::onOk 
DialogHandler.prototype.onOk = function(result) {}

// virtual method: InputChooseFile::onCancel 
DialogHandler.prototype.onCancel = function(result) {}

// scripts used by Rich Text Editor

function RT_getRange() {
	var sel;
	var obj=null;
	sel = document.selection
	
	if(sel.type != "Control")	{
		obj = sel.createRange().parentElement();
	}
	else {
		obj = sel.createRange()(0);
	}
	
	if (obj.isContentEditable) {
		return obj;
	} else {
		alert("please select text to modify");
		return null;
	}
}

function RT_getFullRange() {
	var sel;
	var obj=null;
	sel = document.selection
	
	if (sel.type != "Control"){
		range = sel.createRange();
		if (!range)	{
			alert("please select text to cleanup");
			return null;
		}		
		
		if (range.htmlText.length == 0)
		{
			obj = range.parentElement();
			if (obj.tagName=="BODY") {
				alert("please select text to cleanup");
				return null;
			}
				
			while(obj && obj.isContentEditable && obj.className!="input")
			{
				obj = obj.parentElement;
			}
		}
		else
			obj = range.parentElement();
			
		return obj;
	}
	else {
		obj = sel.createRange()(0);
	}
	
	if (typeof(obj.isContentEditable)!="undefined" && obj.isContentEditable) {
		return obj;
	} else {
		alert("please select text to cleanup");
		return null;
	}
}

function RT_execCommand(cmd)
{
	if (cmd.substring(0,3) == "RT_")
		eval(cmd+"();");
	else
	{
		if (RT_getRange()!=null)
			document.execCommand(cmd);
	}
}

//  ==== Image selection, follows Dialog API ====
// RTE: manage image inserted from DB
function InputChooseImage() {
}
InputChooseImage.prototype=new DialogHandler();

InputChooseImage.prototype.onOk = function(result) {
	if (!result)
		return;

	var box=result.size.box;

	document.execCommand("InsertImage",false,result.url);
	if (document.selection.type=="Control")
	{
		// get inserted img object
		var oControlRange = document.selection.createRange();
		var img = oControlRange.item(0);
		if (img.tagName=="IMG")
		{
			img.border=result.border;
			if (result.align!="inline")
				img.align=result.align;
		}
	}
}

// class InputChooseFile : DB dialog handler for RTE
// manage insertion of link to file from DB
function InputChooseFile() {
}
InputChooseFile.prototype=new DialogHandler();

// method: InputChooseFile::onOk 
InputChooseFile.prototype.onOk = function(result) {
	if (!result)
		return;

	var box=result.size.box;

	document.execCommand("InsertImage",false,result.url);
	if (document.selection.type=="Control")
	{
		// get inserted img object
		var oControlRange = document.selection.createRange();
		var img = oControlRange.item(0);
		if (img.tagName=="IMG")
		{
			img.border=result.border;
			if (result.align!="inline")
				img.align=result.align;
		}
	}
}

//  ==== fullscreen popup, follows Dialog API ====
// manage input field
function fullscreen() {
}

fullscreen.prototype.open = function(url,w,h,e) {
	this.w=w;
	this.h=h;
	this.e=e;
	openDialog(url,this,w,h);
}

fullscreen.prototype.onOk = function(result) {
	if (!result)
		return;

	this.e.innerHTML=result.html;
}

fullscreen.prototype.onCancel = function() {
	alert("dialog cancel");
}


// class InputFieldImage: attach image field to this class
function InputFieldImage(field) {
	this.field=field;
}
InputFieldImage.prototype=new DialogHandler();
InputFieldImage.prototype.onOk=function(result){	
		this.field.value=result.url;
}

function getImageField(field) {
	//alert("click");
	(new InputFieldImage(field)).open('/rte/image_dlg.php',800,600);
}

function RT_insertImg() {
	(new InputChooseImage()).open('/rte/image_dlg.php',800,600);
}

function RT_maximize(edit_area) {
	(new fullscreen()).open('/rte/fullscreen.php',800,600,edit_area);
}

// ==== Cleanup of html range ====
function RT_cleanHTML()
{
	var e = RT_getFullRange();
		
	if (e!=null && e.innerHTML)
		e.innerHTML = RT_cleanup(e.innerHTML);
}
	
// Cleanup code coming from outside (with drag/drop copy/paste, etc.).
// Specially useful for cleaning up code generated by microsoft word.
//
// Removes class and style attributes, xml directive and islands,
// tags with namespaces, empty p or span tags.
/*
function RT_cleanup(src) 
{  
	var s = ""+src;
	return s.replace(/<([\w]+) class=([^ |>]*)([^>]*)/gi, "<"+"$1$3"
	).replace(/<([\w]+) style="([^"]*)"([^>]*)/gi, "<"+"$1$3"
	).replace(/<\\?\??xml[^>]*>/gi, ""
	).replace(/<script[^>]*>[^<]*<\/script>/gi, ""
	).replace(/<\/?\w+:[^>]*>/gi, ""
	).replace(/<p([^>])*>(&nbsp;)*\s*<\/p>/gi,""
	).replace(/<\/?font([^>])*>/gi,""
	).replace(/<span([^>])*>(&nbsp;)*\s*<\/span>/gi,"");
}
*/

function RT_cleanup(src) 
{  
	var s = ""+src;
	return s.replace(/<([\w]+) class=([^ |>]*)([^>]*)/gi, "<"+"$1$3"
	).replace(/<([\w]+) style="([^"]*)"([^>]*)/gi, "<"+"$1$3"
	).replace(/<\\?\??xml[^>]*>/gi, ""
	).replace(/<script[^>]*>[^<]*<\/script>/gi, ""
	).replace(/<\/?\w+:[^>]*>/gi, ""
	).replace(/<p([^>])*>(&nbsp;)*\s*<\/p>/gi,""
	).replace(/<\/?(span|h[1-9]|font|table|tr|td|tbody|th|div)([^>])*>/gi,""
	).replace(/<span([^>])*>(&nbsp;)*\s*<\/span>/gi,""
	).replace(/<b([^>])*>(&nbsp;)*\s*<\/b>/gi,""
	).replace(/<u([^>])*>(&nbsp;)*\s*<\/u>/gi,""
	);
}

function helpPlease(ref,whichcontent,header)
{
	if (document.all||document.getElementById)
	{
		var he=document.getElementById? document.getElementById("helpBox"):document.all.helpBox;

		s = '<div class="help">';
		if (header)	
				s+=	'<div class="helpHeader">Help</div>';
		s+= whichcontent +'</div>';
		he.innerHTML=s;
	}
}

function writeHelpBox()
{
	helpPlease(null,"For help on form fields please click on <img src='/nx/skins/default/images/help.gif'/> icons.","Help");
}

//------------ localised strings ----------------
function NxStrings(props, parent)
{
	if ("undefined"==typeof(props))
		this.props = new Object();
	else
		this.props = props;

	if ("undefined"==typeof(parent))
		this.parent = null;
	else
		this.parent = parent;
}

NxStrings.prototype.getString = function(name)
{
	if (typeof(cascade)=="undefined")
		cascade = true;

	if("undefined"!=typeof(this.props[name]))
		return this.props[name];
	else if (cascade && this.parent!=null)
		return this.parent.getString(name);
	else
		return name;
}

var en_res = 
	{ 
		"title":"title",
		"error":"error",
		"day":"day",
		"month":"month",
		"year":"year",
		"is not set":"is not set",
		"invalid date":"invalid date",
		"error, please check following fields":"error, please check following fields",
		"invalid date, check":"invalid date, check"
	};

var fr_res = 
	{ 
		"error":"erreur",
		"day":"jour",
		"month":"mois",
		"year":"annee",
		"is not set":"",
		"invalid date":"date invalide",
		"error, please check following fields":"erreur, verifiez les champs suivants",
		"invalid date, check":"date invalide, verifiez"
	};

var Nx_res = new NxStrings(en_res);

//------------ FORM FIELD CONTROLS ----------------

// TEXT AREA
function keypress_textarea(event,max)
{
	var ta = event.srcElement || event.target;

	if (ta.value.length > max)
	{
		event.cancelBubble = true;
		event.returnValue=false;
		return true;
	}
	else
	{
		event.cancelBubble = false;
		return false;
	}
}

function onchange_textarea(event,max)
{
	var ta = event.srcElement;

	if (ta.value.length > max)
	{
		alert("text is too long (maximum :" + max + " characters)");
		ta.value = ta.value.substr(0,max);
		event.cancelBubble = true;
		event.returnValue=false;
		return true;
	}
	else
	{
		event.cancelBubble = false;
		return false;
	}

}

// NUMBER
function keypress_number(event)
{
	evt = event || window.event;
	var keyPressed = evt.which || evt.keyCode;
	if ((keyPressed < 45 || keyPressed > 57) && (keyPressed != 8))
	{
		try {
			evt.preventDefault();
		} catch(e) {
			evt.returnValue = false;
		}
	}
}

// PHONE NUMBER
function keypress_phonenumber(event)
{
	var c = event.keyCode;
	if (c == 40 || c == 41 || c == 43 || c == 32)
		event.returnValue = true;
	else if (c < 45 || c > 57)
	{
		event.returnValue = false;
	}
	else
		event.returnValue = true;
}
// --------------- /FIELDS ---------------


// --------------- IMAGE FIELD CONTROL ---------------

// "keep" radio button
function Nx_onKeepImage(radioB,img,inputFile,inputDB)
{
	if (inputDB)
	{
		inputDB.style.visibility="hidden";
	}
//	var src=img.getAttribute("srcKeep");
	var src=radioB.getAttribute("srcKeep");
//	alert(src);
	img.src=src;
	inputFile.style.visibility="hidden";
	inputFile.setAttribute("value","");
	img.style.visibility="";
	return;
}

// "replace" radio button
function Nx_onReplaceImage(img,inputFile,inputDB)
{
	if (inputDB)
	{
		inputDB.style.visibility="hidden";
	}
	var path = inputFile.getAttribute("srcFile");
	// alert(path);
	if (typeof(path) != "undefined" && path != null && path != "")
	{
		// NB. bug in Opera that provides only partail file name..
		//inputFile.value=path;
		p = "file:///"+path.replace(/\\/g,"/");
		img.src = p;
		img.style.visibility="";
	}
	else
	{
		img.style.visibility="hidden";
	}
	inputFile.style.visibility="";
	return;
}

// "delete" radio button
var onChange=false;
function Nx_onDeleteImage(img,inputFile,inputDB)
{
	if (inputDB)
	{
		inputDB.style.visibility="hidden";
	}
	onChange=true;
	img.style.visibility="hidden";
	inputFile.style.visibility="hidden";
	inputFile.setAttribute("value","");
	onChange=false;
}

// "image" radio button
function Nx_onDBImage(img,inputFile,inputDB,inputDBUrl)
{
	onChange=true;
	img.style.visibility="hidden";
	inputFile.style.visibility="hidden";
	inputFile.setAttribute("value","");
	if (inputDB)
	{
		inputDB.style.visibility="";
		if (inputDBUrl.value)
		{
			img.src=inputDBUrl.value;
			img.style.visibility="";
		}
	}
	onChange=false;
}

// class InputFieldImageDB
// description: dialog control handler: manage image selection

// receive input fields as parameters so that it can update them
function InputFieldImageDB(fields) {
		this.fields=fields;
}
InputFieldImageDB.prototype=new DialogHandler();

// method InputFieldImageDB::onOk
// when image selected: refresh thumbnail and add URL in DB URL field
InputFieldImageDB.prototype.onOk=function(result){	
	// request mutex (for event pb)
	if (onChange==true)
		return;
	onChange=true;

	// set input tag value with path result (which includes image properties)
	var path = result;
	var e=this.fields.inputDBUrl;
	e.value=path;

	// refresh thumbnail if valid iamge path and user hasn't clicked on other radio button 
	// (in that case the image would be hidden)
	var img=this.fields.img;
	if (typeof(path) != "undefined" && path != null && path != "" && e.style.visibility!="hidden")
	{
		img.src = path;
		img.style.visibility="";
	}

	// release mutex
	onChange=false;
}
// /class InputFieldImageDB

// DB button handler:  open DB application and pass paramnters to it, 
function Nx_chooseDBImage(img,inputFile,inputDB,inputDBUrl,app)
{
	// inputFile: 
	// inputDB:
	// inputDBUrl: stores URL for image
	(new InputFieldImageDB({img:img,inputFile:inputFile,inputDB:inputDB,inputDBUrl:inputDBUrl})).open(app,800,600,'image_db');
}

// handler: is it used?
function NX_onDBFileChange(e,img)
{
	// request mutex (for event pb)
	if (onChange==true)
		return;
	onChange=true;

	// get path from 
	var path = e.value;

	// refresh thumbnail if valid iamge path and user hasn't clicked on other radio button 
	// (in that case the image would be hidden)
	if (typeof(path) != "undefined" && path != null && path != "" && e.style.visibility!="hidden")
	{
		e.setAttribute("srcFile",e.value);
		img.src = path;
		img.style.visibility="";
	}

	onChange=false;
}

// handler: called when file field has changed (ie user has clicked on browse)
function Nx_onFileChange(e,img)
{
	// request mutex (for event pb)
	if (onChange==true)
		return;
	onChange=true;

	// get image url from field value
	var path = e.value;

	// refresh image if image is still displayed
	if (typeof(path) != "undefined" && path != null && path != "" && e.style.visibility!="hidden")
	{
		// store image url in srcFile attribute
		e.setAttribute("srcFile",e.value);

		// update thumbnail (use local path asd URL)
		p = "file:///"+path.replace(/\\/g,"/");
		img.src = p;
		img.style.visibility="";
	}

	// release mutex
	onChange=false;
}

function NX_onEditChange(fname)
{
	f = document.getElementById(fname);
	edit = document.getElementById("edit_"+fname);
	f.value=edit.innerHTML;
}

// --------------- /IMAGE UPLOAD CONTROL ---------------

// --------------- FILE CONTROL ---------------
// radio button: keep
function Nx_onKeepFile(radioB,f_div,inputFileFile,inputDB)
{
	if (inputDB)
	{
		inputDB.style.visibility="hidden";
	}
	inputFileFile.style.visibility="hidden";
	inputFileFile.setAttribute("value","");
	f_div.style.visibility="";
	f_div.innerHTML = Nx_previewFile(radioB.getAttribute("srcKeep"));
	return;
}

function Nx_previewFile(desc) {
	var params=new NxQueryString(desc);
	return '<a href="'+params.getParam('url','#')+'">'+params.getParam('title','file')+'</a>';
}

// radio button: replace
function Nx_onReplaceFile(f_div,inputFileFile,inputDB)
{
	if (inputDB)
	{
		inputDB.style.visibility="hidden";
	}
	var path = inputFileFile.getAttribute("srcFileFile");
	// alert(path);
	if (typeof(path) != "undefined" && path != null && path != "")
	{
		// NB. bug in Opera that provides only partail file name..
		//inputFileFile.value=path;
		p = "file:///"+path.replace(/\\/g,"/");
		//todo f.href = p;
		// f_div.style.visibility="";
		f_div.style.visibility="hidden";
	}
	else
	{
		f_div.style.visibility="hidden";
	}
	inputFileFile.style.visibility="";
	return;
}

// radio button: delete
function Nx_onDeleteFile(f_div,inputFileFile,inputDB)
{
	if (inputDB)
	{
		inputDB.style.visibility="hidden";
	}
	onChange=true;
	f_div.style.visibility="hidden";
	inputFileFile.style.visibility="hidden";
	inputFileFile.setAttribute("value","");
	onChange=false;
}

// radio button: db
function Nx_onDBFile(f_div,inputFileFile,inputDB,inputDBUrl)
{
	onChange=true;
	f_div.style.visibility="hidden";
	inputFileFile.style.visibility="hidden";
	inputFileFile.setAttribute("value","");
	if (inputDB)
	{
		inputDB.style.visibility="";
		if (inputDBUrl.value)
		{
			f_div.innerHTML = Nx_previewFile(inputDBUrl.value);
			f_div.style.visibility="";
		}
	}
	onChange=false;
}

// class InputFieldFileDB : DB dialog handler for file field
function InputFieldFileDB(fields) {
		this.fields=fields;
}
InputFieldFileDB.prototype=new DialogHandler();
InputFieldFileDB.prototype.onOk=function(qs){	
	if (onChange==true)
		return;
	onChange=true;

	// store file info as query string
	var e=this.fields.inputDBUrl;
	e.value=qs;

	var f_div=this.fields.f_div;
	if (typeof(qs) != "undefined" && qs != null && qs != "" && e.style.visibility!="hidden")
	{
		// extract parameters from query string returned from dialog
		f_div.innerHTML=Nx_previewFile(qs);

		f_div.style.visibility="";
		this.fields.inputDBUrl.value=qs;
	}
	onChange=false;
}
// /class InputFieldFileDB

function Nx_chooseDBFile(f_div,inputFileFile,inputDB,inputDBUrl,app)
{
	(new InputFieldFileDB({f_div:f_div,inputFileFile:inputFileFile,inputDB:inputDB,inputDBUrl:inputDBUrl})).open(app,800,600);
}

// handler: db file field has changed
function NX_onDBFileFileChange(e,f_div)
{
	if (onChange==true)
		return;
	onChange=true;
	var path = e.value;
	if (typeof(path) != "undefined" && path != null && path != "" && e.style.visibility!="hidden")
	{
		e.setAttribute("srcFileFile",e.value);
		//todo f.href = path;
		f_div.style.visibility="";
	}

	onChange=false;
}

// handler: browse file field has changed
function Nx_onFileFileChange(e,f_div)
{
	// alert('file changed'+f.href);

	if (onChange==true)
		return;
	onChange=true;
	var path = e.value;
	if (typeof(path) != "undefined" && path != null && path != "" && e.style.visibility!="hidden")
	{
		e.setAttribute("srcFileFile",e.value);
		p = "file:///"+path.replace(/\\/g,"/");
		//todo f.href = p;
		f_div.style.visibility="";
	}

	onChange=false;
}

function NX_onEditFileChange(fname)
{
	f = document.getElementById(fname);
	edit = document.getElementById("edit_"+fname);
	f.value=edit.innerHTML;
}

// /FILE 

// -------- specific field checks -------------------
function isValidEmail(sValue)
{
	if(isEmptyString(sValue))
		return false;

	var isValid=true;
	var atIndex = sValue.indexOf("@");
	var dotLastIndex = sValue.lastIndexOf(".");
	if((atIndex < 1)|| (sValue.indexOf(".") == -1) || (atIndex > sValue.length-4) || (dotLastIndex < atIndex) || (dotLastIndex == (sValue.length-1)) || (dotLastIndex == atIndex+1))
		isValid=false;

	return isValid;
}

function checkSelect(label,f,isRequired)
{
	if (typeof(f)=="undefined" || f== null)
		return "";
//		return "\n+ " + label + " " + Nx_res.getString("is not set");

	s = getValue(f);

	if ((s == "?" || s == "") && isRequired)
		return "\n+ " + label + " " + Nx_res.getString("is not set");

	return "";
}

function checkDate(label,fday,fmonth,fyear,isRequired)
{
	if (typeof(fday)=="undefined" || fday== null)
		return "";

	var d = getValue(fday);
	var m = getValue(fmonth);
	var y = getValue(fyear);

	if (d == "?" && m == "?" && (y=="?" ||isEmptyString(y)))
	{
		// all field blank
		if (isRequired)
			return "\n+ " + label + " " + Nx_res.getString("is not set");
		return "";
	}
	
	var error = "";
	var sep="";
	isIntegerRange(d,1,31);
	if (!isIntegerRange(d,1,31))
	{
		error += sep+ Nx_res.getString("day");
		sep=", ";
	}

	if (!isIntegerRange(m,1,12))
	{
		error += sep+Nx_res.getString("month");
		sep=", ";
	}

	if (!isIntegerRange(y,0,9999))
	{
		error += sep+Nx_res.getString("year");
		sep=", ";
	}

	if (error != "" || !isValidDate(d,m,y))
	{
		if (error != "")
			return "\n+ " + label + ": "+Nx_res.getString("invalid date, check")+" "+error;
		else
			return "\n+ " + label + ": "+Nx_res.getString("invalid date");
	}

	return "";
}

function checkText(label,f,isRequired)
{
	if (typeof(f)=="undefined" || f== null)
		return "";

	var s = f.value;

	if (isEmptyString(s) && isRequired)
		return "\n+ " + label + " " + Nx_res.getString("is not set");

	return "";
}

function checkReg(label,f,isRequired,reg,errMsg)
{
	if (typeof(f)=="undefined" || f== null)
		return "";

	var s = f.value;

	if (isEmptyString(s))
	{
		if (isRequired)
			return "\n+ " + label + " " + Nx_res.getString("is not set");
	}
	else
	{
		var re = new RegExp(reg,"ig");
		if (!re.test(s))
			return "\n+ " + label + " " + Nx_res.getString(errMsg);
	}
	
	return "";
}

function checkTelephone(label,f,isRequired)
{
	return checkReg(label,f,isRequired,"^[0-9 +()]+$","check format, valid characters are +() 0-9");
}

function checkLocalPhone(label,f,isRequired)
{
	return checkReg(label,f,isRequired,"^[0-9 ]+$","check format, valid characters are 0..9");
}

function checkCheckBoxTree(label,frm,fName,isRequired)
{
	var f = frm[fName+"[]"];
	if (!isRequired)
		return "";

	for(var i=0;i<f.length;i++)
	if(f[i].checked==true)
		return "";

	return "\n+ " + label + " " + Nx_res.getString("is not set");
}

//------ Rich Editor
function checkRichText(label,fname,isRequired,f)
{
	if (typeof(f)=="undefined" || f== null)
	{
		var f = document.getElementById(fname);
	}

	var edit = document.getElementById("edit_"+fname);

	if (typeof(f)=="undefined" || f== null)
		return "";

	if (edit!=null)
	{
		f.value=edit.innerHTML;
	}

	if (isEmptyString(f.value) && isRequired)
		return "\n+ " + label + " " + Nx_res.getString("is not set");

	return "";
}

function onSubmitForm(formName,url)
{
	var form = document[formName];
	form.action = url;

	if(form.onsubmit()==true)
		form.submit();
}

// supports actions with parameters and action=get
function NxSubmit(f) {
	if (f.onsubmit())
	{
		if (f.method.toLowerCase()=='get' && /\?/.test(f.action))
			document.location=f.action+"&"+Form.serialize(f);
		else
			f.submit();
	}
}

function Nx_navigateTo(url) {
	document.location=url;
	window.event.cancelBubble=true;
	window.event.returnValue=false;
	return false;
}

function Nx_popup(url,target,w,h) {
	window.open(url,target,"height=" + h + ",width=" + w + ",status=no,toolbar=no,resizable=yes,scrollbars=yes,menubar=no,location=no");
	window.event.cancelBubble=true;
	window.event.returnValue=false;
	return false;
}

Nx_alert = function (s) {
	alert(s);
}
@include(js/browser,device)
@include(js/sysform,device)
@include(js/core/wdialog/open,device)
@include(js/RTEdit,skin and device)
@include(js/help,skin and device)
@include(js/NxString,skin and device)

//------------ Event handlers ----------------
function keypress_textarea(max)
{
	var ta = event.srcElement;

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

function onchange_textarea(max)
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

function keypress_number()
{
	if (event.keyCode < 45 || event.keyCode > 57) 
		event.returnValue = false;
}

function keypress_phonenumber()
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


// IMAGE 
function Nx_onKeepImage(img,inputFile)
{
	img.src=img.getAttribute("srcKeep");
	inputFile.style.visibility="hidden";
	inputFile.setAttribute("value","");
	img.style.visibility="";
	return;
}

function Nx_onReplaceImage(img,inputFile)
{
	var path = inputFile.getAttribute("srcFile");
	if (typeof(path) != "undefined" && path != null && path != "")
	{
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

var onChange=false;
function Nx_onDeleteImage(img,inputFile)
{
	onChange=true;
	img.style.visibility="hidden";
	inputFile.style.visibility="hidden";
	inputFile.setAttribute("value","");
	onChange=false;
}


function Nx_onFileChange(e,img)
{
	if (onChange==true)
		return;
	onChange=true;
	var path = e.value;
	if (typeof(path) != "undefined" && path != null && path != "" && e.style.visibility!="hidden")
	{
		e.setAttribute("srcFile",e.value);
		p = "file:///"+path.replace(/\\/g,"/");
		img.src = p;
		img.style.visibility="";
	}

	onChange=false;
}

function NX_onEditChange(fname)
{
	f = document.getElementById(fname);
	edit = document.getElementById("edit_"+fname);
	f.value=edit.innerHTML;
}

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

	s = getValue(f);

	if ((s == "?" || s == "") && isRequired)
		return "\n+ " + label + " " + Nx_res.getString("is not set");

	return "";
}

function checkDate(label,fday,fmonth,fyear,isRequired)
{
	if (typeof(fday)=="undefined" || fday== null)
		return "";

	d = getValue(fday);
	m = getValue(fmonth);
	y = getValue(fyear);

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

	s = f.value;

	if (isEmptyString(s) && isRequired)
		return "\n+ " + label + " " + Nx_res.getString("is not set");

	return "";
}

function checkReg(label,f,isRequired,reg,errMsg)
{
	if (typeof(f)=="undefined" || f== null)
		return "";

	s = f.value;

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
function checkRichText(label,fname,isRequired)
{
	f = document.getElementById(fname);
	edit = document.getElementById("edit_"+fname);

	if (typeof(f)=="undefined" || f== null)
		return "";

	f.value=edit.innerHTML;

	if (isEmptyString(f.value) && isRequired)
		return "\n+ " + label + " " + Nx_res.getString("is not set");

	return "";
}

function onSubmitForm(formName,url)
{
	form = document[formName];
	form.action = url;

	if(form.onsubmit()==true)
		form.submit();
}

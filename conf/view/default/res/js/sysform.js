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
	else{
		alert("error: " + oElement.name + " is not a text or select object");
		return null;
	}
}

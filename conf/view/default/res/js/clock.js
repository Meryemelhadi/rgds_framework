function nxGetElementById(id)
{
	if (document.getElementById)
		return document.getElementById(id);
	else if (document.all)
		return document.all[id];
	else if (document.layers)
		return document[id];
	else
		return null;
}

function dateString() 
{
	var d = new Date();
	var s = d.toLocaleString();
	var re = /:[0-9]+$/;
	return s.replace(re, "");
}

function onTick()
{
	var clck = nxGetElementById("clock");

	if (clck != null && clck.innerHTML)
		clck.innerHTML=dateString();

	window.setTimeout("onTick()",30*1000);
}

onTick();

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

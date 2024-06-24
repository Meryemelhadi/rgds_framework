function init(){
	coord1 = document.helpBox;
}

function helpPlease(ref,whichcontent,header)
{
	x = document.layers["helpBox"];	
	mess= "<TABLE WIDTH=150 BORDER=0 CELLPADDING=0 CELLSPACING=0 bgcolor='#FFFFFF'><TR><TD WIDTH=150 colspan='5'><IMG SRC='/nx/images/spacer.gif' WIDTH=150 HEIGHT=16></TD></TR><TR><TD WIDTH=150 colspan='5' bgcolor='#999999'><IMG SRC='/nx/images/spacer.gif' WIDTH=150 HEIGHT=1></TD></TR><TR><TD WIDTH=1 bgcolor='#999999'><IMG SRC='/nx/images/spacer.gif' WIDTH=1 HEIGHT=100></TD><TD WIDTH=5><IMG SRC='/nx/images/spacer.gif' WIDTH=5></TD><TD WIDTH=148 valign='top'><IMG SRC='/nx/images/spacer.gif' WIDTH=138 HEIGHT=10><br><font class='txtpetit'>" + whichcontent + "</font></TD><TD WIDTH=5><IMG SRC='/nx/images/spacer.gif' WIDTH=5></TD><TD WIDTH=1 bgcolor='#999999'><IMG SRC='/nx/images/spacer.gif' WIDTH=1 HEIGHT=100></TD></TR><TR><TD WIDTH=150 colspan='5' bgcolor='#999999'><IMG SRC='/nx/images/spacer.gif' WIDTH=150 HEIGHT=1></TD></TR></TABLE>";
	x.document.open();
	x.document.write(mess);
	x.document.close();
}

function writeHelpBox()
{
	helpPlease(null,"For help on form fields please click on <img src='/nx/skins/default/images/help.gif'/> icons.","Help");
}
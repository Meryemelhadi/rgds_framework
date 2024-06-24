// Dialog API
function openDialog(url,args,iWidth,iHeight){
    var sEdge="Raised";
	var bCenter="Yes";
	var bHelp="No";
	var bResize="Yes";
	var bStatus="No";

//	var sFeatures="dialogHeight: " + iHeight + "px; dialogWidth: " + iWidth + "px; dialogTop: " + iTop + "px; dialogLeft: " + iLeft + "px; edge: " + sEdge + "; center: " + bCenter + "; help: " + bHelp + "; resizable: " + bResize + "; status: " + bStatus + ";";

	var sFeatures="center: " + bCenter + "; resizable: " + bResize + "; status: " + bStatus + ";";
	window.showModalDialog(url, args, sFeatures)
}

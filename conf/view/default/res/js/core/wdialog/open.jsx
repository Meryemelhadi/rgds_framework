

// Dialog API
window.dlg_handlers=[];

function openDialog(url,args,iWidth,iHeight,target){

	// store handler ref and get its ID
	var id=window.dlg_handlers.length;
	window.dlg_handlers[id]=args;

	// complete url with id
	if (url.search(/\?/)
		url +='&';
	else
		url.='?';

	// set target
	if (typeof(target)=="undefined")
		target=null;
		
	return window.open(url,target,"height=" + iHeight + ",width=" + iWidth + ",status=no,toolbar=no,menubar=no,location=no");
}

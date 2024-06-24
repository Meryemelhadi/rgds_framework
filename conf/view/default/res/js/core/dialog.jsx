

function DialogControl() {
	this.handler=window.dialogArguments;
}

DialogControl.prototype.returnValue=function(val) {
	this.handler.onOk(val);
	window.returnValue=true;
	window.close();
}

DialogControl.prototype.cancel=function() {
	this.handler.onCancel();
	window.returnValue=false;
	window.close();
}

DialogControl.prototype.resizeTo=function(w,h) {
	// dialog
	window.dialogWidth=w+"px";
	window.dialogHeight=h+"px";

	// std window
	window.resizeTo(w,h); 
}

var dialog=new DialogControl();


function DialogControl() {
	if (typeof(window.parent.dlg_handlers)!="undefined")
	{
		var id=window.parent.dlg_handlers.length;
		this.handler=window.parent.dlg_handlers[id];
	}
	else
		this.handler=null;
}

DialogControl.prototype.returnValue=function(val) {
	if (this.handler!=null)
	{
		this.handler.onOk(val);
		window.close();
	}
	else
		alert('no parent window...');
}

DialogControl.prototype.cancel=function() {
	if (this.handler!=null)
	{
		this.handler.onCancel();
		window.close();
	}
}

DialogControl.prototype.resizeTo=function(w,h) {
	window.resizeTo(w,h); 
}

var dialog=new DialogControl();


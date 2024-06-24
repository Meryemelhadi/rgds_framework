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
	if (nxbrowser.isIE)
		return this.getYear();
	else
		return this.getYear() + 1900;
}

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
	if (y < 100)
	{
		y += 2000;
	}

	var date = new Date(y,m,d);
	if (date.getDate()==d && date.getMonth()==m && date.getFullYear()==y)
		return true;
	else
		return false;
}

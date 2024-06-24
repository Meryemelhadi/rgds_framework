//------------ localised strings ----------------
function NxStrings(props, parent)
{
	if ("undefined"==typeof(props))
		this.props = new Object();
	else
		this.props = props;

	if ("undefined"==typeof(parent))
		this.parent = null;
	else
		this.parent = parent;
}

NxStrings.prototype.getString = function(name)
{
	if (typeof(cascade)=="undefined")
		cascade = true;

	if("undefined"!=typeof(this.props[name]))
		return this.props[name];
	else if (cascade && this.parent!=null)
		return this.parent.getString(name);
	else
		return name;
}

var en_res = 
	{ 
		"title":"title",
		"error":"error",
		"day":"day",
		"month":"month",
		"year":"year",
		"is not set":"is not set",
		"invalid date":"invalid date",
		"error, please check following fields":"error, please check following fields",
		"invalid date, check":"invalid date, check"
	};

var fr_res = 
	{ 
		"error":"erreur",
		"day":"jour",
		"month":"mois",
		"year":"annee",
		"is not set":"",
		"invalid date":"date invalide",
		"error, please check following fields":"erreur, verifiez les champs suivants",
		"invalid date, check":"date invalide, verifiez"
	};

var Nx_res = new NxStrings(en_res);
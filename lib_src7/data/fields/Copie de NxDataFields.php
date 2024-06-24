<script language="php">
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 TODO:
	- complete DataField library:
		+ image (file + size attributes + alt + resize)
		+ compute "FORM" parameters from fields
		+ add form ID (useful when several different forms on the page)
		+ page or form state
		+ rich text field form
		+ calendar field

	- in pass 1: translate all "-" chars to "_" in attribute names.

	- map sub types to primary types:
		+ email -> text + filter + size

	- initialise choice lists descriptors needed by dynamic fields.
	- initialise Fields descriptors and objects from <field> elements
	- when field grouping, creates names: short, extended, group (done in pass 1)
	- create session object datasources and data output (=> save/retrieve properties from/to another page)
	- create user object datasources and data output (=> save/retrieve properties from/to user database)
	- compound data views can use session objects as datasources.
	- store choice lists in XML files
	- create web site (application and/or session object) and page property sets (transient global object)
	- create user property set objects (in session)

	property chain:	
		<b>web site</b>:	field -> page -> web site
		<b>portal</b>: user field -> user page -> user web site (clones of regular objects + modification)
*/

// require_once(NX_LIB."NxProperties.inc");

// text choice field: "no selection" value
// define("TC_NO_SELECTION","?");
define('TC_NO_SELECTION','');

// ======================= Field Descriptor =================================
class NxFieldDesc extends NxProperties
{
	var $name;

	function __construct($props,$parent)
	{
		parent::__construct($props,$parent);
		$this->name = $this->getProperty('name','',false);
	}

	function & createField(&$recData, $dataFormat,&$errorCB_)
	{
		$errorCB=new NxErrorCB();
		
		if ($this->props['type'] == '')
			die('error in dataset:'.$this->props['name']);

		// create field
		$className = 'NxDataField_' . $this->props['type'];

		// load field handler class (if already loaded, does nothing
		if (!class_exists($className))
			require_once(NX_LIB."data/fields/{$className}.php");
		
		$field = new $className($this);
		$handler="readFrom{$dataFormat}";

		// initialise it
		switch(strtolower($dataFormat))
		{
			case 'form':
			case 'db':
				$field->$handler($recData,$errorCB);
				break;
			case 'object':
			case 'array':
				if (!isset($recData[$this->name]))
					$field->setDefault();
				else
					$field->readFromStore($recData[$this->name]);
				break;
			case 'default':
				$field->setDefault();
				break;
			default:
				$field->clear();
				break;
		}
		
		if ($errorCB->isError())
			$errorCB_->addOtherError($errorCB);
		
		return $field;
	}

	function & getName()
	{
		return $this->name;
	}

	/** returns the prompt if any. check localised version in that order: 
	 *  localised "field.prompt:field name", localised field "prompt" property, "prompt" property */
	function & getPrompt($default=null)
	{
		if (!$default)
			$default = $this->getProperty('prompt',null,false);
		return $this->getString('field.prompt:'.$this->name,null,null,$default,$default);
	}
	
	function & getFormName()
	{
		$fname=$this->getProperty("alias",$this->name,false);
		if (!$this->getProperty("multiRecords",false,false))
			return $fname;
		else
			return "{$fname}[]";
	}
	
	function & getType()
	{
		return $this->props['type'];
	}
	
	function getAlias()	{
		return $this->getProperty("alias",$this->name,false);
	}
	
	function getLabel($lang=null)
	{
		$label =& $this->props["label"];

		// check if lang enabled
		if (!is_array($label))
		{		
			return $this->getString("field:$label",$lang,null,$label,"field:$this->name");
		}

		// if default lang, get it from properties
		if ($lang == null)
			$lang = $this->getProperty("lang",null);
			
		// if lang doesn't exist for label, fallback on first entry in the array
		if (isset($label[$lang]))
			return $label[$lang];
		else
			return reset($label);			
	}

	function isRequired()
	{
		return $this->getProperty("isRequired",false,false);
	}

	// can be edited in form
	function isEdit()
	{
		return (boolean)$this->getProperty("isEdit",true,false);
	}
	
	// hidden field
	function isHidden()
	{
		return ! $this->getProperty("show",true,false);
	}

	// to be part of form at all
	function isInput()
	{
		return $this->getProperty("isInput",true,false);
	}

	function isKey()
	{
		return $this->getProperty("isKey",null,false)!=null;
	}

	function addError(&$errorCB,$errorType, $p1="",$p2="",$p3="")
	{
		$list = $this->getChoiceList("FieldErrors",null,'strings');
		$msg = sprintf($list->getChoice($errorType),$p1,$p2,$p3);
		$this->errorMsg = $msg;
		$msg = "\"".$this->getLabel()."\" : ".$msg;
		$errorCB->addError($msg);
	}
	
	function onSubmitCheck($frm)
	{
		// get onSubmitCheck handler (of the form of sprintf format string, with 
		//  param 1: js form object  
		//  param 2: label (string)
		//  param 3: isRequired (bool)
		if ($this->isEdit())
		{
			$checker = $this->getProperty("onSubmitCheck",null,false);
			if ($checker != null)
			{
				$label =& $this->props["label"];
				$label=preg_replace("/[']/","\\'",$label);
				return sprintf($checker,$frm,$this->name,$label,$this->isRequired()?"true":"false");
			}
		}
		return "";
	}
	
	function getControl($control="select")
	{
		$controlClass = "NxControl_".$control;
		if (!class_exists($controlClass))
			require_once(NX_LIB."data/fields/controls/{$controlClass}.php");
			
		return new $controlClass();
	}
	
	function &getList($listName=null) 
	{
		if ($listName==null)
		{
			$listName = $this->getProperty("list",$this->props['name'],false);

			if ($ds=$this->getProperty("ds",null,false))
				$listName.=preg_replace('/[:]/','_',$ds);
		}
			
		$dir=$this->getProperty('dir','lists',false);
		
		if (($ds=$this->getProperty('ds',null,false))!=null)
		{
			include_once('lists/NxChoiceList_ds.inc');
			$dfmt=$this->getProperty('ds_format','{field:cat.label}',false);
			$fieldVal=$this->getProperty('ds_field_value','cat.value',false);
			$list = new NxChoiceList_ds($this,$listName,$dir,$ds,null,$dfmt,$fieldVal);
		}
		else
			$list = new NxChoiceList2($this,$listName,$dir,'','','');
			
		return $list;
	}

}

// ============================== DATA FIELDS ==============================
/** Data field interface */
class NxData_Field
{
	var $desc;
	var $errorMsg;
	
	function &NxData_Field(&$desc)
	{
		$this->desc =& $desc;
	}
	
	function getName() 
	{
		return $this->desc->name;
	}
	
	function & getFName()
	{
		return $this->desc->getProperty("ufname","",false);
	}	
	
	function & getLabel($lang=null)
	{
		return $this->desc->getLabel($lang);
	}
	
	function & getPrompt($default=null)
	{
		return $this->desc->getPrompt($default);
	}	
	
	function isEdit()
	{
		return $this->desc->isEdit();		
	}
	
	function isHidden()
	{
		return $this->desc->isHidden();
	}

	function isInput()
	{
		return $this->desc->isInput();
	}
	
	function isRequired()
	{
		return $this->desc->isRequired();
	}
	
	function onSubmitCheck($frm)
	{
		$desc=$this->desc;
		
		// get onSubmitCheck handler (of the form of sprintf format string, with 
		//  param 1: js form object  
		//  param 2: label (string)
		//  param 3: isRequired (bool)
		if ($desc->isEdit())
		{
			$checker = $desc->getProperty("onSubmitCheck",null,false);
			if ($checker != null)
			{
				$label =& $desc->props["label"];
				$label=preg_replace("/[']/","\\'",$label);
				return sprintf($checker,$frm,$desc->name,$label,$desc->isRequired()?"true":"false");
			}
		}
		return "";
	}
	
	function isKey()
	{
		return $this->desc->isKey();
	}
	
	// function called when deleting a record from a repository (such as DB)
	function onDelete(&$errorCB)
	{
	}
	
	// returns a cloned verison of the field. Clones resources as well.
	function clone_it(&$errorCB)
	{
		return $this;
	}

	function getProperty($name,$deft=null,$cascade=true)
	{
		return $this->desc->getProperty($name,$deft,$cascade);
	}

	function addError(&$errorCB,$errorType, $p1="",$p2="",$p3="")
	{
		$list = $this->desc->getChoiceList("FieldErrors");
		$msg = sprintf($list->getChoice($errorType),$p1,$p2,$p3);
		$this->errorMsg = $msg;
		$label = $this->getLabel();
		$errorCB->addError("\"".$label."\" : ".$msg);
	}
	
	function isOk()
	{
		return !isset($this->errorMsg);
	}
	function error()
	{
		return isset($this->errorMsg)?$this->errorMsg:'';
	}

	// process default value and returns it
	function getDefaultValue()
	{
		if (($v = $this->desc->getQProperty('value',null,false))!==null)
			return $v;
		else
			return $this->desc->getQProperty('default',null,false);
	}
	
	// set field data from "default" or "value" property
	function setDefault($opts=null)
	{ 
		$v=$this->getDefaultValue();

		if ($v === null)
			return false;

		// parse value
		return $this->setValue($v);
	}

	// set field data from a given value, return true is value set or false otherwise
	function setValue($v) { 
		return false;
	}
	
	function toStatus() { 
		return isset($this->errorMsg)?'error':'ok';
	}
	
	// functions to overload
	function clear() {}
	function readFromDB($record){}
	function & toDB($opts=null) 			{return "NULL";}
	function toHTML($opts=null) 			{return "n/a"; }
	function toString($opts=null) 		{return $this->toHTML($opts); }
	function toForm($opts=null) 			{return "n/a"; }
	function readFromForm(&$recData,&$errorCB)
	{
		// try to pick the value from "value" property
		$v = $this->desc->getQProperty("value",null,false); 
		if ($v!==null && $this->setValue($v)!==false)
		{
			return true;
		}
		
		// otherwise is no input, just set default property
		if (!$this->desc->isInput())
		{
			$this->setDefault();
			return true;
		}
		else
		{
			// or get data from form
			$res = $this->_readFromForm($recData,$errorCB);
			if  ($errorCB->isError())
			{
				nxError("cant read form for field: {$this->desc->name}",'NX_DATA');
			}
			
			// security patch XSS
			if($res)
				$this->value=preg_replace('/<([.]?)script[^>]+>/i','[$1script removed]',$this->value);

			return $res;	
		}
	}
	
	function _readFromForm(&$recData,&$errorCB)
	{
		nxError('_readFromForm not implemented for :'.get_class($this));
		return false;
	}

	function toStore()			{return $this->toObject(); }
	function readFromObject($v)	{
		return $this->readFromStore($v); 
	}
	function readFromStore()	{}
	
	// return a field alias hiding actual field name (default to name)
	function getAlias()			{
		return $this->desc->getProperty("alias",$this->desc->name,false);
	}
	
	function toUrlParam()		{
		// TODO: check if this should be a raw string instead of HTML...
		return $this->getAlias() . "=" . rawurlencode($this->toString());
	}

	function toList($options) { 
		if ($options=='js' || $options=='json')
			return '[]';
		return array();
	}
		
	// utility
	function getFieldData(&$recData,$fname,$deft)
	{
		if (isset($recData[$fname]))
			return $recData[$fname];
		else 
			return $deft;
	}
	
	// encoding a sub object stored in DB as a url (ex. images params)
	function serialise(& $o)
	{
		if ($o===null)
			return null;
		$url = "";
		foreach ($o as $k=>$v)
			$url.= "$k=".urlencode($v)."&";
			
		return $url;
	}

	function &unserialise($str)
	{
		parse_str($str, $a);
		$o = array();
		foreach ($a as $k => $v)
			$o[$k] = urldecode($v);
			
		return $o;
	}
	
	function is_transient() {
		return false;
	}
	
	function is_pseudo() {
		return false;
	}
}

/** Data field : text */
class NxDataField_text extends NxData_Field
{
	var $value;

	function &NxDataField_text(&$desc)
	{
		$this->desc =& $desc;
		$this->value = '';
	}

	function clear()
	{
		$this->value = '';
	}

	function setValue($deft)
	{
		if (!isset($deft) || $deft === null)
		{
			$this->value = '';
			return false;
		}

		$this->value = $deft;
		return true;
	}

	function & toObject($opts=null)
	{ 
		if ($opts=='format')
			if ($opts=$this->desc->getProperty('format','',false))
				return sprintf($opts,$this->value);

		return $this->value;
	}

	function readFromStore(&$v){
		$this->value = $v;
	}

	function readFromDB($recData)
	{
		$this->value = $this->getFieldData($recData,$this->getFName(),"");
	}

	function & toDB($opts=null) 
	{
		return "'".$this->value."'";
	}

	function toString($opts=null) {
		if (isset($this->value) && ($this->value !== '' || $this->value === 0))
			// return preg_replace("/&#0?39;/","'",$this->value);
			return preg_replace("/(\n|\r)/",' ',html_entity_decode($this->value,ENT_QUOTES));
		else
			return '';
	}
	
	function toJS($opts=null)
	{
		$src=$this->toHTML();
		return preg_replace(array("#(\n|\r)+#","#'+#"),array('',''),$src);
	}

	function toStatus() { 
		return isset($this->errorMsg)?'error': (empty($this->value)?'empty':'ok');
	}
	
	function toHTML($opts=null,$default='&nbsp;')
	{
		if (!isset($this->value))
		{
			// nxError('undefined value for field '.$this->desc->getName().' type:'.get_class($this));
			return $default;
		}
		
		if ($this->value !== '' || $this->value === 0)
		{
			if ($opts)
				return sprintf($opts,$this->value);
			else
				return $this->value;
		}
		else
			return $default;
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		// appear in the form
		if (!$desc->isInput())
			return '';
				
		// is hidden field
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$this->value."\" />";

		// is hidden field + display?
		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$this->value."\" />";			

		// get control class (ie text, telephone, email, etc.)
		$control =& $desc->getControl($desc->getProperty("control","text",false));
		return $control->toForm($this->value,$this);
	}
	
	function toFormat($opts='')
	{
		if ($opts=='')
			$opts=$desc->getProperty('format','',false);
			
		return $this->toHTML($opts);
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = $this->desc;
		$control = $desc->getControl($desc->getProperty("control","text",false));
		
		// read control value
		$this->value = $control->readForm($recData,$this,$errorCB);
		if ($errorCB->isError())
			return false;
			
		if (($this->value == null) && $this->desc->isRequired())
		{
			$this->addError($errorCB, 'empty');
			return false;
		}
		
		return true;
/*		
		
		$this->value = nl2br(htmlspecialchars($this->getFieldData($recData,$this->desc->name,""),ENT_QUOTES));

		if (($this->value == "") && $this->desc->isRequired())
		{
			$this->addError($errorCB, "empty");
			return false;
		}
		
		return true;
*/
	}
}

</script>

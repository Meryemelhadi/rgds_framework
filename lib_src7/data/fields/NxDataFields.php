<?php
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
include_once('lists/NxChoiceList_ds.inc');

$GLOBALS['DATA_ENUM_vals']=array();
$GLOBALS['DATA_ENUM_lists']=array();
$GLOBALS['FORCE_NO_EDIT']=null;
$GLOBALS['DATA_ENUM_fmt']=array();

// ======================= Field Descriptor =================================
class NxFieldDesc extends NxProperties
{
	var $name;

	function __construct($props,$parent,$config=null)
	{
		parent::__construct($props,$parent);
		$this->name = $this->getProperty('name','',false);

		if($config  && isset($config[$this->name]))
		{
			$this->props = $config[$this->name] + $this->props;
		}
	}

	/* magic getter */
	function __get($f) {
		if (method_exists ( $this , $tf="get$f" ) || method_exists ( $this , $tf="get$f" ))
		{
			$res=$this->$tf();
			return $res;
		}
		return null;
	}

	function createField(&$recData, $dataFormat,&$errorCB_,&$record)
	{
		$errorCB=new NxErrorCB();
		
		if ($this->props['type'] == '')
			die('error in dataset: no "type" attribute defined for field '.$this->props['name']);

		// create field
		$className = 'NxDataField_' . $this->props['type'];

		// load field handler class (if already loaded, does nothing
		if (!class_exists($className))
		{
			$success=require_once(NX_LIB."data/fields/{$className}.php");
			if ($success===false)
				$success=require_once(NX_CONF."data/fields/{$className}.php");
		}
		
		$field = new $className($this);
		$field->attachRecord($record);
		
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
				else {
					$fdata = $recData[$this->name];
					$field->readFromStore($fdata);
				}
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

	function getName()
	{
		return $this->name;
	}

	/** returns the prompt if any. check localised version in that order: 
	 *  localised "field.prompt:field name", localised field "prompt" property, "prompt" property */
	function getPrompt($default=null)
	{
		if (!$default)
			$default = $this->getProperty('prompt',null,false);
		
//		$key='field.prompt:'.$this->name;
        $key='field.prompt:'.$this->getProperty('srcName',$this->name,false);
        
		$prompt=$this->getString($key,null,null,$default,$default);
		if ($prompt==$key)
			return null;
		return $prompt;
	}

	function getPrompt_searchEmpty()
	{
		if(! ($default = $this->getProperty('prompt_search_empty',null,false)))
			return null;
		
        $key='field.prompt_search_empty:'.$this->getProperty('srcName','',false);
        
		$prompt=$this->getString($key,null,null,$default,$default);
		if ($prompt==$key)
			return null;
		return $prompt;
	}

	
	function getFormName()
	{
		$fname=$this->getProperty("alias",$this->name,false);
		if (!$this->getProperty("multiRecords",false,false))
			return $fname;
		else
			return "{$fname}[]";
	}
	
	function getType()
	{
		return $this->props['type'];
	}
	
	function getAlias($short=false)	{
		if($res = $this->getProperty('alias',null,false))
			return $res;

		if ($short)
			return $this->getProperty('srcName',$this->name,false);
		else
			return $this->name;
	}
	
	function getLabel($lang=null)
	{
		$label =& $this->props['label'];

		// check if lang enabled
		if (!is_array($label))
		{		
			return $this->getString("field:$label",$lang,null,$label,"field:$this->name");
		}

		// if default lang, get it from properties
		if ($lang == null)
			$lang = $this->getProperty('lang',null);
			
		// if lang doesn't exist for label, fallback on first entry in the array
		if (isset($label[$lang]))
			return $label[$lang];
		else
			return reset($label);			
	}

	function isRequired()
	{
		return $this->getProperty('isRequired',false,false);
	}

	// can be edited in form
	function isEdit()
	{
        // overload edit mode in sub forms (if parent form not editable)
		if($GLOBALS['FORCE_NO_EDIT']===true)
			return false;

		return (boolean)$this->getProperty('isEdit',true,false);
	}
	
	// hidden field
	function isHidden()
	{
		return ! $this->getProperty('show',true,false);
	}

	function isPerm()
	{		
		if (($perm = $this->getProperty('permission',null, false)) && !($GLOBALS['_NX_user']->checkPerm($perm)))
		{
			return false;
		}

		return true;
	}

	// to be part of form at all
	function isInput()
	{
		return $this->getProperty('isInput',true,false);
	}

	function isKey()
	{
		return $this->getProperty('isKey',null,false)!=null;
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
				// $label =& $this->props["label"];
				$label = $this->getLabel();
				$label=preg_replace("/[']/","\\'",$label);
				// onSubmitCheck="checkRCS('%s','%s','%s',%s)"
				return sprintf($checker,$frm,$this->name,$label,$this->isRequired()?"true":"false");
			}
		}
		return "";
	}
	
	function getControl($control="select")
	{
		$controlClass = "NxControl_".$control;

		if (!class_exists($controlClass))
			if (is_file($f=NX_CONF."aspects/fields/controls/{$controlClass}.php") ||
				is_file($f=NX_LIB."data/fields/controls/{$controlClass}.php"))
				require_once($f);
			else
				if($f = $this->getConfFile('lib/aspects/fields/controls',$controlClass.'.php'))
					require_once($f);
				else
					nxError("unknown field control $control of class $controlClass",'NX_DATA');
					
		return new $controlClass();
	}
	
	function getDefaultValue()
	{
		if (($v = $this->getQProperty('value',null,false))!==null)
			return $v;
		else
			return $this->getQProperty('default',null,false);
	}
	
	function keepValue($v) {
		$v=trim($v);
		if(!$v)
			return;
			
		$ds=$this->getProperty('ds',null,false);
		if ($ds && !isset($GLOBALS['DATA_ENUM_vals'][$ds][$v]))
		{
			$GLOBALS['DATA_ENUM_vals'][$ds][$v]=$v;

			if (isset($GLOBALS['DATA_ENUM_lists'][$ds]))
				unset($GLOBALS['DATA_ENUM_lists'][$ds]);
		}
	}

	function getEnumValues() {
		$ds=$this->getProperty('ds',null,false);
		if (!$ds)
			return null;

		if (!isset($GLOBALS['DATA_ENUM_vals'][$ds]))
			return '';

		return preg_replace('/[,|;]+/',"','",implode(',',$GLOBALS['DATA_ENUM_vals'][$ds]));
	}

	function getList($listName=null,$reload=false) 
	{
		$ds=$this->getProperty('ds',null,false);
        $id = $ds;
        if(isset($this->props['ds_format']))
            $id .= $this->props['ds_format'];
        
        if(!$reload)
		    if (($id!=null) && isset($GLOBALS['DATA_ENUM_lists'][$id]))
			    return $GLOBALS['DATA_ENUM_lists'][$id];

        if ($listName==null)
        {
            $listName = $this->getProperty("list",$this->props['name'],false);

            if ($ds)
                $listName.=preg_replace('/[:]/','_',$ds);
        }
        		
		$dir=$this->getProperty('dir','lists',false);
		

		if ($ds)
		{
//			include_once('lists/NxChoiceList_ds.inc');
			$dfmt=$this->getProperty('ds_format','{field:cat.label}',false);
			$fieldVal=$this->getProperty('ds_field_value','cat.value',false);

			$this->parent->parent->setProperty('cat.enum',$this->getEnumValues());
			$list = new NxChoiceList_ds($this,$listName,$dir,$ds,null,$dfmt,$fieldVal);
			if(!$reload)
				$GLOBALS['DATA_ENUM_lists'][$id] = $list;
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
	var $record;
	var $isClone=false;
	
	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}
	
	function attachRecord(&$record){
		$this->record=&$record;
	}

	function getName() 
	{
		return $this->desc->name;
	}
	
	function getAlias()	{
		if (isset($this->record->recordPrefix))
			$prefix=$this->record->recordPrefix;
		else
			$prefix='';
			
		return $prefix.$this->desc->getProperty('alias',$this->desc->name,false);
	}
	
	function getFormName()
	{
		$fname=$this->getAlias();
		if (!$this->desc->getProperty("multiRecords",false,false))
			return $fname;
		else
			return "{$fname}[]";
	}
		
	function getFName()
	{
		return $this->desc->getProperty("ufname","",false);
	}	
	
	function getLabel($lang=null)
	{
		return $this->desc->getLabel($lang);
	}
	
	function getPrompt($default=null)
	{
		return $this->desc->getPrompt($default);
	}	
	
	function isEdit()
	{
		return $this->desc->isEdit();		
	}
	
	function isPerm()
	{		
		return $this->desc->isPerm();
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
				//$label =& $desc->props["label"];
				$label = $this->getLabel();
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
	
	// function called when creating new record from a repository (such as DB)
	function onNew($oid,&$errorCB)
	{
	}
	// function called when deleting a record from a repository (such as DB)
	function onDelete(&$errorCB)
	{
	}
	// function called when deleting a record from a repository (such as DB)
	function onSave($isInsert,&$errorCB)
	{
	}
	
	// returns a cloned verison of the field. Clones resources as well.
	function clone_it(&$record, &$errorCB,$deepCopy=true)
	{
		if (version_compare(phpversion(), '5.0') < 0) {
			//$f=unserialize(serialize($this));
			$f=$this;
		}
		else
		{
			$f=clone($this);
		}
		
		$f->attachRecord($record);
		return $f;
	}

	function getProperty($name,$deft=null,$cascade=true)
	{
		return $this->desc->getProperty($name,$deft,$cascade);
	}
	function setProperty($name,$v)
	{
		if(!isset($this->isClone))
		{
			$this->desc = clone($this->desc);
			$this->isClone = true;
		}

		return $this->desc->setProperty($name,$v);
	}

	function addError(&$errorCB,$errorType, $p1="",$p2="",$p3="")
	{
		$list = $this->desc->getChoiceList("FieldErrors");
		$msg = sprintf($list->getChoice($errorType),$p1,$p2,$p3);
		$this->errorMsg = $msg;
		$label = $this->getLabel();
		if (isset($errorCB))
			$errorCB->addError("\"".$label."\" : ".$msg);
		else
			nxError("\"".$label."\" : ".$msg);
	}
	
	function isOk()
	{
		return !isset($this->errorMsg);
	}
	function error()
	{
		return isset($this->errorMsg)?$this->errorMsg:'';
	}
	function is_transient() {
		return false;
	}
	function is_pseudo() {
		return false;
	}
	function isEmpty()
	{
		return !isset($this->value)||!$this->value;
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
	function readFromDB($record)	{}
	function toDB($opts=null) 	{return "NULL";}
	function toHTML($opts=null) 	{return "n/a"; }
	function toJS($opts=null)		{return $this->toHTML();	}
	function toString($opts=null) 	{return $this->toHTML($opts); }
	function toForm($opts=null) 	{return "n/a"; }
	function toValue() 				{return null; }
	function toBin($opts=null) 	{return null; }
	function toRecords() 		{ return null; }
	
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
	function readFromStore(&$v)	{}

	function readFromJSON($json)
	{
		;
	}
	function toJSON($opts=null) 
	{
		return null;
	}

	function toUrlParam()		{
		// TODO: check if this should be a raw string instead of HTML...
		return $this->desc->getAlias(true) . "=" . rawurlencode($this->toString());
	}

	function toList($options) { 
		if ($options=='js' || $options=='json')
			return '[]';
		return array();
	}
		
	// utility
	function getFieldData(&$recData,$fname,$deft='')
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
	
	/* magic getter */
	function __get($f) {
		if (method_exists ( $this , $tf="to$f" ) || method_exists ( $this , $tf="get$f" ) || method_exists ($this,$tf=$f))
		{
			$res=$this->$tf();
			if ($tf=='todb')
				$res=trim($res,'\'');
			return $res;
		}
		return null;
	}

	function __set($f,$v) {
		if($f=='db')
			$v=nl2br(htmlspecialchars($v,ENT_QUOTES));
		elseif($f=='json')
			return $this->readFromJSON($v);

		$this->setValue($v);
	}

	function __toString() {
		return $this->toString();
	}
}

/** Data field : text */
class NxDataField_text extends NxData_Field
{
	var $value;

	function __construct(&$desc)
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
		$this->value = $this->getFieldData($recData,$this->getFName(),'');
	}

	function toDB($opts=null) 
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

	function readFromJSON($json)
	{
		$json2 = preg_replace(array('/\\\u([0-9a-z]{4})/',"/[']/",'/\\\"/'), array('&#x$1;',"\\'",'\\\\\\"'), $json);
		// nxLog("SAVE JSON : [$json2]",'JSON');
		$this->value = $json2;
	}

	function toJSON($opts=null) 
	{
		$json = str_replace("\n",'\\n',$this->value);
		// nxLog("RED JSON : [$json]",'JSON');
		$json2 = html_entity_decode($json,ENT_NOQUOTES,'UTF-8');
		return $json2;
	}
	
	function toJS($opts='no br')
	{
		$src=$this->toHTML();
		if($opts=='no_br' || $opts=='no br' || $opts=='no br')
			return preg_replace(array("#(\n|\r|<br */?>)+#","#'+#",'#"#'),array('','','\\"'),$src);
		else
			return preg_replace(array("#(\n|\r)+#","#'+#",'#"#'),array('','','\\"'),$src);
	}

	function toStatus() { 
		return isset($this->errorMsg)?'error': ((empty($this->value)||$this->value=='&nbsp;')?'empty':'ok');
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
				$v = sprintf($opts,$this->value);
			else
				$v = $this->value;
		}
		else
			$v = $default;

		// get control class (ie text, telephone, email, etc.)
		/*
		$c = $this->desc->getProperty("control_html",null,false);
		if ($c) {
			$control = $this->desc->getControl();
			return $control->toHTML($v,$this);
		}
		else
		*/
		$v = preg_replace("%&lt;br[ /]*&gt;%", ' ', $v);

		// try appling i8n string to value (useful for workflow states)
		if($this->desc->getProperty('i8n','false',false)=='true')
			$v = __($v);

		return $v;
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;
		$fname = $this->getAlias();

		// appear in the form
		if (!$desc->isInput())
			return '';
			
		/*
		// is hidden field
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$this->value."\" />";

		// is hidden field + display?
		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value=\"".$this->value."\" />";			
		*/

		// get control class (ie text, telephone, email, etc.)
		$control = $desc->getControl($desc->getProperty("control","text",false));
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
		$value = $control->readForm($recData,$this,$errorCB);
		$this->value = $value;
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

class NxDataField_phone extends NxDataField_text
{
	function toHTML($opts=null,$default='&nbsp;')
	{
		if (!isset($this->value))
		{
			// nxError('undefined value for field '.$this->desc->getName().' type:'.get_class($this));
			$v=$default;
		}
		elseif ($this->value !== '' || $this->value === 0)
		{
			if ($opts)
				$v = sprintf($opts,$this->value);
			else
				$v = $this->value;
		}
		else
			$v = $default;

		$desc = $this->desc;
		$control = $desc->getControl($desc->getProperty("control","local_phone",false));
		
		// read control value
		return $control->toHTML($v,$this);
	}
}

?>
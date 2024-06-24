<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : choice list */
class NxDataField_records extends NxData_Field
{
	var $records;
	var $value='';
	var $newRecords=array();

	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}

	function toStatus() {
		return true;
	}
	
	function clear()
	{
		$this->records=null;
	}
		
	// process default value and returns it
	function getDefaultValue()
	{
		return '';
	}
	
	function setValue($deft)
	{	
		$this->records=$deft;
		return true;
	}
	
	function toUrlParam()		{
		return '';
		/*
		if ($this->records)
			return $this->getAlias() . "=" . rawurlencode($this->records->getUrlKey(true));
		else
			return '';
			*/
	}

	function & toObject($opts=null)
	{ 
		return $this->$records; 
	}
	
	function readFromStore(&$v){
		// $this->records = $v;

	}
	
	function readFromDB($recData)
	{
		//$this->records = $this->getFromDS();
		$desc = & $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));
		$control->loadFromDB($this);
	}

	function toDB($opts=null)
	{	
		return '';
		$desc = & $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));
		$control->storeToDB($this,'insert');
	}

	function toString($opts=null)
	{
		// return '';
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = & $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));

		return $control->toHTML($this);
	}
	
	/* apply a format on the choice list. Format is list of the form:
			array('empty'=>'%s:empty','selected'=>'%s:ok') : display all items
		or
			array('first'=>'[%s','last'=>'%s]','others'=>',%s','unique'=>'[%s]') */
	
	function toHTML($opts=null)
	{		
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = & $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));

		return $control->toHTML($this);
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return "";

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$this->value."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$this->value."\" />";			
		
		// get control class (ie combo, list, radio, tick, etc.)
		$control = $desc->getControl($desc->getProperty("control","records",false));
		return $control->toForm($this->value,$this);
	}
	
	function toRecords() {
		$desc = & $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));
		return $control->toRecords($this);
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));
		
		// read control value
		$this->value = $control->readForm($recData,$this);

		if ($this->value == null)
		{
			$this->value = TC_NO_SELECTION;
			if ($this->desc->isRequired())
			{
				$this->addError($errorCB, "not selected");
				return false;
			}
		}
		
		return true;
	}
	
	function onDelete(&$errorCB)
	{
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));
		
		// read control value
		$control->onDelete($this,$errorCB);
	}
	
	// function called when creating new record from a repository (such as DB)
	function onNew($oid,&$errorCB)
	{
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = $this->desc;
		$control = $desc->getControl($desc->getProperty("control","records",false));
		
		// read control value
		$control->onNew($this,$oid);
	}
}

?>
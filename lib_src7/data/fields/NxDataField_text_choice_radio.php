<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : choice list */
class NxDataField_text_choice_radio extends NxDataField_text_choice
{
	var $listName;
	var $value;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$this->listName = $this->desc->getProperty("list","",false);
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return "";
				
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$this->value."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$this->value."\" />";			
		// get choice list array (as "value" => "label")
		
		$list =& $desc->getList();
		$a =& $list->getListAsArray();
		
		// get control class (ie combo, list, radio, tick, etc.)
		$controlClass = "NxControl_".$desc->getProperty("control","combo",false);
		if (!class_exists($controlClass))
			require_once(NX_LIB.$controlClass.".inc");
		$control = new $controlClass();
	
		$desc->getProperty("multipleChoices",false,false);
			
		$style = $desc->getStyle("select","choice");
		return $list->getFormList($desc->name,$this->value,$style,$desc->getProperty("multipleChoices",false,false));
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$this->value = 	$this->getFieldData($recData,$this->desc->name,TC_NO_SELECTION);

		if (($this->value === TC_NO_SELECTION) && $this->desc->isRequired())
		{
			$this->addError($errorCB, "not selected");
			return false;
		}
		
		return true;
	}
}

?>
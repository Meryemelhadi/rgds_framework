<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_checkbox_boolean
{
	function __construct()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm($value,&$desc)
	{
		// get choice list array (as "value" => "label")
		$fname = $this->getAlias();
				
		$isChecked='';
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			if (isset($selected_options[0]) && $selected_options[0]==1)
				$isChecked=' checked="true" ';
		}
		else
			$value='';
			
		return '<span><input class="checkbox" type="checkbox" name="'.$fname.'[]" value="'.$value.'"'.$isChecked.'></input></span>';
	}
	
	function readForm(&$recData,&$desc)
	{
		// $fname = $desc->name;
		$fname = $this->getAlias();
		
		if (!isset($recData[$fname]))
			return null;
			
		$values = $recData[$fname];
		if (is_array($values))
			$value = implode($values,$desc->sep);
		else
			$value = $values;
					
		// remap false value
		if (empty($value))
			$value='0'; // false
		else
			$value='1'; // true

		return $value;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_select
{
	function NxControl_select()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm($value,&$desc)
	{
		// get choice list array (as "value" => "label")
		$list =& $desc->getList();
		$listArray =& $list->getListAsArray();
		$prompt=$desc->getPrompt();
		$prompt_value=$desc->getProperty('prompt_value','?',false);

		$style = $desc->getStyle("select","choice");
		$isMultiple=$desc->getProperty("multiple",false,false);
		// $fname = $desc->name;
		$fname = $desc->getAlias();
		
		$sel = array();
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}
		
		if ($isMultiple)
		{
			$isMultiple = " multiple=\"true\"";
			$fname.='[]';
		}
		else
			$isMultiple = "";

		$res = "<select class=\"select\" ".$isMultiple." name=\"".$fname."\" ".$style.">";
		if ($prompt	&& !isset($listArray[$prompt_value]))
		{
			$res .= "<option value=\"$prompt_value\">".$prompt."</option>";
		}
		foreach (array_keys($listArray) as $k) 
		{
			$lab = &$listArray[$k];
			$res .= "<option value=\"$k\" ". (isset($sel[$k]) ? "selected=\"true\"" : "").">".$lab."</option>";
		}
		return $res."</select>";
	}

	function readForm(&$recData,&$desc)
	{
		// $fname = $desc->name;
		$fname = $desc->getAlias();
		
		if (!isset($recData[$fname]))
			return null;
			
		$values = $recData[$fname];
		if (is_array($values))
			$value = implode($values,$this->desc->sep);
		else
			$value = $values;
			
		return $value;
	}
}

?>
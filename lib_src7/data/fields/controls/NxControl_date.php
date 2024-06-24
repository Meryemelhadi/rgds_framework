<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxControl_date
{
	function __construct()	{}
	function toHTML() {}
		
	function toForm(&$f)
	{
		$desc=&$f->desc;
		$falias=$fname = $f->getAlias();
	
		if (!$desc->isEdit() || $desc->isHidden())
		{
			$res = 
			"<input type=\"hidden\" name=\"".$falias."_day"."\" value=\"".$f->day->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_month"."\" value=\"".$f->month->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_year"."\" value=\"".$f->year->value."\"/>";

			if (!$desc->isEdit() && !$desc->isHidden())
				$res .= $f->toHTML();
				
			return $res;
		}

		$style = $desc->getStyle("input","date");
	
		$res = "<span class=\"input_date\" nowrap=\"true\" ".$style.">";
		$res .= $f->day->toForm();
		
		// $list =& $desc->getChoiceList("months");
		// $res .= $list->getFormList($f->falias."_month",$f->month);
		$res .= $f->month->toForm();
		$res .= $f->year->toForm() . "</span>";

		return $res;
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		$falias=$fname = $f->getAlias();

		if (isset($recData[$falias]) && $recData[$falias]!='' && ($date=$f->strptime($recData[$falias])!==false)) {
			$deftdate = getdate($date); 
			$f->day->value = $deftdate["mday"];
			$f->month->value = (string)(intval($deftdate["mon"])); 
			$f->year->value = $deftdate["year"]; 
		}
		else {
			$f->day->value = $f->getFieldData($recData,$falias."_day",TC_NO_SELECTION);
			$f->month->value = $f->getFieldData($recData,$falias."_month",TC_NO_SELECTION);
			$f->year->value = $f->getFieldData($recData,$falias."_year",TC_NO_SELECTION);
		}
		
		if ($f->desc->isRequired())
		{
			$fields = null;
			if ($f->day->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $f->desc->getString("day");		
			}
			
			if ($f->month->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $f->desc->getString("month");
			}

			if ($f->year->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $f->desc->getString("year");
			}

			// check fields have been entered
			if (isset($fields))
			{
				$f->addError($errorCB, "incomplete date", $fields);
				return false;
			}
			
			// check date existence
			if (!checkdate((int)$f->month->value,(int)$f->day->value, (int)$f->year->value))
			{
				$f->addError($errorCB, "invalid date", $fields);
				return false;
			}
		}

		return true;
		
	}
}

?>
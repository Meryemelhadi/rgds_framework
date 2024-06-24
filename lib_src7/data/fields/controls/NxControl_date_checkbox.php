<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxControl_date_checkbox_on_off
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
			return true;
	}
	
	function toForm(&$f,$controlName,$opts=null)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();

		$locale=$desc->getProperty('lang','en');
		if ($locale=='fr')
			$locale='fr_FR';

		$dayV = $f->day->value;
		if ($dayV=='?' || $dayV=='-' || $dayV==='')
			$inputVal='';
		else
		{
			if ($locale=='fr_FR')
				$inputVal = str_pad($f->day->value, 2, '0', STR_PAD_LEFT).'/'.str_pad($f->month->value, 2, '0', STR_PAD_LEFT).'/'.$f->year->value;
			else
				$inputVal = str_pad($f->month->value, 2, '0', STR_PAD_LEFT).'-'.str_pad($f->day->value, 2, '0', STR_PAD_LEFT).'-'.$f->year->value;
		}
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $f->toHTML($opts).'<input type="hidden" name="'.$fname."\" value='".$inputVal."' />";

//		return '<input type="checkbox" name="'.$fname."\" value='".$inputVal."' />".'<label for="'.$fname.'">'.$f->toHTML($opts).'</label>';
		return '<input class="checkbox" type="checkbox" name="'.$fname."\" value='".$inputVal."' />";
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($v = $recData[$fname])=="" || ($v =='-/-/-'))
			$v = "?-?-?";
	
		list($year,$m,$d) = explode("-",str_replace('/','-',$v));

		if (strlen($d)>2)
			list($d,$t)=explode(' ',$d);
			
		// remove "0" heading (date choice-lists are based on numbers)
		$f->month->value = ltrim($m,"0");
		$f->day->value = ltrim($d,"0");	
		$f->year->value = ltrim($year,"0");	
		
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
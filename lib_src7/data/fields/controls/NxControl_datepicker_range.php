<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!isset($GLOBALS['form_datepicker']))
	$GLOBALS['form_datepicker']=false;

class NxControl_datepicker
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_datepicker'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/datepicker/datepicker.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/datepicker/',NX_DOC_ROOT.'nx/controls/datepicker/');
		}
		$GLOBALS['form_datepicker']=true;

		return false;
	}
	
	function toForm(&$f,$controlName,$opts=null)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();

		$locale=$desc->getProperty('lang','en');
		if ($locale=='fr')
			$locale='fr_FR';

		$arValues = explode('|', $inputVal);	
		$minValue = $arValues[0];
		$maxValue = $arValues[1];		

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

		$s='';

		// add resources once
		if (!$this->init($desc))
		{
			$s.='
	<link rel="stylesheet" href="/nx/controls/datepicker/datepicker.css" />
	<script>
		if (typeof ajax_require != "undefined")
		{
			ajax_require("/nx/controls/datepicker/prototype-date-extensions.js","prototype-date-extensions");
			ajax_require("/nx/controls/datepicker/datepicker.js","datepicker");
		}
	</script>
';
		}
		
		$s.=<<<EOF
		<input id="{$fname}" name="{$fname}"  style="width:auto" value="{$inputVal}" xclass="datetimepicker date" xreadonly="readonly"/>
		<script type="text/javascript">
		if (typeof ajax_require != "undefined")
		{
			ajax_onload(function(){
				new window.Control.DatePicker(document.getElementById('{$fname}'), { icon: '/nx/controls/datepicker/images/calendar.png', locale:'{$locale}', timePicker:false, timePickerAdjacent: false, use24hrs: true });
			},"datepicker");
		}
		</script>
EOF;

		return $s;
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($v = $recData[$fname])=="" || ($v =='-/-/-'))
		{
			$v = $f->desc->getQProperty("default",null,false); 
			if ($v==null || $f->setValue($v)==false)
				$v = "?-?-?";
			else
				return true;
		}
	
		$locale=$desc->getProperty('lang','en');
		if ($locale=='fr')
		{
			list($d,$m,$year) = explode("-",str_replace(array('/','.'),'-',$v));
		}
		else
		{
			list($m,$d,$year) = explode("-",str_replace(array('/','.'),'-',$v));
		}

		if($m=='')
			$m=date('m');

		if($year=='')
			$year=date('Y');

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
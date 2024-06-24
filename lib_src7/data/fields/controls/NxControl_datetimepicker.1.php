<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!isset($GLOBALS['form_datepicker']))
	$GLOBALS['form_datepicker']=false;

class NxControl_datetimepicker
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_datepicker'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/datepicker/datepicker.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/datepicker/',NX_DOC_ROOT.'nx/datepicker/');
		}
		$GLOBALS['form_datepicker']=true;

		return false;
	}
	
	function toForm(&$f)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();
	
		$inputVal = str_pad($f->day->value, 2, '0', STR_PAD_LEFT).'/'.str_pad($f->month->value, 2, '0', STR_PAD_LEFT).'/'.$f->year->value .' '.str_pad($f->h->value, 2, '0', STR_PAD_LEFT).':'.str_pad($f->month->mn, 2, '0', STR_PAD_LEFT).':'.$f->s->value;
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $value.'<input type="hidden" name="'.$fname."\" value='".$inputVal."' />";
		
		$s='';

		// add resources once
		if (!$this->init($desc))
		{
			$s.='
	<link rel="stylesheet" href="/nx/datepicker/datepicker.css" />
	<script type="text/javascript" language="javascript" src="/nx/datepicker/prototype-date-extensions.js"></script>
	<script type="text/javascript" language="javascript" src="/nx/datepicker/datepicker.js"></script>
';
		}

		$s.=<<<EOF
		<input id="{$fname}" name="{$fname}"  style="width:auto" value="{$inputVal}" xclass="datetimepicker date" readonly="readonly"/>
		<script type="text/javascript">
			new Control.DatePicker(document.getElementById('{$fname}'), { icon: '/nx/datepicker/images/calendar.png', locale:'fr_FR', timePicker:true, timePickerAdjacent: false, use24hrs: true });
		</script>
EOF;

		return $s;
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		$errorCB=new NxErrorCB();
		
		if (!isset($recData[$fname]) || ($v = $recData[$fname])=="")
			$v = "?-?-?";
				
		if(!preg_match('#([0-9]+)[-/]([0-9]+)[-/]([0-9]+)[ ]+([0-9]+)[:h]([0-9]+)(?:[:mn]+([0-9]+)[s ]*)?#',$recData[$fname],$matches))
		{
			$f->addError($errorCB, "invalid date");
			return false;
		}

		list($x,$d,$m,$year,$h,$mn,$s) = $matches;
		if (strlen($d)>2)
			list($d,$t)=explode(' ',$d);
			
		// remove "0" heading (date choice-lists are based on numbers)
		$f->month->value = ltrim($m,"0");
		$f->day->value = ltrim($d,"0");	
		$f->year->value = ltrim($year,"0");	

		$f->h->value = ltrim($h,"0");
		if($f->h->value=='')
			$f->h->value='0';
			
		$f->mn->value = ltrim($mn,"0");
		if($f->mn->value=='')
			$f->mn->value='0';
			 
		if ($s==0)
			$s='00';
		$f->s->value = ltrim($s,"0");
		if($f->s->value=='')
			$f->s->value='0';
		
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

			// time
			if ($f->h->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $f->desc->getString("hours");
			}
			if ($f->mn->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $f->desc->getString("minutes");
			}
			if ($f->s->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $f->desc->getString("seconds");
			}

			// check fields have been entered
			if (isset($fields))
			{
				$errorCB=new NxErrorCB();
				$f->addError($errorCB, "incomplete date", $fields);
				return false;
			}
			
			// check date existence
			if (!checkdate((int)$f->month->value,(int)$f->day->value, (int)$f->year->value))
			{
				$errorCB=new NxErrorCB();				
				$f->addError($errorCB, "invalid date", $fields);
				return false;
			}
		}

		return true;
	}
}

?>
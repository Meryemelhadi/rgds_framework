<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'data/fields/NxDataField_integerList.php');

/** Data field: date */
class NxDataField_date extends NxData_Field
{
	var $day;
	var $month;
	var $year;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$name = $desc->name;
		
		$this->day = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$name."_day","prompt"=>$this->desc->getString("day",null,'strings'),"style"=>"","type"=>"integerList","min"=>1,"max"=>31),$desc));
		$this->month = new NxDataField_text_choice(
			new NxFieldDesc(array("name"=>$name."_month",
				"type"=>"choiceList",
				"list"=>"months","style"=>""),$desc));	
		$this->year = "";
	}

	function clear()
	{
		$this->day->clear();
		$this->month->clear();
		$this->year = "";
	}

	function setValue($deft)
	{
		if ($deft == null)
			return false;
					
		// read default date value (any kind of format is supported including now, tomorrow, etc.)
		// todo PHP5: add 2nd param with current timestamp as there is a bug (takes now as today at midnight..
		if (($timestamp = $this->strptime($deft)) === -1)
			return false;

		$deftdate = getdate($timestamp); 
		$this->day->value = $deftdate["mday"];
		$this->month->value = (string)(intval($deftdate["mon"])); 
		$this->year = $deftdate["year"]; 
		return true;
	}

	// process default value and returns it
	function getDefaultValue($opts=null)
	{
		$deft = $this->desc->getQProperty('default',null,false);
		if ($deft)
			$time=$this->strptime($deft);
		else
			$time=time();
			
		if ($opts!=null)
			$dateFormat=$opts;
		else
			$dateFormat =& $this->desc->getProperty("format","short");
							
		switch($dateFormat)
		{
			case 'long':
				return strftime($this->desc->getProperty('dateFormatLong','%A, %d %B %Y'),$time);
			case 'unix_s':
				return $time;
			case 'unix_ms':
				return ''.$time.'000';
			case 'short':
				return strftime($this->desc->getProperty("dateFormatShort","%x"),$time);
			default: // '%d %B %Y'
				return strftime($dateFormat,$time);
		}
	}

	function & toObject($opts=null)
	{ 
		return array(
			"day"=>$this->day->value,
			"month"=>$this->month->value,
			"year"=>$this->year); 
	}
	function readFromStore(&$a){
		if (is_array($a))
		{
			$this->day->value = $a["day"];
			$this->month->value = $a["month"];
			$this->year = $a["year"];
		}
		else
			$this->setDefault();
	}
	
	function readFromDB($recData)
	{
		$v = $this->getFieldData($recData,$this->getFName(),"NULL");
		if ($v == "NULL")
		{
			$v = "?-?-?";
		}
		
		
		list($this->year,$m,$d) = explode("-",$v);
		if (strlen($d)>2)
			list($d,$t)=explode(' ',$d);
			
		// remove "0" heading (date choice-lists are based on numbers)
		$this->month->value = ltrim($m,"0");
		$this->day->value = ltrim($d,"0");	
	}

	function & toDB($opts=null) 
	{
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
		{
			return "'{$this->year}-{$this->month->value}-{$this->day->value}'";
		}
		else
		{
			return "NULL";
		}
	}
	
	function toStatus() { 
		return isset($this->errorMsg)?'error': 
			((empty($this->year)&&$this->month->value==0&&$this->day->value)?'empty':'ok');
	}

	function toTime() {
		return $this->toHTML('unix_s');
	}	
	
	function toHTML($opts=null)
	{
		// NB. locale is set up at the page level
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
		{
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat =& $this->desc->getProperty("format","short");
				
			$time=mktime(0,0,0,$this->month->value,$this->day->value,$this->year);
				
			$s=null;
			switch($dateFormat)
			{
				case 'long':
					$s=$this->strftime($this->desc->getProperty('dateFormatLong','%A, %d %B %Y'),$time);
					break;
				case 'unix_s':
					$s= $time;
					break;
				case 'unix_ms':
					$s= ''.$time.'000';
				case 'short':
					$s= $this->strftime($this->desc->getProperty("dateFormatShort","%x"),$time);
					break;
				default: // '%d %B %Y'
					$s= $this->strftime($dateFormat,$time);
					break;
			}
			
			return $s;
		}
		else
		{
			return "n/a";
		}
	}
	
	function toJS($opts=null)
	{
		// NB. locale is set up at the page level
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
		{
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat =& $this->desc->getProperty("format","short");
				
			$time=mktime (0,0,0,$this->month->value,$this->day->value,$this->year);
				
			switch($dateFormat)
			{
				case 'long':
					return $this->strftime($this->desc->getProperty('dateFormatLong','%A, %d %B %Y'),$time);
				case 'unix_s':
					return $time;
				case 'unix_ms':
					return ''.$time.'000';
				default: // '%d %B %Y'
					return $this->strftime('%d %B %Y',$time);
			}
		}
		else
		{
			return "n/a";
		}
	}


	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return "";
		
		$falias=$desc->getAlias();

		if (!$desc->isEdit() || $desc->isHidden())
		{
			$res = 
			"<input type=\"hidden\" name=\"".$falias."_day"."\" value=\"".$this->day->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_month"."\" value=\"".$this->month->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_year"."\" value=\"".$this->year."\"/>";

			if (!$desc->isEdit() && !$desc->isHidden())
				$res .= $this->toHTML();
				
			return $res;
		}
			
		$style = $desc->getStyle("input","date");
	
		$res = "<div class=\"input_date\" nowrap=\"true\" ".$style.">";
		// $list =& $desc->getChoiceList("day-of-month");
		// $res = $list->getFormList($this->falias."_day",$this->day);
		$res .= $this->day->toForm();
		
		// $list =& $desc->getChoiceList("months");
		// $res .= $list->getFormList($this->falias."_month",$this->month);
		$res .= $this->month->toForm();

		$res .= "<input class=\"text \" id=\"input_year\" type=\"text\" name=\"".$falias."_year"."\" size=\"4\" maxlength=\"4\" value=\"".$this->year."\" onkeypress=\"keypress_number(event)\" onpaste=\"keypress_number(event)\"/></div>";
		return $res;
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$falias=$desc->getAlias();

		if (isset($recData[$falias]) && $recData[$falias]!='' && ($date=$this->strptime($recData[$falias])!==false)) {
			$deftdate = getdate($date); 
			$this->day->value = $deftdate["mday"];
			$this->month->value = (string)(intval($deftdate["mon"])); 
			$this->year = $deftdate["year"]; 
		}
		else {
			$this->day->value = $this->getFieldData($recData,$falias."_day",TC_NO_SELECTION);
			$this->month->value = $this->getFieldData($recData,$falias."_month",TC_NO_SELECTION);
			$this->year = $this->getFieldData($recData,$falias."_year",TC_NO_SELECTION);
		}
		
		if ($this->desc->isRequired())
		{
			$fields = null;
			if ($this->day->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $this->desc->getString("day");		
			}
			
			if ($this->month->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $this->desc->getString("month");
			}

			if ($this->year == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ", ";
				else
					$fields = "";
					
				$fields .= $this->desc->getString("year");
			}

			// check fields have been entered
			if (isset($fields))
			{
				$this->addError($errorCB, "incomplete date", $fields);
				return false;
			}
			
			// check date existence
			if (!checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
			{
				$this->addError($errorCB, "invalid date", $fields);
				return false;
			}
		}

		return true;
	}
	
	/** get timestamp from date string, fix pb with european format */
	function strptime($time)
	{
		// fix for european times
		if (function_exists('strptime '))
		{
			$dateFormat=$this->desc->getProperty("dateFormatShort","%x");
			return strptime($time,$dateFormat);
		}

		// fix for european times
		if (preg_match('/^fr/i',setlocale(LC_TIME,0))&&preg_match('&^[0-9]{2}/[0-9]{2}/&',$time))
			// european time -> us time
			$time=preg_replace('&^([0-9]{2})/([0-9]{2})/&','$2/$1/',$time);
					
		// read default date value (any kind of format is supported including now, tomorrow, etc.)
		// todo PHP5: add 2nd param with current timestamp as there is a bug (takes now as today at midnight..
		return strtotime($time,time());			
	}
	
	/** display string from timestamp. Check encoding */
	function strftime($dateFormat,$time, $encode=true) {
		global $NX_CHARSET;
		$s = strftime($dateFormat,$time);
		if ($encode && $NX_CHARSET!='ISO-8859-1') {
			if (function_exists('iconv'))
				$s=iconv("ISO-8859-1",$NX_CHARSET,$s);
			else
				$s=utf8_encode($s);
		}
		return $s;
	}

}

?>
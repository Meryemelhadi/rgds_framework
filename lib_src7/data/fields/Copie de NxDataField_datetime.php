<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'data/fields/NxDataField_integerList.php');

/** Data field: date */
class NxDataField_datetime extends NxData_Field
{
	var $day;
	var $month;
	var $year;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$name = $desc->name;
		
		$this->day = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$name."_day","prompt"=>$this->desc->getString("day",null,'sys_date'),"style"=>"","type"=>"integerList","min"=>1,"max"=>31),$desc));
		$this->month = new NxDataField_text_choice(
			new NxFieldDesc(array("name"=>$name."_month","type"=>"choiceList","list"=>"months","style"=>""),$desc));	
		$this->year = '';
		$this->h='00';
		$this->m='00';
		$this->s='00';
	}

	function clear()
	{
		$this->day->clear();
		$this->month->clear();
		$this->year = '';
		$this->h='00';
		$this->m='00';
		$this->s='00';
	}

	function setValue($deft)
	{
		if ($deft == null)
			return false;

		// read default date value (any kind of format is supported including now, tomorrow, etc.)
		if (($timestamp = $this->strptime($deft)) === -1)
			return false;

		$deftdate = getdate($timestamp); 
		$this->day->value = $deftdate['mday'];
		$this->month->value = (string)(intval($deftdate['mon'])+0); 
		$this->year = $deftdate['year']; 
		$this->h=$deftdate['hours'];
		$this->m=$deftdate['minutes'];
		$this->s=$deftdate['seconds'];
		
		return true;
	}
	
	function toTime() {
		return $this->toHTML('unix_s');
	}
	
	function & toObject($opts=null)
	{ 
		return array(
			'd'=>$this->day->value,
			'm'=>$this->month->value,
			'y'=>$this->year,
			'h'=>$this->h,
			'mn'=>$this->m,
			's'=>$this->s,
			); 
	}
	function readFromStore(&$a){
			$this->day->value = $a['d'];
			$this->month->value = $a['m'];
			$this->year = $a['y'];
			$this->h = $a['h'];
			$this->m = $a['mn'];
			$this->s = $a['s'];
	}
	
	function readFromDB($recData)
	{
		$v = $this->getFieldData($recData,$this->getFName(),"NULL");
		if ($v == 'NULL')
			$v = '?-?-? 00:00:00';
		
		if (preg_match('/([?]|[0-9]{4})-?([?]|[0-9]{2})-?([?]|[0-9]{2})[ ]?([0-2][0-9]):?([0-6][0-9]):?([0-6][0-9])/',
				$v,$matches))
		{
			list($dummy,$this->year,$m,$d,$this->h,$this->m,$this->s)=$matches;
			// remove "0" heading (date choice-lists are based on numbers)
			$this->month->value = ltrim($m,"0");
			$this->day->value = ltrim($d,"0");	
		}
		elseif (preg_match('/([?]|[0-9]{4})-?([?]|[0-9]{2})-?([?]|[0-9]{2})(?:[ ]?([0-2][0-9]):?([0-6][0-9]):?([0-6][0-9]))?/',
				$v,$matches))
		{
			list($dummy,$this->year,$m,$d)=$matches;
			$this->h=$this->m=$this->s=0;
			// remove "0" heading (date choice-lists are based on numbers)
			$this->month->value = ltrim($m,"0");
			$this->day->value = ltrim($d,"0");	
		}
		else
		{
			nxError($this->getFName().' field: bad date time format:'.$v,'NX_DATA');
		}
	}
	
	function & toDB($opts=null) 
	{
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
		{
			return "'{$this->year}-{$this->month->value}-{$this->day->value} {$this->h}:{$this->m}:{$this->s}'";
		}
		else
		{
			return 'NULL';
		}
	}

	function toHTML($opts=null)
	{
		// NB. locale is set up at the page level
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
		{
			$t=mktime ($this->h,$this->m,$this->s,$this->month->value,$this->day->value,$this->year);	
			
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat =& $this->desc->getProperty("format","short");
				
			switch($this->desc->getProperty("format","short"))
			{								
				case 'unix_s':
				case "timestamp":
					return $t;
				case 'unix_ms':
					return ''.$t.'000';
				case "long":
					$fmt = $this->desc->getProperty("dateTimeFormatLong","%A %d %B %Y, %Hh%Mm%S");
					return $this->strftime($fmt,$t);
				case "short":
					$fmt = $this->desc->getProperty("dateTimeFormatShort","%x %X");
					return $this->strftime($fmt,$t);
				default: // '%d %B %Y'
					return $this->strftime($dateFormat,$t);
			}										
		}
		else
		{
			return 'n/a';
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
			"<input type=\"hidden\" name=\"".$falias."_h"."\" value=\"".$this->h."\"/>";
			"<input type=\"hidden\" name=\"".$falias."_m"."\" value=\"".$this->m."\"/>";
			"<input type=\"hidden\" name=\"".$falias."_s"."\" value=\"".$this->s."\"/>";

			if (!$desc->isEdit() && !$desc->isHidden())
				$res .= $this->toHTML();
				
			return $res;
		}
			
		$style = $desc->getStyle("input","datetime");
	
		$res = "<div class=\"input_datetime\" nowrap=\"true\" ".$style.">";
		// $list =& $desc->getChoiceList("day-of-month");
		// $res = $list->getFormList($this->desc->name."_day",$this->day);
		$res .= $this->day->toForm();
		
		// $list =& $desc->getChoiceList("months");
		// $res .= $list->getFormList($this->desc->name."_month",$this->month);
		$res .= $this->month->toForm();

		$res .= 
		'<input class="text input_year" id="input_year" type="text" name="'.$falias.'_year" size="4" maxlength="4" value="'.$this->year.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/> '.

		'<input class="text" id="input_h" type="text input_year" name="'.$falias.'_h" size="2" maxlength="2" value="'.$this->h.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/>:'.
		'<input class="text" id="input_m" type="text" name="'.$falias.'_m" size="2" maxlength="2" value="'.$this->m.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/>:'.
		'<input class="text" id="input_s" type="text" name="'.$falias.'_s" size="2" maxlength="2" value="'.$this->s.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/></div>';
		return $res;
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$falias=$desc->getAlias();
		
		$this->day->value = $this->getFieldData($recData,$falias."_day",TC_NO_SELECTION);
		$this->month->value = $this->getFieldData($recData,$falias."_month",TC_NO_SELECTION);
		$this->year = $this->getFieldData($recData,$falias."_year",TC_NO_SELECTION);

		$this->h = $this->getFieldData($recData,$falias."_h",TC_NO_SELECTION);
		$this->m = $this->getFieldData($recData,$falias."_m",TC_NO_SELECTION);
		$this->s = $this->getFieldData($recData,$falias."_s",0);

		if ($this->desc->isRequired())
		{
			$fields = null;
			if ($this->day->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ', ';
				else
					$fields = '';
					
				$fields .= $this->desc->getString("day");		
			}
			
			if ($this->month->value == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ', ';
				else
					$fields = '';
					
				$fields .= $this->desc->getString("month");
			}

			if ($this->year == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ', ';
				else
					$fields = '';
					
				$fields .= $this->desc->getString("year");
			}

			if ($this->h == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ', ';
				else
					$fields = '';
					
				$fields .= $this->desc->getString("hours");
			}
			if ($this->m == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ', ';
				else
					$fields = '';
					
				$fields .= $this->desc->getString("minutes");
			}
/*			if ($this->s == TC_NO_SELECTION)
			{
				if (isset($fields))
					$fields .= ', ';
				else
					$fields = '';
					
				$fields .= $this->desc->getString("seconds");
			}
*/

			// check fields have been entered
			if (isset($fields))
			{
				$this->addError($errorCB, 'incomplete date/time', $fields);
				return false;
			}
			
			// check date existence
			if (!checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year))
			{
				$this->addError($errorCB, "invalid date", $fields);
				return false;
			}
			if ((int)$this->h>24 || ((int)$this->h>60)||((int)$this->s>60))
			{
				$this->addError($errorCB, "invalid time", $fields);
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
			if (function_exists(iconv))
				$s=iconv("ISO-8859-1",$NX_CHARSET,$s);
			else
				$s=utf8_encode($s);
		}
		return $s;
	}	
}

?>
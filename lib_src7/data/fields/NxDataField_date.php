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

		$deftdate = getdate(time()); 
		$year = $deftdate["year"];
		$falias=$this->getAlias();
	
		$this->day = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$falias."_day","prompt"=>$this->desc->getString("day",null,'strings'),"style"=>"","type"=>"integerList","min"=>1,"max"=>31),$desc));
		$this->month = new NxDataField_text_choice(
			new NxFieldDesc(array("name"=>$falias."_month",
				"type"=>"choiceList",
				"list"=>"months","style"=>""),$desc));	
		$this->year = new NxDataField_integerList(
			new NxFieldDesc(array(
				"name"=>$falias."_year",
				"prompt"=>$this->desc->getString("year",null,'strings'),
				"style"=>"",
				"type"=>"integerList",
				"min"=>$desc->getProperty('min_year',1940,false),
				"max"=>$desc->getProperty('max_year',$year+20,false),
				"increment"=>$desc->getProperty('increment_year',1,false)
			),$desc));
	}
	
	function attachRecord(&$record){
		$this->record=&$record;
		$this->day->attachRecord($record);
		$this->month->attachRecord($record);
		$this->year->attachRecord($record);
	}

	function clear()
	{
		$this->day->clear();
		$this->month->clear();
		$this->year->clear();
	}

	function setValue($deft)
	{
		if ($deft == null)
			return false;
			
		if ($deft=='?-?-?')
		{
			$this->clear();
			return true;
		}
		// read default date value (any kind of format is supported including now, tomorrow, etc.)
		// todo PHP5: add 2nd param with current timestamp as there is a bug (takes now as today at midnight..
		if(is_numeric($deft))
            $timestamp = $deft;
        elseif(is_array($deft) && isset($deft['d']))
        {
            $this->day->value = $deft['d'];
            $this->month->value = $deft['m'];
            $this->year->value = $deft['y']; 
            return true;
        }
        else
			if (($timestamp = $this->strptime($deft)) === -1)
				return false;

		if ($timestamp===FALSE)
		{
			return false;
		}

		$deftdate = getdate($timestamp); 
		$this->day->value = $deftdate["mday"];
		$this->month->value = (string)(intval($deftdate["mon"])); 
		$this->year->value = $deftdate["year"]; 
		return true;
	}

	// process default value and returns it
	function getDefaultValue($opts=null)
	{
		$deft = $this->desc->getQProperty('value',null,false);
		if (!$deft)		
			$deft = $this->desc->getQProperty('default',null,false);
			
		if ($deft)
			$time=$this->strptime($deft);
		else
			return null;
			// $time=time();
			
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

	function toObject($opts=null)
	{ 
		return array(
			"day"=>$this->day->value,
			"month"=>$this->month->value,
			"year"=>$this->year->value,
			"d"=>$this->day->value,
			"m"=>$this->month->value,
			"y"=>$this->year->value
			); 
	}
	
	function readFromStore(&$a)
	{
		if (is_array($a))
		{
			$this->day->value = $a["day"];
			$this->month->value = $a["month"];
			$this->year->value = $a["year"];
		}
		else
			$this->setDefault();
	}
	
	function readFromDB($recData)
	{
		$v = $this->getFieldData($recData,$this->getFName(),"NULL");
		if ($v == "NULL")
		{
			$v=$this->getDefaultValue('%Y-%m-%d');
			if (!$v)
				$v = '?-?-?';
		}

		list($year,$m,$d) = explode("-",$v);
		if (strlen($d)>2)
			list($d,$t)=explode(' ',$d);
			
		// remove "0" heading (date choice-lists are based on numbers)
		$this->month->value = ltrim($m,"0");
		$this->day->value = ltrim($d,"0");	
		$this->year->value = ltrim($year,"0");	
	}

	function toDB($opts=null) 
	{
		if ($this->isOk())
		{
			return "'{$this->year->value}-".str_pad($this->month->value,2,'0',STR_PAD_LEFT).'-'.str_pad($this->day->value,2,'0',STR_PAD_LEFT)."'";
		}
		else
		{
			return "NULL";
		}
	}
	
	function toStatus() { 
		return isset($this->errorMsg)?'error': 
			(
				(empty($this->year->value)&&$this->month->value==0&&$this->day->value)?
				'empty':
				(
					$this->isOk() ? 'ok' : 'error'
				)
			)
			;
	}

	function toTime() {
		return $this->toHTML('unix_s');
	}	

	function isOk() {
		$y = (int)$this->year->value;

		if($y < 1900)
			return false;

		if($y < 2038)
			return checkdate((int)$this->month->value,(int)$this->day->value, $y);

		// $d=new DateTime("{$this->year->value}-{$this->month->value}-{$this->day->value}");

		return true;
	}	
	function isEmpty()
	{
		return !$this->isOk();
	}

	function isEqual($other)
	{
		return 
			$this->day->value==$other->day->value &&
			$this->month->value==$other->month->value &&
			$this->year->value==$other->year->value
			;
	}

	function toUrlParam() {
		return $this->getAlias(true) . "=" . rawurlencode($this->toHTML('%x'));
	}
	
	function toHTML($opts=null)
	{
		$c = $this->desc->getProperty("control_html",null,false);
		if ($c) {
			$control = $this->desc->getControl($c);
			return $control->toHTML($value,$this);
		}

		// NB. locale is set up at the page level
		if ($this->isOk())
		{
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat = $this->desc->getProperty("format","short");

			$dateFormat=$this->desc->getString("format_date_$dateFormat",null,'form_fields',$dateFormat);
				
			if($this->year->value > 2038)
				if($dateFormat!='time')
					return "{$this->day->value}/{$this->month->value}/{$this->year->value}";
				else
					return null;

			$time=mktime(0,0,0,$this->month->value,$this->day->value,$this->year->value);
				
			$s=null;
			switch($dateFormat)
			{
				case 'time':
				case 'unix_s':
					$s= $time;
					break;
				case 'unix_ms':
					$s= ''.$time.'000';
				/*
				case 'long':
					$s=$this->strftime($this->desc->getProperty('dateFormatLong','%A, %d %B %Y'),$time);
					break;
				case 'short':
					$s= $this->strftime($this->desc->getProperty("dateFormatShort","%x"),$time);
					break; */
				default: // '%d %B %Y'
					$s= $this->strftime($dateFormat,$time);
					break;
			}
			
			return $s;
		}
		else
		{
			return $this->desc->getString("format_date_vide",null,'form_fields',"n/a");
		}
	}
	
	function toJS($opts=null)
	{
		// NB. locale is set up at the page level
		if ($this->isOk())
		{
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat =& $this->desc->getProperty("format","short");
				
			$time=mktime (0,0,0,$this->month->value,$this->day->value,$this->year->value);
				
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
		{
			return "";
		}

		if (defined('NX_DATE_CONTROL'))
			$type=NX_DATE_CONTROL;
		else
			$type="datepicker_bs";

		$control = $desc->getControl($desc->getProperty("control",$type,false));
		return $control->toForm($this,$type,$opts);
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$desc = & $this->desc;
		if (defined('NX_DATE_CONTROL'))
			$type=NX_DATE_CONTROL;
		else
			$type="datepicker";
		$control = $desc->getControl($desc->getProperty("control",$type,false));
		return $control->readForm($recData,$this,$type);
	}
	
	/** get timestamp from date string, fix pb with european format */
	function strptime($time)
	{
		// fix for european times
		if (false && function_exists('strptime'))
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
        $s = strftime(utf8_decode($dateFormat),$time);
        if (NX_ENCODE_DATE && $encode && $NX_CHARSET!='ISO-8859-1') {
            if (function_exists('iconv'))
                $s=iconv("ISO-8859-1",$NX_CHARSET,$s);
            else
                $s=utf8_encode($s);
        }
        return $s;
    }    

}

?>

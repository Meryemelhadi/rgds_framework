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

	var $h;
	var $mn;
	var $s;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$name = $desc->name;
		$falias=$this->getAlias();
		
		$this->day = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$falias."_day","prompt"=>$this->desc->getString("day",null,'sys_date'),"style"=>"","type"=>"integerList","min"=>1,"max"=>31),$desc),true);
		$this->month = new NxDataField_text_choice(
			new NxFieldDesc(array("name"=>$falias."_month","type"=>"choiceList","list"=>"months","style"=>""),$desc));	
		$this->year = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$falias."_year","prompt"=>$this->desc->getString("year",null,'strings'),"style"=>"","type"=>"integerList","min"=>1970,"max"=>2010),$desc));
		
		$this->h = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$falias."_h","prompt"=>$this->desc->getString("hour",null,'strings'),"style"=>"","type"=>"integerList","min"=>$desc->getProperty("min-h",0,false),"max"=>$desc->getProperty("max-h",23,false)),$desc),true);
		$this->mn = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$falias."_mn","prompt"=>$this->desc->getString("mn",null,'strings'),"style"=>"","type"=>"integerList","min"=>0,"max"=>59,'increment'=>$desc->getProperty("increment-mn",1,false)),$desc),true);
		
		$this->s = new NxDataField_integerList(
			new NxFieldDesc(array("name"=>$falias."_s","prompt"=>$this->desc->getString("s",null,'strings'),"style"=>"","type"=>"integerList","min"=>0,"max"=>59,'increment'=>$desc->getProperty("increment-s",1,false)),$desc),true);
	}

	function clear()
	{
		$this->day->clear();
		$this->month->clear();
		$this->year->clear();
		$this->h->clear();
		$this->mn->clear();
		$this->s->clear();
	}
	
	function attachRecord(&$record){
		$this->record=&$record;
		
		$this->day->attachRecord($record);
		$this->month->attachRecord($record);
		$this->year->attachRecord($record);

		$this->h->attachRecord($record);
		$this->mn->attachRecord($record);
		$this->s->attachRecord($record);
	}

	function setValue($deft)
	{
		if ($deft == null)
			return false;

		// read default date value (any kind of format is supported including now, tomorrow, etc.)
		if(is_numeric($deft))
            $timestamp = $deft;
        elseif(is_array($deft) && isset($deft['d']))
        {
            $this->day->value = $deft['d'];
            $this->month->value = $deft['m'];
            $this->year->value = $deft['y']; 
            $this->h->value=$deft['h'];
            $this->mn->value=$deft['mn'];
            $this->s->value=$deft['s'];            
            return true;
        }
        elseif (($timestamp = $this->strptime($deft)) === -1)
				return false;

		$deftdate = getdate($timestamp); 
		$this->day->value = $deftdate['mday'];
		$this->month->value = (string)(intval($deftdate['mon'])+0); 
		$this->year->value = $deftdate['year']; 
		$this->h->value=$deftdate['hours'];
		$this->mn->value=$deftdate['minutes'];
		$this->s->value=$deftdate['seconds'];
		
		return true;
	}
	
	function toTime() {
		return $this->toHTML('unix_s');
	}

	function toObject($opts=null)
	{ 
		return array(
			'd'=>$this->day->value,
			'm'=>$this->month->value,
			'y'=>$this->year->value,
			'h'=>$this->h->value,
			'mn'=>$this->mn->value,
			's'=>$this->s->value,
			); 
	}
	function readFromStore(&$a){
		if (is_array($a))
		{
			$this->day->value = $a['d'];
			$this->month->value = $a['m'];
			$this->year->value = $a['y'];
			$this->h->value = $a['h'];
			$this->mn->value = $a['mn'];
			$this->s->value = $a['s'];
		}
		else
			$this->setValue($a);
	}
	
	function readFromDB($recData)
	{
		$v = $this->getFieldData($recData,$this->getFName(),"NULL");
		if ($v == 'NULL')
			$v = '?-?-? ?:?:?';
		
		if (preg_match('/([?]|[0-9]{4})-?([?]|[0-9]{2})-?([?]|[0-9]{2})[ ]?([0-2][0-9]):?([0-6][0-9]):?([0-6][0-9])/',
				$v,$matches))
		{
			list($dummy,$year,$m,$d,$h,$mn,$s)=$matches;
			// remove "0" heading (date choice-lists are based on numbers)
			$this->month->value = ltrim($m,"0");
			$this->day->value = ltrim($d,"0");	
			$this->year->value =ltrim($year,"0");
			$this->h->value =($v=ltrim($h,"0"))===''?0:$v;
			$this->mn->value =($v=ltrim($mn,"0"))===''?0:$v;
			$this->s->value =($v=ltrim($s,"0"))===''?0:$v;
		}
		elseif (preg_match('/([?]|[0-9]{4})-?([?]|[0-9]{2})-?([?]|[0-9]{2})(?:[ ]?([0-2][0-9]):?([0-6][0-9]):?([0-6][0-9]))?/',
				$v,$matches))
		{
			// yyyy-mm-dd hh:m:s
			list($dummy,$year,$m,$d)=$matches;
			$this->month->value = ltrim($m,"0");
			$this->day->value = ltrim($d,"0");	
			$this->year->value =ltrim($year,"0");
			$this->h->value =0;
			$this->mn->value =0;
			$this->s->value =0;
		}
		elseif (preg_match('/([?]|[0-9]{2})-?([?]|[0-9]{2})-?([?]|[0-9]{4})(?:[ ]?([0-2][0-9]):?([0-6][0-9]):?([0-6][0-9]))?/',
				$v,$matches))
		{
			// dd-mm-yyyy hh:m:s
			list($dummy,$d,$m,$year)=$matches;
			$this->month->value = ltrim($m,"0");
			$this->day->value = ltrim($d,"0");	
			$this->year->value =ltrim($year,"0");
			$this->h->value =0;
			$this->mn->value =0;
			$this->s->value =0;
		}
		else
		{
			// DebugBreak();
			nxError($this->getFName().' field: bad date time format:'.$v,'NX_DATA');
		}
	}
	
	function toDB($opts=null) 
	{
		if ($this->isOk())
		{
			return "'{$this->year->value}-".str_pad($this->month->value,2,'0',STR_PAD_LEFT).'-'.str_pad($this->day->value,2,'0',STR_PAD_LEFT).' '.str_pad($this->h->value,2,'0',STR_PAD_LEFT).':'.str_pad($this->mn->value,2,'0',STR_PAD_LEFT).':'.str_pad($this->s->value,2,'0',STR_PAD_LEFT)."'";
		}

        // date not set and type is time        
        if ($this->desc->getProperty("control","timestamp",false)=='time')
            return "'1970-01-01 {$this->h->value}:{$this->mn->value}:{$this->s->value}'";

		return 'NULL';
	}

	function isOk() {

		$y = (int)$this->year->value;
		if($y > 1971)
			return checkdate((int)$this->month->value,(int)$this->day->value, $y);
		else
			return false;
	}
	function isEmpty()
	{
		return !$this->isOk();
	}

	function toUrlParam() {
		return $this->getAlias() . "=" . rawurlencode($this->toHTML('%x'));
	}

	function toHTML($opts=null)
	{
		// NB. locale is set up at the page level
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year->value))
		{
			$t=
				mktime(
					$this->h->value?(int)$this->h->value:0,
					$this->mn->value?(int)$this->mn->value:0,
					$this->s->value?(int)$this->s->value:0,
					(int)$this->month->value,
					(int)$this->day->value,
					(int)$this->year->value);	
			
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat = $this->desc->getProperty("format","short");
				
			$df=$this->desc->getString("format_datetime_$dateFormat",null,'form_fields',$dateFormat);
			
			switch($df)
			{								
				case 'unix_s':
				case "timestamp":
					return $t;
				case 'unix_ms':
					return ''.$t.'000';
				/*case "long":
					$fmt = $this->desc->getProperty("dateTimeFormatLong","%A %d %B %Y, %Hh%Mm%S");
					return $this->strftime($fmt,$t);
				case "short":
					$fmt = $this->desc->getProperty("dateTimeFormatShort","%x %X");
					return $this->strftime($fmt,$t);*/
				default: // '%d %B %Y'
					return $this->strftime($df,$t);
			}										
		}
		else
		{
			return $this->desc->getString("format_datetime_vide",null,'form_fields',"n/a");
		}
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return '';

		if (defined('NX_DATETIME_CONTROL'))
			$type=NX_DATETIME_CONTROL;
		else
			$type="datetime";
		$controlName = $desc->getProperty("control",$type,false);

		if ($controlName=='time')
		{
			if (defined('NX_TIME_CONTROL'))
				$type=NX_TIME_CONTROL;
			else
				$type="datetime";

			//$control = $desc->getControl('datetime');
			$control = $desc->getControl($type);
		}
		elseif ($controlName=='timestamp')
			$control = $desc->getControl($type);
		else
			$control = $desc->getControl($type);

		return $control->toForm($this,$controlName,$opts);
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$desc = & $this->desc;
		if (defined('NX_DATETIME_CONTROL'))
			$type=NX_DATETIME_CONTROL;
		else
			$type="datetime";
		$controlName = $desc->getProperty("control",$type,false);

		if ($controlName=='time')
		{
			if (defined('NX_TIME_CONTROL'))
				$type=NX_TIME_CONTROL;
			else
				$type="datetime";

			//$control = $desc->getControl('datetime');
			$control = $desc->getControl($type);
		}
		elseif ($controlName=='timestamp')
			$control = $desc->getControl($type);
		else
			$control = $desc->getControl($type);

		return $control->readForm($recData,$this,$type);
	}
	
	
	/** get timestamp from date string, fix pb with european format */
	function strptime($time)
	{
		if(!$time)
			return 0;

		// fix for european times
		if (false && function_exists('strptime '))
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

function NX_strftime($dateFormat,$time, $encode=true) {
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

?>

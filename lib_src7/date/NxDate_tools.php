<?php

class NxDate_tools{

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

		if (defined('NX_ENCODE_DATE'))
			$encode=NX_ENCODE_DATE;

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
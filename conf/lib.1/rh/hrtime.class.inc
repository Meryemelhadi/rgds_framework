<?php

class HR_Time
{
	static function nbFullMonths($datefrom,$dateto) 
	{
		$difference = $dateto - $datefrom;
        $months_difference = floor($difference / 2678400);
        while (mktime(date("H", $datefrom), date("i", $datefrom), date("s", $datefrom), date("n", $datefrom)+($months_difference), date("j", $dateto), date("Y", $datefrom)) < $dateto) {
            $months_difference++;
        }
        $months_difference--;
        return $months_difference;
    }
	static function nbFullYears($datefrom,$dateto) 
	{
		$difference = $dateto - $datefrom;
		
		$years_difference = floor($difference / 31536000);
		if (mktime(date("H", $datefrom), date("i", $datefrom), date("s", $datefrom), date("n", $datefrom), date("j", $datefrom), date("Y", $datefrom)+$years_difference) > $dateto) {
            $years_difference--;
        }
        if (mktime(date("H", $dateto), date("i", $dateto), date("s", $dateto), date("n", $dateto), date("j", $dateto), date("Y", $dateto)-($years_difference+1)) > $datefrom) {
            $years_difference++;
        }
        return $years_difference;
    }

	static function nbDays($datefrom,$dateto) 
	{
		$difference = $dateto - $datefrom;
        return floor($difference / 86400);
    }

	static function formatTime($time='now',$dateFormat='%x', $encode=true) {
		$time = strtotime($time);

        global $NX_CHARSET;
        $s = strftime($dateFormat,$time);
        if (NX_ENCODE_DATE && $encode && $NX_CHARSET!='ISO-8859-1') {
            if (function_exists(iconv))
                $s=iconv("ISO-8859-1",$NX_CHARSET,$s);
            else
                $s=utf8_encode($s);
        }
        return $s;
    }    

	static function getDayOfTheWeek($dateTime)
	{
		$day_of_week = strftime("%u",$dateTime);
		if ($day_of_week!==FALSE)
			return $day_of_week;

		$day_of_week = strftime("%w",$dateTime);
		if($day_of_week==0)
			return 7;
		return $day_of_week;
	}

	static function getWeek($dateTime)
	{
		return strftime("%W",$dateTime);
	}
}

class RH_LegalDay extends PersistentObject 
{
}

class RH_LegalDayPeer
{
	// auto factory
	static function getByName($name) {
	}

	// auto factory
	static function getByOid($oid) {
		$o = new RH_LegalDay();
		$o->load('rh.conge.legal_days@rh','db', array('legal_days'=>$oid));
		return $o;
	}
}

?>
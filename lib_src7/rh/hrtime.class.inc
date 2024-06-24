<?php

include_once(NB_LIB.'orm/persistence.inc');

$GLOBALS['HRTime_config']=array(
	'nbDaysperWeek'=>defined(RH_NB_DAYS_PER_WEEK)?RH_NB_DAYS_PER_WEEK:6,
	'canLeaveSameDay'=>defined(RH_CAN_LEAVE_SAME_DAY)?RH_CAN_LEAVE_SAME_DAY:false,
	'ds_holidays'=>'rh.jours_feries'
);

class HR_Time
{
	var $ctxt;
	function HR_Time(&$context,$params=null)
	{		
		$this->ctxt=&$context;
		$this->params=$params?$params:$GLOBALS['HRTime_config'];
	}

	function getDayOfTheWeek($dateTime)
	{
		$day_of_week = strftime("%u",$dateTime);
		if ($day_of_week!==FALSE)
			return $day_of_week;

		$day_of_week = strftime("%w",$dateTime);
		if($day_of_week==0)
			return 7;
		return $day_of_week;
	}

	function getWeek($dateTime)
	{
		return strftime("%W",$dateTime);
	}

	function getHolidaysArray($startTime,$endTime)
	{	
		$hols = new PersistentCollection($this->ctxt,$this->params['ds_holidays'],'db',
			array('start_date'=>$startTime,'end_date'=>$endTime)
		);
		$holidayObjs = $hols->getRecordArray();
		
		$hdArray = array();
		for ( $i = 0; $i < count($holidayObjs); $i++ )
		{
			$currentRecord = $holidayObjs[$i];

			$startDate		= $currentRecord->getField('start_date');
			$endDate		= $currentRecord->getField('end_date');
			$startDatetime	= $startDate->toTime();
			$endDateTime	= $endDate->toTime();

			$currentDay = $startDatetime;
			while( $currentDay <= $endDateTime )
			{
				$dayOfW=$this->getDayOfTheWeek($currentDay);
				if( $dayOfW != 6 && $dayOfW != 7 )
				{
					$hdArray[] = date("Y-m-d",$currentDay);
				}
				$currentDay+=60*60*24;
			}
		}

		return array_unique($hdArray);
	}

	function isAWorkingDay($dateTime, $holidays)
	{
		$day_of_week = $this->getDayOfTheWeek($dateTime);
		if($day_of_week == 7)
			return false;

		foreach($holidays as $holiday)
		{
			$time_stamp=strtotime($holiday);
			if ( $dateTime >= $time_stamp && 
				$dateTime  < $time_stamp + 24*60*60)
			{
				return false;
			}
		}

		return true;
	}

	function countWorkingDays($startDateField, $endDateField,$halfStartDate,$halfEndDate,&$nbHols)
	{
		$startDateTime	=$startDateField->toTime();
		$endDateTime	=$endDateField->toTime();
		$days = ($endDateTime - $startDateTime) / (60*60*24);

		$no_full_weeks = floor($days / 7);
		$no_remaining_days = fmod ($days, 7);

		$the_first_day_of_week = $this->getDayOfTheWeek($startDateTime);
		$the_last_day_of_week = $this->getDayOfTheWeek($endDateTime);

		$the_first_week = $this->getWeek($startDateTime);
		$the_last_week = $this->getWeek($endDateTime);

		// case 1: same week
		if ($the_first_week==$the_last_week)
		{
			if ($the_last_day_of_week == 7 ) 
			{
				$no_remaining_days--;
			}
		}
		elseif ($the_first_day_of_week <= $the_last_day_of_week)
		{
			if ($the_first_day_of_week <= 6 && 6 <= $the_last_day_of_week) 
			{
				$no_remaining_days--;
			}
			if ($the_first_day_of_week <= 7 && 7 <= $the_last_day_of_week) 
			{
				$no_remaining_days--;
			}
		}
		else
		{
			if ($the_first_day_of_week <= 6)
			{
				$no_remaining_days--;
			}
		}

		
		// count working days in between start/end weeks
		$nbDaysperWeek=$this->params['nbDaysperWeek'];
		$workingDays = $no_full_weeks * $nbDaysperWeek;
		if ($no_remaining_days > 0 )
		{
			$workingDays += $no_remaining_days;
		}
		
		// remove holidays
		$start=trim($startDateField->toDB(),"'");
		$end=trim($endDateField->toDB(),"'");
		$holidays=$this->getHolidaysArray($start,$end);

		$nbHols=0;
		foreach($holidays as $holiday)
		{
			$time_stamp=strtotime($holiday);
			$dayOfW=$this->getDayOfTheWeek($time_stamp);

			if ( $startDateTime <= $time_stamp && 
				$time_stamp <= $endDateTime && 
				!($nbDaysperWeek == 6 && $dayOfW == 6) &&
				$dayOfW != 7 )
			{
				$nbHols++;
				$workingDays--;
			}
		}

		// update with half days
		if( $workingDays > 0 )
		{
			if ($this->isAWorkingDay($startDateTime, $holidays))
				$workingDays -= (($halfStartDate==1)?0.5:0);
				
			if ($this->isAWorkingDay($endDateTime, $holidays))
				$workingDays +=(($halfEndDate==1)?0.5:0);
		}

		return $workingDays;
	}
	function canLeaveSameDay() {
		return $this->params['canLeaveSameDay'];
	}

	function getLegalDays($legaldayType,&$legalDaysName)
	{
		$legalDaysObj=new PersistentObject($this->ctxt,'rh.conge.demand.selected_legal_day_ajax','db',
			array('legal_days'=>$legaldayType));

		if ($legalDaysObj->isOk())
		{
			$legalDaysName=$legalDaysObj->getFieldValue('legal_day_name','string','');
			return (int)$legalDaysObj->getFieldValue('allowed_days','string','');
		}
		return 0;

	}

}

?>
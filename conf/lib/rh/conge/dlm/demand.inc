<?php

include_once(NX_XLIB.'lib/rh/conge/workflow.inc');

class rh_conge_demand_dlm extends rh_conge_demand
{
	function __construct($ruleset='DLM') {
		parent::__construct($ruleset);

		$this->params=array(
			'nbDaysperWeek'=>6,
			'canLeaveSameDay'=>false,
			'ds_holidays'=>'rh.jours_feries@rh'
		);
	}

	// le credit de l'utilisateur acquis pour la periode
	function getNominalYearCredit($user,$toDateTime) 
	{
		// get type of employee
		$role = $user->csp->string;
		if (preg_match('/direct/i',$role))
			$credit = 33;
		else
			$credit = 26;

		return $credit;
	}

	// le credit de l'utilisateur acquis pour la periode
	function getOwnedCreditToDateForUser($user,$toDateTime,&$nbMonths) 
	{
		$fromDateTime = $this->getStartOfYearForUser($user);
		$nbMonths = HR_Time::nbFullMonths($fromDateTime, $toDateTime);
		$yearCredit = $this->getNominalYearCredit($user);
		return $yearCredit*$nbMonths/12;
	}

	// le credit de l'utilisateur pour l'année (pro rata si embauché dans l'année)
	function getYearCreditForUser($user,&$nbMonths) 
	{
		$yearCredit = $this->getNominalYearCredit($user);

		$fromDateTime = $this->getStartOfYearForUser($user);
		$toDateTime   = $this->getEndOfYearForUser($user);
		
		$nbMonths = HR_Time::nbFullMonths($fromDateTime, $toDateTime);
		if ($nbMonths==12)
			return $yearCredit;
		else
			return $yearCredit*$nbMonths/12;
	}

	
/*

Les agents ne peuvent obtenir leur premier congé annuel qu’après six mois
de présence et après titularisation. La durée de ce congé est
proportionnelle au temps de présence au cours de l’année de référence sans
que la durée de congé à prendre puisse être inférieure à dix jours dont
neuf jours ouvrables

*/
	function isOkFirstYear($user,$demandedDays)
	{
		return true;
	}

	function checkRules($user,$force=false) 
	{
		$demandeObj=$this;

		$currentDemandID = $this->oid;
		$startDateTime   = $this->start_date->time;
		$endDateTime	 = $this->end_date->time;
		$halfStartDate   = $this->half_start_date->object;
		$halfEndDate     = $this->half_end_date->object;

		if ($startDateTime==0 or $endDateTime==0)
			return "Merci d'indiquer les dates de votre cong&eacute; ci-dessus";
		
		// StartDate doit ?tre strictement sup?rieure a la date d'aujourd'hui
		$now = strtotime(date("m/d/y",time()));
		if (!$force && !$this->canLeaveSameDay())
		{
			if( $startDateTime <= $now )
				return "La date de d&eacute;but doit &ecirc;tre sup&eacute;rieure &agrave; la date courante";
		}
		else
		{
			if( !$force && ($startDateTime+24*60*60 <= $now))
				return "La date de d&eacute;but doit &ecirc;tre sup&eacute;rieure &agrave; la date courante";
		}

		if( ($startDateTime == $endDateTime) && ($halfStartDate==$halfEndDate || $halfStartDate==1))
			return "La date de d&eacute;but doit &ecirc;tre strictement inf&eacute;rieure &agrave; la date de reprise";

		$first_day_weekday = HR_Time::getDayOfTheWeek($startDateTime);
		$last_day_weekday = HR_Time::getDayOfTheWeek($endDateTime);
		if( $first_day_weekday == 7 || $last_day_weekday==7)
			return "La date de début ou de fin de congé ne peut pas être un Dimanche";

		// Le reliquat doit ?tre sup?rieur au nombre de jours demand?s
		$userReliquat = $user->reliquat->object;

		// get reliquat data
		$infos=$this->getInfos($user);
		$demandedDays=$infos['demandedDays'];

		// calculer le nombre de jours si pas d?ja fait..
		if ($demandedDays === null)
		{
			$demandedDays = $this->countWorkingDays(
				$startDateTime, $endDateTime,
				$halfStartDate,$halfEndDate,$nbHols);
		}
		
		// v?rifier si on d?passe le quotat de jours de cong?s
		if( $demandedDays > $userReliquat )
			return "Votre cr&eacute;dit ne vous permet pas de prendre ce cong&eacute;";

		// StartDate doit ?tre inf??rieure ou ?gale au endDate
		if( $startDateTime > $endDateTime )
			return "La date de d&eacute;but du cong&eacute; doit &ecirc;tre inf&eacute;rieure ou &eacute;gale &agrave; la date de reprise";
		
		return $this->checkOverlapRH();
	}

	function getInfos($user)
	{
		if ($this->infos !== null)
				return $this->infos;

		$demandeObj=$this;
		$tab = array();
		
		$startDateField = $demandeObj->start_date;
		$startDateTime  = $startDateField->time;

		$endDateField  	= $demandeObj->end_date;
		$endDateTime    = $endDateField->time;

		$userReliquat 	= $user->reliquat->object;
		$lastCredit 	= $user->last_credit->object;
		$currentCredit	= $user->current_credit->object;
		$currentYearField 	= $user->current_year;
			
		$halfStartDate = $demandeObj->half_start_date->object;
		$halfEndDate   = $demandeObj->half_end_date->object;

		$demandedDays = $this->countWorkingDays(
			$startDateField, $endDateField,
			$halfStartDate,$halfEndDate,$nbHols);

		// get legal days to remove from count
		$legalDayType   = $demandeObj->legal_days;
		$legalDays = $this->getLegalDays($legalDayType->object,$legalDaysName);

		// remove legal days
		$demandedDays -= $legalDays;

		// test du nombre de jours demandes			
		if ( $demandedDays <= 0)
		{
			$demandedDays = 0 ;
			
			$this->infos = array(
					'status'		=> 'ok',
					'nbHolidays'    => $nbHols,
					'currentCredit' => $currentCredit,
					'lastCredit'    => $lastCredit,
					'demandedDays'  => $demandedDays,
					'reliquat'      => $userReliquat,
					'message'		=> 
						(($legalDays <= 0) ? '' : 'cong&eacute; l&eacute;gal '.$legalDaysName.' = '.$legalDays.' jour'.(($legalDays>1)? 's' : '' ).'').
						(($nbHols <= 0) ? '' : ' jours f&eacute;ri&eacute;s = '.$nbHols.' jour'.(($nbHols>1)? 's' : '' ).'')
			);
			
		}
			
		// fin du test de nombre de jours demandes
		if ( $lastCredit <= 0 )
		{
			$currentCredit = $currentCredit - $demandedDays;
		}
		else
		{
			if ( $lastCredit != 0 )
			{
				if ( $lastCredit >= $demandedDays )
				{
					$lastCredit = $lastCredit - $demandedDays;
				}
				else
				{ 
					$nbDays = 0;
					
					$nbDays = $demandedDays - $lastCredit;
					
					$lastCredit = 0;
					
					$currentCredit = $currentCredit - $nbDays;
				}
			}	  
		}

		$userReliquat = $lastCredit + $currentCredit;

		$this->infos = array(
				"status"		=> 'ok',
				"nbHolidays"    => $nbHols,
				"currentCredit" => $currentCredit,
				"lastCredit"    => $lastCredit,
				"demandedDays"  => $demandedDays,
				"reliquat"      => $userReliquat,
				"legalDays"		=> $legalDays,
				"message"		=> 
					(($legalDays <= 0) ? '' : 'Cong&eacute; l&eacute;gal '.$legalDaysName.' = '.$legalDays.' jour'.(($legalDays>1)? 's' : '' ).'').
					(($nbHols <= 0) ? '' : ' Jour'.(($nbHols>1)? 's' : '' ).' f&eacute;ri&eacute;'.(($nbHols>1)? 's' : '' ).' = '.$nbHols)

			);

		return $this->infos;
	  }

	function countWorkingDays($startDateField, $endDateField,$halfStartDate,$halfEndDate,$nbHols)
	{
		$startDateTime	=$startDateField->time;
		$endDateTime	=$endDateField->time;
		$days = ($endDateTime - $startDateTime) / (60*60*24);

		$no_full_weeks = floor($days / 7);
		$no_remaining_days = fmod ($days, 7);

		$first_day_weekday = HR_Time::getDayOfTheWeek($startDateTime);
		$last_day_weekday = HR_Time::getDayOfTheWeek($endDateTime);

		$first_week = HR_Time::getWeek($startDateTime);
		$last_week = HR_Time::getWeek($endDateTime);

		// case 1: same week
		if ($first_week==$last_week)
		{
			// remove weekend days if same week
			if ($last_day_weekday > $this->params['nbDaysperWeek'] ) 
			{
				$no_remaining_days-= $last_day_weekday - $this->params['nbDaysperWeek']-1;
			}
		}
		// case 2 : at least on 2 weeks
		elseif ($first_day_weekday <= $last_day_weekday)
		{
			if ($first_day_weekday <= 6 && $last_day_weekday >=6) 
			{
//				$no_remaining_days--;
			}
			if ($first_day_weekday <= 7 && $last_day_weekday==7) 
			{
				$no_remaining_days--;
			}
		}

		else
		{
			if ($first_day_weekday <= $this->params['nbDaysperWeek'])
			{
				$no_remaining_days-= 7 - $this->params['nbDaysperWeek'];
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
		$start=trim($startDateField->db,"'");
		$end=trim($endDateField->db,"'");
		$holidays=$this->getHolidaysArray($start,$end);

		$nbHols=0;
		foreach($holidays as $holiday)
		{
			$time_stamp=strtotime($holiday);
			if ( $startDateTime <= $time_stamp && $time_stamp < $endDateTime && 
				!$this->isWeekend($time_stamp) )
			{
				$nbHols++;
				$workingDays--;
			}

		}

		// update with half days
		if( $workingDays > 0 || ($first_week==$last_week && $first_day_weekday == $last_day_weekday))
		{
			if ($this->isAWorkingDay($startDateTime, $holidays))
				$workingDays -= (($halfStartDate==1)?0.5:0);
				
			if ($this->isAWorkingDay($endDateTime, $holidays))
				$workingDays +=(($halfEndDate==1)?0.5:0);
		}

		return $workingDays;
	}

}

class rh_conge_demandPeer
{
	static function mapToRulesFor($demand,$user=null) 
	{
		$class = self::getClassForDemand($demand,$user);

		$o = new $class();
		$o->setRecord($demand->record);
		return $o;
	}
	
	static function getClassForDemand($demand,$user=null) {
		if ($user === null)
		{
			if ($demand->empid->object == '')
				$demand->empid->object = Framework::getUser()->getProperty('oid',0);

			$user = rh_userPeer::getByOid($demand->empid->object);
		}

		return self::getClassBySite($user->localisation);
	}

	static function getClassForUserOid($userOid=null) {
		if ($userOid === null)
		{
			$userOid = Framework::getUser()->getProperty('oid',0);
		}

		$user = rh_userPeer::getByOid($userOid);

		return self::getClassBySite($user->localisation);
	}
	
	static function getClassBySite($site) {
		return 'rh_conge_demand_dlm';
	}

	static function applyDemandClasses($col,$userOid=null) {
		if ($userOid)
		{
			// all demands for the same user => same class
			$class=self::getClassForUserOid($userOid);
			$col->apply($class);
			return $col;
		}

		// demands from different users
		foreach($col->array as $demandRec)
		{
			$class = self::getClassForDemand($demandRec);
			$o = new $class();
			$o->setRecord($demandRec);
		}

		return $col;
	}

	// auto factory
	static function getByOid($oid,$class='rh_conge_demand') {
		$o = new $class();
		$o->load($o->ds_by_oid,'db', array('oid'=>$oid));

		return self::mapToRulesFor($o);
	}
	static function getFromRequest($class='rh_conge_demand') {
		$o = new $class();
		$o->load($o->ds_by_oid,'request');
		return self::mapToRulesFor($o);
	}
	static function getFromRecord($record,$class='rh_conge_demand') {
		$o = new $class();
		$o->setRecord($record);
		return self::mapToRulesFor($o);
	}
	static function getEmpty($params=null,$class='rh_conge_demand',$user=null) {
		$o = new $class();
		$o->load($o->ds_demand_new,'empty');
		
		// init values
		if (!$params)
			$params=array(
				'start_date'=>HR_Time::formatTime('now+1days','%x'),
				'end_date'=>HR_Time::formatTime('now+2days','%x'),
				'demand_state'=>1
			);

		foreach($params as $fn=>$v)
			$o->$fn = $v;

		return self::mapToRulesFor($o,$user);
	}
	static function deleteByOid($oid,$class='rh_conge_demand') {
		$o = new $class();
		$o->load($o->ds_by_oid,'db', array('oid'=>$oid));
		if ($o->isOk())
			$o->delete($o->ds_by_oid,'db');

		return $o;
	}

	static function getByDate($start_date,$end_date,$userOid=null,$demand_state=null,
		$class=null) 
	{
		if (!$class)
			$class='rh_conge_demand';

		$o = new $class();
		$col = new PersistentCollection();
		$col->load($o->ds_by_date,'db', array(
			'start_date'=>$start_date,
			'end_date'=>$end_date,
			'empid'=>$userOid?$userOid:'',
			'demand_state'=>$demand_state
			));

		// attach records to demand class
		$col = self::applyDemandClasses($col,$userOid);

		return $col;
	}

	static function getByDs($ds,$userOid=null,$params=null,$data=null) 
	{
		$col = new PersistentCollection();
		$col->load($ds,'db', $params ,$data);
		
		// attach records to demand class
		$col = self::applyDemandClasses($col,$userOid);

		return $col;

		// return $col->$class;
	}

}

?>
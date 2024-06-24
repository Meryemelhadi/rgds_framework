<?php

include_once(NX_XLIB.'lib/rh/conge/workflow.inc');

/*

III.SIEGE

Les règles restent les mêmes que celles convenues en début du projet.

RG1 - Le 1er samedi du congé
Les jours fériés et les week-ends (Dimanches) sont retirés des jours à prendre, les samedis sont comptés, et la règle pour le premier samedi du congé est : 
	- Si le premier jour de congé est un Jeudi ou Vendredi le Premier 	Samedi n'est pas compté
	- Si le premier jour de congé est AUTRE que le jeudi / vendredi le 	premier samedi est compté.



*/

class rh_conge_demand_wafa extends rh_conge_demand
{
	function __construct() {
		parent::__construct('Siège');

		$this->params=array(
			'nbDaysperWeek'=>6,
			'canLeaveSameDay'=>false,
			'ds_holidays'=>'rh.jours_feries@rh'
		);
	}

	/*
		le credit de l'utilisateur acquis pour la periode

		§        Cadres : 26 j ouvrables
		 
		§        Non cadres : Les jours en plus sont rajoutés en début d’année et non à la date anniversaire de l’ancienneté 
		 
		-         moins de 5 ans             : 22 j
		-         [ 5 – 10 ans [                : 23,5 j
		-         [ 10 – 15 ans [              : 25 j
		-         15 ans et plus               : 26 j

	*/
	function getNominalYearCredit($user,$toDateTime) 
	{

		// get type of employee
		$role = $user->csp->string;
		if (preg_match('/cadre/i',$role))
			$credit = 26;
		else
		{
			$nbYears = $this->getNbYearsOfService($user,$toDateTime);
			if ($nbYears < 5)
				$credit = 22;
			elseif($nbYears < 10)
				$credit = 23.5;
			elseif($nbYears < 15)
				$credit = 25;

			$credit += floor($nbYears/5)*1.5;
		}


		return $credit;
	}

	/*
		vérifies les règles de coherences pour le congés

		RG5 - Congé sans solde
		Possibilité de prendre un congé, et le compter sur l'année suivante, si le crédit au moment de la demande est nul.
		Exemple : 
			Un collaborateur qui a épuisé tout son crédit 2008, fait une demande de congé au mois de décembre 2008, il aura la possibilité de demander à 	comptabiliser le congé de son solde de l'année 2009.
	
	*/

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

		if( $startDateTime == $endDateTime )
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
		
		// Liste des demandes de cong?s coincidant avec la demande
		$ctxt = Framework::getContext();
		$listeConges = $ctxt->getRecords('rh.conge.demand.valid_demands',null,'db',false,$demandeObj);
		if( $listeConges && $listeConges->count() > 0)
			return "Il existe d&eacute;j&agrave; des demandes de cong&eacute; coincidant avec le cong&eacute; que vous demandez";

		// Liste des ordres de mission coincidant avec la demande
		$listeMissions = $ctxt->getRecords('rh.conge.demand.demande_mission',null,'db',false,$demandeObj);
		if( $listeMissions &&  $listeMissions->count() > 0 )
			return "La demande de cong&eacute; coincide avec un ou plusieurs ordres de mission existants";	

		return true;
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
		if ( $lastCredit == 0 )
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
				$no_remaining_days-= $last_day_weekday - $this->params['nbDaysperWeek'];
			}
		}
		// case 2 : at least on 2 weeks
		elseif ($first_day_weekday <= $last_day_weekday)
		{
			// check if ok... seems not to be ok for "6 days weeks"
			if ($first_day_weekday <= 6 && 6 <= $last_day_weekday) 
			{
				$no_remaining_days--;
			}
			if ($first_day_weekday <= 7 && 7 <= $last_day_weekday) 
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

		// si 1er jour = jeudi ou vendredi, ne pas compter le samedi
		if (($first_day_weekday == 4 || $first_day_weekday == 5) && $days > 2 )
		{
			$no_remaining_days-= 1;
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
		if( $workingDays > 0 )
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
		return 'rh_conge_demand_wafa';
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
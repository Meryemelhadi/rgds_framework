<?php

include_once(NX_XLIB.'lib/rh/conge/workflow.inc');

// régles de congés spécifiques

/*

II. NADOR

Congés réguliers :
Techniciens, agent de maitrise et cadres administratif (Tamca) : 30jours
calendaire. Cette durée est majorée d’un jour par année de services
effectifs au –delà des dix premières années sans que cette majoration
puisse avoir pour effet de porter la durée totale du congé à plus de trente
sept jours calendaires


Ouvriers Employés(OE) : 21 jours calendaires cette durée est majorée d’un
jour  pour deux années de services effectifs jusqu’à un maximum de douze
jours, sans dépasser 33 jours.

Le congé régulier ne peut en aucune manière être fractionné en plus de deux
tranches :
La première tranche doit être d’une durée minimum de dix (10) jours.
La 2ème tranche d’une durée minimum de sept (7) jours

Les agents ne peuvent obtenir leur premier congé annuel qu’après six mois
de présence et après titularisation. La durée de ce congé est
proportionnelle au temps de présence au cours de l’année de référence sans
que la durée de congé à prendre puisse être inférieure à dix jours dont
neuf jours ouvrables

Congés exceptionnels :
Il est accordé aux agents TAMCA et OE un congé exceptionnel avec solde à
défalquer des droits à congé régulier jusqu’à concurrence de dix(10) jours.
Les dix jours peuvent être fractionnés au minimum par ½ journée à 3jours au
maximum jusqu’à épuisement de dix (10) jours.

*/

class rh_conge_demand_sonasid_nador extends rh_conge_demand
{
	const NB_JOURS_MIN_TRANCHE1 = 10;
	const NB_JOURS_MIN_TRANCHE2 = 7;

	function __construct($ruleset='nador') {
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
		$nbYears = $this->getNbYearsOfService($user,$toDateTime);

		// get type of employee
		$role = $user->csp->string;
		if (preg_match('/ouvrier/i',$role))
			$credit = $this->getNominalYearCredit_OE($user,$nbYears);
		else
			$credit = $this->getNominalYearCredit_TAMCA($user,$nbYears);

		return $credit;
	}

/*
Techniciens, agent de maitrise et cadres administratif (Tamca) : 30jours
calendaire. Cette durée est majorée d’un jour par année de services
effectifs au –delà des dix premières années sans que cette majoration
puisse avoir pour effet de porter la durée totale du congé à plus de trente
sept jours calendaires
*/
	function getNominalYearCredit_TAMCA($user,$nbYears) 
	{
		$credit = 30;

		if ($nbYears > 10)
		{
			$add = (($nbYears-10)/2);		
			$credit += $add;
		}

		return ($credit > 37) ? 37 :$credit;
	}
	
/*
Ouvriers Employés(OE) : 21 jours calendaires cette durée est majorée d’un
jour  pour deux années de services effectifs jusqu’à un maximum de douze
jours, sans dépasser 33 jours.
*/
	function getNominalYearCredit_OE($user,$nbYears) 
	{
		$credit = 21;

		$add = floor($nbYears/2);
		if ($add>12)
			$add = 12;
		
		$credit += $add;
		return ($credit > 33) ? 33 :$credit;
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
		$nbMonths=0;
		$actualCredit = $this->getOwnedCreditToDateForUser($user,$this->start_date->time,$nbMonths);

		// not the first year => ok
		if ($nbMonths > 12)
			return true;

		// first year for that user

		// more than 6 months?
		if ($nbMonths <= 6)
			return 'Vous ne pouvez prendre de congés qu\'après 6 mois pleins (première année de service)';

		$nbOwnedMonths=0;
		$ownedCredit = $this->getOwnedCreditToDateForUser($user,$this->start_date->time,$nbOwnedMonths);

		// is it enough?
		if ($ownedCredit < $demandedDays)
		{
			return 'Vous ne pouvez pas demander plus de '.$ownedCredit.' jours (première année de service)';
		}

		$startDate   = $this->start_date;
		$endDate	 = $this->end_date;
		$halfStartDate   = $this->half_start_date->object;
		$halfEndDate     = $this->half_end_date->object;

		$nbOfficedays = $this->countWorkingDays(
				$startDate, $endDate,
				$halfStartDate,$halfEndDate,$nbHols);

		// check credit
		if ($demandedDays < 10 || $nbOfficedays<9)
		{
			return 'Vous devez prendre au moins 10 jours de congés dont 9 jours ouvrables (première année de service)';
		}

		return true;
	}

	// vérifies les regles de coherences pour le cong?s
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

		// StartDate doit ?tre inf??rieure ou ?gale au endDate
		if( $startDateTime > $endDateTime )
			return "La date de d&eacute;but du cong&eacute; doit &ecirc;tre inf&eacute;rieure ou &eacute;gale &agrave; la date de reprise";
		
		// StartDate doit ?tre strictement sup?rieure a la date d'aujourd'hui
		$now = strtotime(date("m/d/y",time()));
		if (!$force && !$this->canLeaveSameDay())
		{
			if( $startDateTime <= $now )
				return "La date de d&eacute;but doit &ecirc;tre sup&eacute;rieure &agrave; la date courante";
		}
		else
		{
			if(!$force && ($startDateTime+24*60*60 <= $now))
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

		// check rules for first year
		if(($res = $this->isOkFirstYear($user,$demandedDays))!==true)
			return $res;

		// verifier si on depasse le quotat de jours de conges
		if( $demandedDays > $userReliquat )
			return "Votre cr&eacute;dit ne vous permet pas de prendre ce cong&eacute;";

/*

Le congé régulier ne peut en aucune manière être fractionné en plus de deux
tranches :
La première tranche doit être d’une durée minimum de dix (10) jours.
La 2ème tranche d’une durée minimum de sept (7) jours

*/
		// recuperer les congés precedents
		$start_date = '01/01/'.date("Y");
		$end_date = '31/12/'.date("Y");
		$prevDemands = rh_conge_demandPeer::getByDate($start_date,$end_date,$user->oid->object);
		if (preg_match('/exceptionnel/i',$this->legal_days->string))
			$isRegulier = false;
		else
			$isRegulier = true;

		$nbDaysException=0; $countException=0;$countRegular=0;
		foreach ($prevDemands as $demand)
		{
			$days_to_take = $this->days_to_take->object;

			if (preg_match('/exceptionnel/i',$demand->legal_days->string))
			{
				$nbDaysException += $days_to_take;
				$countException++;
			}
			else
			{
				$countRegular++;
			}
		}

		if ($isRegulier)
		{
			// conges réguliers : Le congé régulier ne peut en aucune manière être fractionné en plus de deux tranches 
			if ($countRegular==0)
			{
				// 1ere tranche
				//	 La première tranche doit être d'une durée minimum de dix (10) jours.

				// vérifier qu'il prend au moins 10 jours la premiere tranche
				if ($demandedDays < self::NB_JOURS_MIN_TRANCHE1)
					return preg_replace('/NB_JOURS_MIN_TRANCHE1/',self::NB_JOURS_MIN_TRANCHE1, 'Vous devez prendre au moins NB_JOURS_MIN_TRANCHE1 pour la première tranche');
		
				// laisser un minimum de 7 jours pour la 2eme tranche
				if ($userReliquat-$demandedDays > 0 && $userReliquat-$demandedDays < self::NB_JOURS_MIN_TRANCHE1)
					return preg_replace('/NB_JOURS_MIN_TRANCHE2/',self::NB_JOURS_MIN_TRANCHE2, 'Si vous fractionnez votre congé, vous devez conserver au minimum NB_JOURS_MIN_TRANCHE2 jours pour la deuxième tranche');
			}
			else
			{
				// deuxième tranche: La 2ème tranche d'une durée minimum de sept (7) jours
				if( $demandedDays < 7 )
					return "Vous devez prendre au moins 7 jours consécutifs pour cette période (deuxième tranche)";
			}
		}
		else
		{
			// conges exceptionnels
			/*
				 Il est accordé aux agents TAMCA et OE un congé exceptionnel avec
				 solde à
				 défalquer des droits à congé régulier jusqu'à concurrence de dix(10)
				 jours.
				 Les dix jours peuvent être fractionnés au minimum par ½ journée à
				 3jours au
				 maximum jusqu'à épuisement de dix (10) jours.
			*/
			if( $demandedDays > 3 )
				return "Vous devez prendre au maximum 3 jours pour un congé exceptionnel";

			if( $nbDaysException + $demandedDays > 10 )
				return "Vous devez prendre au maximum 10 jours en total de congés exceptionnels et vous en êtes à $nbDaysException jours";
		}

		
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
		else if ( $userReliquat >= $demandedDays )
		{
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
		}
		else
		{
			$this->infos = array(
					"status"		=> 'error',
					"message"		=> 'Votre demande de cong&eacute;s d&eacute;passe votre cr&eacute;dit actuel de '.($demandedDays-$currentCredit). ' jours',
					"currentCredit" => $currentCredit,
					"lastCredit"    => $lastCredit,
					"demandedDays"  => $demandedDays,
					"reliquat"      => $userReliquat,
					"legalDays"		=> $legalDays,
					"nbHolidays"    => $nbHols
				);
		}

		return $this->infos;
	  }
}

/*

.JORF LASFAR:

* Droit CR
 Catégorie Employé : 18 jours ouvrables.
 Catégorie agent de maîtrise : 24 jours ouvrables.

1.5 Jours tous les 5 ans d'ancienneté.

Règles de calcul des congés régulier du Site jorf Lasfar :
Les jours fériés et les repos hebdomadaire ne sont pas comptés quelque soit
le nombre de congé pris par collaborateur.


*/

class rh_conge_demand_sonasid_jorf extends rh_conge_demand
{
	function __construct($ruleset='Jorf') {
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
		$nbYears = $this->getNbYearsOfService($user,$toDateTime);

		// get type of employee
		$role = $user->csp->string;
		if (preg_match('/agent/i',$role))
			$credit = 24;
		else
			$credit = 18;

		$credit += floor($nbYears/5)*1.5;

		return $credit;
	}

	// v?rifies les regles de coherences pour le cong?s
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
		if (!$this->canLeaveSameDay())
		{
			if( $startDateTime <= $now )
				return "La date de d&eacute;but doit &ecirc;tre sup&eacute;rieure &agrave; la date courante";
		}
		else
		{
			if( $startDateTime+24*60*60 <= $now )
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
		else if ( $userReliquat >= $demandedDays )
		{
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
		}
		else
		{
			$this->infos = array(
					"status"		=> 'error',
					"message"		=> 'Votre demande de cong&eacute;s d&eacute;passe votre cr&eacute;dit actuel de '.($demandedDays-$currentCredit). ' jours',
					"currentCredit" => $currentCredit,
					"lastCredit"    => $lastCredit,
					"demandedDays"  => $demandedDays,
					"reliquat"      => $userReliquat,
					"legalDays"		=> $legalDays,
					"nbHolidays"    => $nbHols
				);
		}

		return $this->infos;
	  }
}

class rh_conge_demand_sonasid_lasfar extends rh_conge_demand_sonasid_jorf
{
	function __construct() {
		parent::__construct('Lasfar');
	}
}

/*

III.SIEGE

Les règles restent les mêmes que celles convenues en début du projet.

RG1 - Le 1er samedi du congé
Les jours fériés et les week-ends (Dimanches) sont retirés des jours à prendre, les samedis sont comptés, et la règle pour le premier samedi du congé est : 
	- Si le premier jour de congé est un Jeudi ou Vendredi le Premier 	Samedi n'est pas compté
	- Si le premier jour de congé est AUTRE que le jeudi / vendredi le 	premier samedi est compté.



*/

class rh_conge_demand_sonasid_siege extends rh_conge_demand
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

		Pour le siège
			Ouvrier/Employé => 18 jours ouvrables
			Agents de Maîtrise  => 24 jours ouvrables
			Cadres => 24 jours ouvrables
			Cadres dirigeants => 26 jours ouvrables

			Soldes initiaux des congés pour les personnes à ancienneté :
			Solde initial =	Solde initial sans ancienneté (selon CSP)
				+ 1,5 jours tous les 5 ans d'ancienneté

	*/
	function getNominalYearCredit($user,$toDateTime) 
	{
		$nbYears = $this->getNbYearsOfService($user,$toDateTime);

		// get type of employee
		$role = $user->csp->string;
		if (preg_match('/agent|ma.+trise/i',$role))
			$credit = 24;
		elseif (preg_match('/dirigeant|direct/i',$role))
			$credit = 26;
		elseif (preg_match('/cadre/i',$role))
			$credit = 24;
		else
			$credit = 18;

		$credit += floor($nbYears/5)*1.5;

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
		/*
		if( $demandedDays > $userReliquat )
			return "Votre cr&eacute;dit ne vous permet pas de prendre ce cong&eacute;";
		*/

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
				$no_remaining_days-= $last_day_weekday - $this->params['nbDaysperWeek']-1;
			}
		}
		// case 2 : at least on 2 weeks
		elseif ($first_day_weekday <= $last_day_weekday)
		{
			if ($first_day_weekday <= 6 && $last_day_weekday >=6) 
			{
				$no_remaining_days--;
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
		$site = $site->string;

		if (preg_match('/nador/i',$site))
			return 'rh_conge_demand_sonasid_nador';
		if (preg_match('/jorf|aciérie/i',$site))
			return 'rh_conge_demand_sonasid_jorf';
		if (preg_match('/lasfar/i',$site))
			return 'rh_conge_demand_sonasid_lasfar';

		return 'rh_conge_demand_sonasid_siege';
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
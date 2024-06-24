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
	
	function __construct() {
		parent::__construct();

		$this->params=array(
			'nbDaysperWeek'=>6,
			'canLeaveSameDay'=>false,
			'ds_holidays'=>'rh.jours_feries'
		);
	}

	// le credit de l'utilisateur acquis pour la periode
	function getNominalYearCredit($user,$toDateTime) 
	{
		$nbYears = $this->getNbYearsOfService($user,$toDateTime);

		// get type of employee
		$role = $user->CSP->string;
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
	function isOkFirstYear($user,$demandedDays,$nbOfficedays)
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

		$startDateTime   = $this->start_date->time;
		$endDateTime	 = $this->end_date->time;
		$halfStartDate   = $this->half_start_date->object;
		$halfEndDate     = $this->half_end_date->object;

		$nbOfficedays = $this->countWorkingDays(
				$startDateTime, $endDateTime,
				$halfStartDate,$halfEndDate,$nbHols);

		// check credit
		if ($demandedDays < 10 || $nbOfficedays<9)
		{
			return 'Vous devez prendre au moins 10 jours de congés dont 9 jours ouvrables (première année de service)';
		}

		return true;
	}

	// vérifies les regles de coherences pour le cong?s
	function checkRules($user) 
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
		if ($prevDemands->count()==0)
		{
			// 1ere tranche

			// vérifier qu'il prend au moins 10 jours la premiere tranche
			if ($demandedDays < self::NB_JOURS_MIN_TRANCHE1)
				return preg_replace('/NB_JOURS_MIN_TRANCHE1/',self::NB_JOURS_MIN_TRANCHE1, 'Vous devez prendre au moins NB_JOURS_MIN_TRANCHE1 pour la première tranche');
	
			// laisser un minimum de 7 jours pour la 2eme tranche
			if ($userReliquat-$demandedDays > 0 && $userReliquat-$demandedDays < self::NB_JOURS_MIN_TRANCHE1)
				return preg_replace('/NB_JOURS_MIN_TRANCHE2/',self::NB_JOURS_MIN_TRANCHE2, 'Si vous fractionnez votre congé, vous devez conserver au minimum NB_JOURS_MIN_TRANCHE2 jours pour la deuxième tranche');
		}
		else
		{
			// autre tranche
			if ($userReliquat > $demandedDays)
				return sprintf('Vous ne pouvez pas fractionner votre congé en plus de 2 tranches. Merci de prendre tous vos congés restant');

			// La 2ème tranche d’une durée minimum de sept (7) jours
			if( $demandedDays < 7 )
				return "Vous devez prendre au moins 7 jours consécutifs pour cette période (deuxième tranche)";
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

		$legalDayType   = $demandeObj->legal_days;
		$hrTime = new HR_Time($this->ctxt);

		$demandedDays = $this->countWorkingDays(
			$startDateField, $endDateField,
			$halfStartDate,$halfEndDate,&$nbHols);

		// get legal days to remove from count
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
	// le credit de l'utilisateur acquis pour la periode
	function getNominalYearCredit($user,$toDateTime) 
	{
		$nbYears = $this->getNbYearsOfService($user,$toDateTime);

		// get type of employee
		$role = $user->CSP->string;
		if (preg_match('/agent/i',$role))
			$credit = 24;
		else
			$credit = 18;

		$credit += floor($nbYears/5)*1,5

		return $credit;
	}

	// v?rifies les regles de coherences pour le cong?s
	function checkRules($user) 
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

		$legalDayType   = $demandeObj->legal_days;

		$demandedDays = $this->countWorkingDays(
			$startDateField, $endDateField,
			$halfStartDate,$halfEndDate,&$nbHols);

		// get legal days to remove from count
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
}

/*

III.SIEGE

Les règles restent les mêmes que celles convenues en début du projet.


*/

class rh_conge_demand_sonasid_siege extends rh_conge_demand
{
	// le credit de l'utilisateur acquis pour la periode
	function getNominalYearCredit($user,$toDateTime) 
	{
		return 18;
	}

	// vérifies les règles de coherences pour le congés
	function checkRules($user) 
	{
		$demandeObj=$this;

		$currentDemandID = $this->oid;
		$startDateTime   = $this->start_date->time;
		$endDateTime	 = $this->end_date->time;
		$halfStartDate   = $this->half_start_date->object;
		$halfEndDate     = $this->half_end_date->object;

		if ($startDateTime==0 or $endDateTime==0)
			return "Merci d'indiquer les dates de votre cong&eacute; ci-dessus";
		
		// S'il y a des v?rifications suppl?mentaires  faire, les faire au sur $demandeArray
		$hrTime = new HR_Time($this->ctxt);

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

		$legalDayType   = $demandeObj->legal_days;
		$hrTime = new HR_Time($this->ctxt);

		$demandedDays = $this->countWorkingDays(
			$startDateField, $endDateField,
			$halfStartDate,$halfEndDate,&$nbHols);

		// get legal days to remove from count
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

class rh_conge_demand_sonasidPeer extends rh_conge_demandPeer
{
}

?>
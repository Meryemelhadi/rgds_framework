<?php

include_once(NX_XLIB.'lib/rh/conge/workflow.inc');

class rh_conge_demand_maroc extends rh_conge_demand
{
	function __construct() {
		parent::__construct('Maroc');

		$this->params=array(
			'nbDaysperWeek'=>defined(RH_NB_DAYS_PER_WEEK)?RH_NB_DAYS_PER_WEEK:5,
			'canLeaveSameDay'=>defined(RH_CAN_LEAVE_SAME_DAY)?RH_CAN_LEAVE_SAME_DAY:false,
			'ds_holidays'=>'rh.jours_feries'
		);
	}

	// le credit de l'utilisateur acquis pour la periode
	function getNominalYearCredit($user,$toDateTime) 
	{
		return 18;
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
		return 'rh_conge_demand_maroc';
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
<?php 
require_once(NX_XLIB.'lib/framework.inc');

if (!defined('HR_RULE_SET'))
	define('HR_RULE_SET','maroc');

include_once(NX_XLIB.'lib/rh/conge/'.HR_RULE_SET.'/demand.inc');

class aspects_records_rh_conge_update_holiday extends NxModule
{
	var $record;

	var $start_date;
	var $end_date;

	function __construct(&$record,&$module){
		$this->NxModule($module->props,$module->parent);
		$this->attachTo($module);
		$this->record = &$record;
	}

	function onNew(&$errorCB) {
	}

	function onBeforeSave($isInsert,&$errorCB,&$record) {

		// reload day before update
		if (!$isInsert)
		{
			$oid = $record->oid->object;
			$day = new PersistentObject('rh.jours_feries.by_oid@rh','db',array('oid'=>$oid));
			
			$this->start_date = $day->start_date->db;
			$this->end_date = $day->end_date->db;
		}
		return true;
	}
	function onSave($isInsert,&$errorCB,&$record) {

		if (!$isInsert)
		{
			// refresh demands in the period before update
			$this->refreshDemands($this->start_date,$this->end_date);
		}

		// refresh demands in the period after update
		$start_date = $record->start_date->db;
		$end_date = $record->end_date->db;
		$this->refreshDemands($start_date,$end_date);
	}

	static $holidays = array();
	function onBeforeDelete(&$errorCB,&$record) {
		// reload the full object with date fields
		$oid = $record->oid->object;
		$day = new PersistentObject('rh.jours_feries.by_oid@rh','db',array('oid'=>$oid));
		
		// store day in array (NB. aspect doesnt keep context for delete..)
		self::$holidays[$oid] = $day;

		// $this->start_date = $day->start_date->db;
		// $this->end_date = $day->end_date->db;

		return true;
	}
	function onDelete(&$errorCB,&$record) {

		$oid = $record->oid->object;
		$day = self::$holidays[$oid];

		// refresh demands in the period before delete
		$this->refreshDemands($day->start_date->db,$day->end_date->db);
	}

	function refreshDemands($start_date,$end_date) {

		// get all demands in period
		$demands= rh_conge_demandPeer::getByDate($start_date,$end_date,null,$demand_state=null);

		foreach ($demands->array as $demand)
		{
			$demand->object->refreshReliquat();
		}

	}

}
?>
<?php 
require_once(NX_XLIB.'lib/framework.inc');

if (!defined('HR_RULE_SET'))
	define('HR_RULE_SET','maroc');

include_once(NX_XLIB.'lib/rh/conge/'.HR_RULE_SET.'/demand.inc');

class aspects_records_rh_conge_update_holidays extends NxModule
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

	function onBeforeSave($isInsert,&$errorCB) {
		$record = &$this->record;

		// reload day before update
		$oid = $record->oid->object;
		$day = new PersistentObject('rh.jours_feries.by_oid','db',array('oid'=>$oid));
		
		$this->start_date = $day->start_date->string;
		$this->end_date = $day->end_date->string;
	}
	function onSave($isInsert,&$errorCB) {
		// refresh demands in the period before update
		$this->refreshDemands($this->start_date,$this->end_date);

		// refresh demands in the period after update
		$record = &$this->record;
		$start_date = $record->start_date->string;
		$end_date = $record->end_date->string;
		$this->refreshDemands($start_date,$end_date);
	}

	function onBeforeDelete(&$errorCB) {
		$record = &$this->record;

		$this->start_date = $record->start_date->string;
		$this->end_date = $record->end_date->string;
	}
	function onDelete(&$errorCB) {
		// refresh demands in the period before delete
		$this->refreshDemands($this->start_date,$this->end_date);
	}

	function refreshDemands($start_date,$end_date) {

		// get all demands in period
		$demands= rh_conge_demandPeer::getByDate($start_date,$end_date,$uoid,$demand_state=null);

		foreach ($demands->array as $demand)
		{
			$demand->object->refreshReliquat();
		}

	}

}
?>
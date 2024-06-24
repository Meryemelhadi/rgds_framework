<?php

require_once(NX_XLIB.'lib/framework.inc');
include_once(NX_XLIB.'lib/rh/conge/workflow.inc');
require_once(NX_LIB.'NxData_ArrayRecords.inc');

/* application */
class lib_rh_conge_records_demand 
{
	var $ctxt;

	function __construct($ctxt,$instance) {
		$this->ctxt=$ctxt;
	}

	function getRecords($recDesc,$errorCB,$data,$props) 
	{
		$view = $recDesc->view;
		$uoid= Framework::getUser()->getProperty('oid',0);

		$o = new rh_conge_demand();
		$col = new PersistentCollection();
		$col->load($o->ds_by_date,'db', array(
			'empid'=>$userOid
			));
		return $col->records;

		/*$demands= rh_conge_demandPeer::getByDate('2009-01-01','2009-12-31',$uoid,$demand_state=null,'records');

		if ($demands)
			return $demands;

		return null;
		*/
	}

	function countRecords($recDesc,$errorCB)                 {}
	function addRecords($recDesc,$errorCB,$records,$dbOp)    {}
	function addRecord($recDesc,$errorCB,$record,$dbOp)      {}
	function deleteRecord($recDesc, $errorCB,$props,$record) {}
}

?>
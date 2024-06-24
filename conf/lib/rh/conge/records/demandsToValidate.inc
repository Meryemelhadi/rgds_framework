<?php

require_once(NX_XLIB.'lib/framework.inc');

if (!defined('HR_RULE_SET'))
	define('HR_RULE_SET','maroc');

include_once(NX_XLIB.'lib/rh/conge/'.HR_RULE_SET.'/demand.inc');

/* application */
class lib_rh_conge_records_demandsToValidate 
{
	var $ctxt;

	function __construct($ctxt,$instance) {
		$this->ctxt=$ctxt;
	}

	function getRecords($recDesc,$errorCB,$data,$props) 
	{
		$uoid= Framework::getUser()->getProperty('oid',0);
		$ds=$recDesc->view;

		$col = rh_conge_demandPeer::getByDs('rh.conge.validation.demandsToValidate',$uoid,array('validator_oid'=>$uoid),$data);

		return $col->records;
	}

	function countRecords($recDesc,$errorCB)                 {}
	function addRecords($recDesc,$errorCB,$records,$dbOp)    {}
	function addRecord($recDesc,$errorCB,$record,$dbOp)      {}
	function deleteRecord($recDesc, $errorCB,$props,$record) {}
}

?>
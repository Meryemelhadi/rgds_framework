<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_Session
{
	var $app;
	function __construct(&$props)
	{
		$this->props =& $props;
	}
	
	// data source interface
    function & getRecords($view, &$errorCB)
	{			
		// get record description
		$data = & $this->props->getPlugin("Data");
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return $errorCB->fatal("can't read rec desc",__FILE__,__LINE__);
		
		// get session record
		require_once(NX_LIB . "NxData_RecordInputSource.inc");
		$record = new NxData_RecordInputSource($recDesc);

		// get session slot key
		$slotKey=$this->props->getProperty($view.'.session.slot','record_edit');
		
		// get a reference to the slot
		if (isset($_SESSION[$slotKey]))
			$slot = & $_SESSION[$slotKey];
		else
		{
			$slot = array();
			$errorCB->addError("session time out");
		}

	 	$record->initRecord($slot, "array", $errorCB);
		return $record;
	}

	function & getRecordCount($view, &$errorCB)
	{		
		// check presence of fields ?
		// $_POST[]
		return 1;
	}
	
	// add/modify records with view name
	function putRecords($view, &$errorCB, $records,$dbOp=null)
	{	
		$slotKey=$this->props->getProperty($view.'.session.slot','record_edit');
		
		// get a reference to the slot
		if (isset($_SESSION))
		{
			$_SESSION[$slotKey] = array();
			$slot = & $_SESSION[$slotKey];
		}
		else
		{
			$slot = array();
		}
				
		for ($fieldIt = $record->getFieldIterator(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			$slot[$field->getName()] = $field->toObject();
		}
	}
	
	// ds interface
	function deleteRecords($view, &$records, &$errorCB)
	{
		$slotKey=$this->props->getProperty($view.'.session.slot','record_edit');
		
		// get a reference to the slot
		if (isset($_SESSION))
		{
			$_SESSION[$slotKey]=null;
		}
				
		return true;
	}
	
	// ============ SESSION ============ obsolete ?
	function storeRecord(& $record,$slotRef)
	{
		// get a reference to the slot
		if (isset($_SESSION))
		{
			$_SESSION[$slotRef] = array();
			$slot = & $_SESSION[$slotRef];
		}
		else
		{
			$slot = array();
		}
				
		for ($fieldIt = $record->getFieldIterator(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			$slot[$field->getName()] = $field->toObject();
		}
	}
	
	function loadRecords($slotRef, &$recDesc, &$errorCB)
	{		
		require_once(NX_LIB."NxData_Session.inc");
		$records = new NxData_SessionInput($slotRef, $recDesc, $errorCB);
		return $records;
	}
	
	function loadRecord($slotRef, &$recDesc, &$errorCB)
	{
		require_once(NX_LIB."NxData_Session.inc");
		$records = new NxData_SessionInput($slotRef, $recDesc, $errorCB);
		return $records->getRecord();
	}
}

?>

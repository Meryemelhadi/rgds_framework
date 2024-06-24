<?php

/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB."NxData_RecordInputSource.inc");
require_once(NX_LIB."NxData_Output.inc");

// form data container
class NxData_SessionInput extends NxData_RecordInputSource
{
	function __construct($slotRef, &$recDesc, &$errorCB) 
    {
		parent::__construct($recDesc);
		
		// get a reference to the slot
		if (isset($_SESSION[$slotRef]))
		{
			$slot = & $_SESSION[$slotRef];
		}
		else
		{
			$slot = array();
			$errorCB->addError("session time out");
		}

	 	$this->initRecord($slot, "array", $errorCB);
    }
}

class NxData_SessionOutput extends NxData_Output
{
	var $slot;
	
	function __construct($slotRef, &$recDesc)
	{
		parent::__construct($recDesc,"store");
		
		// get a reference to the slot
		if (isset($_SESSION))
		{
			$_SESSION[$slotRef] = array();
			$this->slot = & $_SESSION[$slotRef];
		}
		else
		{
			$this->slot = array();
		}
	}

	function writeHeader()
	{
		$this->slot = array();
	}
	
	function writeFooter()
	{
	}

	function writeField(&$field)
	{
		$fmethod = $this->fmethod;
		$fname = $field->getName();
		$this->slot[$fname] = $field->$fmethod();
	}
}

?>

<?php

/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB . "NxData_Record.inc");

// single record  data source
class NxData_RecordInputSource extends NxData_Records
{
	var $record;

	// dataFormat: data source format, currently  "form" or "db"
	 function __construct(&$recDesc)
	 {
	 	parent::__construct($recDesc);
	 	$this->record = new NxData_Record($recDesc);
	 }
	 
	 function initRecord(&$data, $dataFormat, &$errorCB,$media=null)
	 {
	 	$this->record->initFromArray($data, $dataFormat, $errorCB,$media);
	 }
	
	 // data source interface
	 function getRecordIterator() { $this->reset(); return $this; }

	 // record collection interface
	 function & getRecord()  { return $this->record; }
	 function & getRecDesc() {	return $this->record->recDesc; }
	function getProperty($name,$deft=null,$cascade=true,$format="HTML") {
		return $this->record->getProperty($name,$deft,$cascade,$format); 
	}	

	 // record iterator interface
	 function next() { 
	 	$this->save = $this->record; 
	 	$this->record = null;
	 }
	 function &reset() { 
	 	if ($this->record == null)
	 		$this->record = $this->save;
	 	else
			$this->save = $this->record;
			
	 	return $this; 
	 }
	 
	function &getCurrent() { 
    	$this->className=$this->record->recDesc->getClass();
		return $this->record; 
	}
	function isValid()	{ return isset($this->record); }
	function count()   { return isset($this->record) ? 1 : 0; }

	function onDelete(&$errorCB)
	{
		if ($this->record)
			$this->record->onDelete($errorCB);
		else if ($this->save)
			$this->save->onDelete($errorCB);
	}
	 
}

?>
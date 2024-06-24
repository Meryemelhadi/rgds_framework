<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB."NxData_Record.inc");

// access a property set as a record
class NxData_PropertyRecords
{
	var $records;
	var $index;

	// dataFormat: data source format, currently  "form" or "db"
	 function __construct(&$props,$cascade=true)
	 {
	 	$recDesc = $this->_getRecDesc();
	 	$errorCB = new NxErrorCB();

	 	$this->index= 0;
	 	$this->records = array();
	 	for ($ps =& $props; $ps != null; $ps =& $ps->parent)
	 	{
		 	$propArray = $ps->props;
		 	foreach ($propArray as $n=>$v)
		 	{
		 		$recData = array("_name"=>$n, "_value" => $v);
		 		$record = new NxData_Record($recDesc);
		 		$record->initFromArray($recData,"array",$errorCB);
		 		$this->records[] = & $record;
		 	}
		 	
		 	if (!$cascade)
		 		break;
	 	}
	 }
	
	 // data source interface
	 function getRecordIterator() { return $this; }

	 // record collection interface
	 function & getRecord()  { return $this->getCurrent(); } // deprecated
	 function & getRecDesc() { return $this->record->recDesc; }

	 // record iterator interface
	 function & next() {
	 	$this->index++;
	 	return $this->getCurrent();
	 }
	 function &reset() { 
	 	$this->index = 0;
	 	return $this->getCurrent();
	 }
	 function &getCurrent() { 
	 	if (count($this->records)>$this->index)
	 		return $this->$records[$this->index]; 
	 	else
	 		return null;
	 }
	 function isValid()	{ return (count($this->records)>$this->index); }
	 function count()   { return count($this->records); }
	 
	// record descriptor for properties
	function _getRecDesc(&$props)
	{
		static $fprops=
			array(	
				array("name"=>"name", 
					 "type"=>"text",
					 "alias"=>"nxk", "label"=>"Name", 
					 "isKey"=>"primary"),
				array("name"=>"values", 
					 "type"=>"text",
					 "label"=>"Value"));

		static $rprops = array(	
				"name"=>"_properties",
				"srcName"=>"properties", 
				"table"=>"_properties", 
				"tableSrcName"=>"properties");
		
		return new NxData_RecordDesc($rprops, $fprops, &$props);
	}

}
	
	
?>
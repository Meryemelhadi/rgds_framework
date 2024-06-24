<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

class NxPlug_http_post
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}
	
    function & getRecords($view, &$errorCB)
	{
		// get record description
		$data = & $this->props->getPlugin("Data");
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return $errorCB->fatal("can't read rec desc",__FILE__,__LINE__);
		
		// get HTTP record	
		require_once(NX_LIB . "NxData_RecordInputSource.inc");
		$record = new NxData_RecordInputSource($recDesc);
	 	$record->initRecord($_POST, "form", $errorCB);
	 	
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
		return false;
	}
}

?>
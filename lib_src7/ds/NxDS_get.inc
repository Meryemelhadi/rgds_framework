<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

// same as "http_post"
class NxDS_get extends NxDS_Source
{
	function __construct(&$props)
	{
		parent::__construct($props);
	}
	
    function getRecords($view, &$errorCB,$data=null)
	{
		if (($recDesc = & $this->getRecDesc($view,$errorCB))==null)
			return null;
		
		// get HTTP record	
		require_once(NX_LIB . "NxData_RecordInputSource.inc");
		$record = new NxData_RecordInputSource($recDesc);
	 	$record->initRecord($_GET, "form", $errorCB,'get');
	 	
	 	return $record;
	}

	function getRecordCount($view, &$errorCB)
	{		
		// check presence of fields ?
		// $_POST[]
		return 1;
	}
	
	// add/modify records with view name
	function putRecord(&$record,$view, &$errorCB,$dbOp=null)
	{	
		if ($record!=null)
		{
			$fields=$record->getProperties(false,'object');
			if ($fields)
				$_GET += $fields;
				
			return true;
		}
		return false;
	}
	
	// add/modify records with view name
	function putRecords(&$records,$view, &$errorCB,$dbOp=null)
	{	
		return $this->putRecord($view,$errorCB,$records->getCurrent(),$dbOp);
	}

}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_http_post extends NxDS_Source
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
	 	$record->initRecord($_POST, "form", $errorCB,'http_post');
	 	
	 	return $record;
	}

	function getRecordCount($view, &$errorCB)
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
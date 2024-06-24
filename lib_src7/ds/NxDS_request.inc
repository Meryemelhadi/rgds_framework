<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

// same as "http_post"
class NxDS_request extends NxDS_Source
{
	function __construct(&$props)
	{
		parent::__construct($props);
	}
	
    function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		if (($recDesc = $this->getRecDesc($view,$errorCB,$props))==null)
			return null;
		
		// get HTTP record	
		require_once(NX_LIB . "NxData_RecordInputSource.inc");
		$record = new NxData_RecordInputSource($recDesc);
	 	$record->initRecord($_REQUEST, "form", $errorCB,'request');
	 	
	 	return $record;
	}

	function getRecordCount($view, &$errorCB)
	{		
		// check presence of fields ?
		// $_POST[]
		return 1;
	}
}

?>
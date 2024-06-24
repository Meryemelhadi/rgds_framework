<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_records extends NxDS_Source
{
	function __construct(&$props)
	{
		parent::__construct($props);
	}
	
    function getRecords($view, &$errorCB,$data=null)
	{			
    	if (($recDesc = & $this->getRecDesc($view,$errorCB))==null)
			return null;
			
    	if (isset($data))    			
		{
			require_once(NX_LIB . 'NxData_ArrayRecords.inc');
			return new NxData_ArrayRecords($data, $recDesc,'db');
		}
	
	 	return null;	
	}

	function getRecordCount($view, &$errorCB,$data=null)
	{		
		// check presence of fields ?
		// $_POST[]
		return $data==null?0:count($data);
	}
	
	// add/modify records with view name
	function putRecords($view, &$errorCB, $records,$dbOp=null)
	{	
		return false;
	}
}

?>
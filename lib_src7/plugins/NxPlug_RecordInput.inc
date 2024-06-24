<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/* 
	This plugin manage input of records from various data sources.
*/
class NxPlug_RecordInput
{
	function __construct(&$props)
	{
		$this->props =& $props;
		$this->data = $this->props->getPlugin("Data");
	}

	// Returns a set of records from a data source, following data view settings.
	// view: data view, $media: data source type, $errorCB, 
	// $slotKey: access of record by key in a data source
	function & getRecords($view, $media, &$errorCB, $slotKey="record_edit")
	{
		// get dataset description
		$recDesc = & $this->data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		// get records from media
		switch (strtolower($media))
		{
			case "post":
				require_once(NX_LIB."NxData_HttpForm.inc");	
				$records = & new NxData_HttpForm("POST",$recDesc, $errorCB);
				break;
			case "db": //update
				$this->db = $this->props->getPlugin("DB");
				$records = & $this->db->getDBRecords($recDesc,$errorCB,null,null);
				break;
			case "session":
				require_once(NX_LIB."NxData_Session.inc");
				$records = & new NxData_SessionInput($slotKey, $recDesc, $errorCB);	
				break;
			case "empty":
			default:
				$records =& $this->data->getEmptyRecord($recDesc,$errorCB);
				break;
		}
		
		return $records;
	}
	
}

?>
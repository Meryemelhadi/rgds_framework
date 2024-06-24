<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

// same as "http_post"
class NxDS_session extends NxDS_Source
{
	function __construct(&$props,$instance='ds_records')
	{
		parent::__construct($props);
		if (!$instance)
			$instance='ds_records';
			
		$this->instance = $instance;

		if (isset($_SESSION) && !isset($_SESSION[$this->instance]))
			$_SESSION[$instance]=array();
	}
	
    function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		if ($props && isset($props['key']))
			$key=$props['key'];
		elseif (is_string($data))
			$key=$data;
		else
			$key=$view;
			
		if (($recDesc = & $this->getRecDesc($view,$errorCB))==null)
			return null;
			
		if (!isset($_SESSION[$this->instance][$key]))
			return null;
		
		// get session record	
		require_once(NX_LIB . 'NxData_RecordInputSource.inc');
		$record = new NxData_RecordInputSource($recDesc);
		$values=$_SESSION[$this->instance][$key];
	 	$record->initRecord($values,'array', $errorCB,'session');
	 	
	 	return $record;
	}

	function getRecordCount($view, &$errorCB)
	{		
		$key=$view;
		if (isset($_SESSION[$this->instance][$key]))
			return 1;
		else
			return 0;		
	}
	
	// add/modify records with view name
	function putRecord(&$record,$view, &$errorCB,$dbOp=null,$props=null)
	{	
		if ($props && isset($props['key']))
			$key=$props['key'];
		else
			$key=$view;

		$fields=$record->getProperties(false,'object');
		
		// store only first record for now..
		if ($dbOp=='insert')
			$_SESSION[$this->instance][$key]=$fields;
		else
			$_SESSION[$this->instance][$key]=$fields+$_SESSION[$this->instance][$key];
		
		return true;
	}
	
	// add/modify records with view name
	function putRecords(&$records,$view, &$errorCB,$dbOp=null,$props=null)
	{	
		return $this->putRecord($view,$errorCB,$records->getCurrent(),$dbOp,$props);
	}
	
	function deleteRecords($view, &$records, &$errorCB)
	{
		$key='record_input';
		if (isset($_SESSION[$this->instance][$key]))
		{
			unset($_SESSION[$this->instance][$key]);	
			return true;
		}
		else
			return false;
	}

}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

// same as "http_post"
class NxDS_cookie extends NxDS_Source
{
	function __construct(&$props)
	{
		parent::__construct($props);
	}
	
    function getRecords($view, &$errorCB,$data=null)
	{
		$cview=preg_replace('/[ .-]/','_',$view);
		
		if (!isset($_COOKIE[$cview]))
			return null;
			
		$rec_data=@unserialize(base64_decode($_COOKIE[$cview]));
		
		if ($rec_data!==false) {	
			if (($recDesc = & $this->getRecDesc($view,$errorCB))==null)
				return null;
			
			// get HTTP record	
			require_once(NX_LIB . "NxData_RecordInputSource.inc");
			$record = new NxData_RecordInputSource($recDesc);
		 	$record->initRecord($rec_data, 'array', $errorCB,'cookie');
		}
			 	
	 	return $record;
	}

	function getRecordCount($view, &$errorCB)
	{		
		$view=preg_replace('/[ .-]/','_',$view);
		
		if (!isset($_COOKIE[$view]))
			return 0;
			
		return 1;
	}
	
	// add/modify records with view name
	function putRecord(&$record,$view, &$errorCB,$dbOp=null)
	{	
		$cview=preg_replace('/[ .-]/','_',$view);
		if ($record!=null)
		{
			$fields=$record->getProperties(false,'object');
			$duration=$this->getProperty('cookie.duration',30*24*3600);
			setcookie($cview,base64_encode(serialize($fields)),time()+$duration);		
			return true;
		}
		return false;
	}
	
	// add/modify records with view name
	function putRecords(&$records,$view, &$errorCB,$dbOp=null)
	{	
		return $this->putRecord($view,$errorCB,$records->getCurrent(),$dbOp);
	}
	
	function deleteRecords($view, &$records, &$errorCB)
	{
		$cview=preg_replace('/[ .-]/','_',$view);
		if (!isset($_COOKIE[$cview]))
			return false;
		
		setcookie($view,null,time()-3600);		
		return true;
	}
	
}

?>
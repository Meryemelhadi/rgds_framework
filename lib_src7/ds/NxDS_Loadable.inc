<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
	interface for all data sources.
*/
class NxDS_Loadable
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}

	// load docs from html (<a> tags) and returns it as a record.
    function getRecords($view, &$errorCB,$data=null)
	{
		return null;
	}

	function getRecordCount($view, &$errorCB)
	{		
		return 0;
	}
	
	// add/modify records with view name
	function putRecords($view, &$errorCB, $records,$dbOp=null)
	{	
		return false;
	}

	function &getRecDesc($view, &$errorCB)
	{
		// get record description
		$data = & $this->props->getPlugin('Data');
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return $errorCB->fatal("can't read rec desc",__FILE__,__LINE__);
			
		return $recDesc;
	}
}
?>
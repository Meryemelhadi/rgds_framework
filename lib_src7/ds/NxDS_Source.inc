<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
	interface for all data sources.
*/
class NxDS_Source extends NxProperties
{
	function __construct(&$props)
	{
		$empty=array();
		parent::__construct($empty,$props);
		// $this->props = &$props;
	}
	
	// load docs from html (<a> tags) and returns it as a record.
    function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		return null;
	}

	function getRecordCount($view, &$errorCB)
	{		
		return 0;
	}
	
	// add/modify records with view name
	// op = 'update' or 'insert'
	function putRecords(&$records,$view, &$errorCB,$dbOp=null,$props=null)
	{	
		return false;
	}
	
	// add/modify records with view name
	// op = 'update' or 'insert'
	function putRecord(&$record,$view, &$errorCB,$dbOp=null,$props=null)
	{	
		return false;
	}
	
	function getLastInsertID(&$id,&$errorCB) {
		return 0;
	}

	function deleteRecords($view, &$records, &$errorCB)
	{
		return true;
	}

	// --------- utilities -------
	function &file_get_contents($filename) 
	{
		$fp = @fopen($filename, "r");
		if (!($fp))
			return 0;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		return $temp;
	}
	
}
?>
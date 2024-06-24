<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

class NxPlug_Page
{
	function __construct(&$props)
	{
		$this->props=&$props;
		$this->error = new NxErrorCB();
	}

	// get records from a view and put them in an array of links.
	// Each link is an array of link parameters (ex. url,text,title,index) and "urlParam" 
	// that contains urlparameters with the record key(s).
	// sets the result into a property.
	function & getLinks($view,$propName=null,$media='DB')
	{
		$ds = $this->props->getPlugIn($media);
		/*
		$linkRole = $props->getProperty($view.".attribute","role");

		// set property with an array
		$aLinks = &$db->getRecordsAndRemap($view,$errorCB,$linkRole);
		$props->setProperty($propName,$aLinks);
		return $aLinks;
		*/
		$links =& $ds->getRecords($view,$this->error);
		
		if ($propName)
			$this->props->setProperty($propName,$links);
			
		return $links;
	}
	
	// get a record from DB and sets it to a property
	function & getRecord($view,$propName=null,$media='DB')
	{
		// get data source plugin
		$ds = $this->props->getPlugIn($media);
		
		// get records
		$records = & $ds->getRecords($view, $this->error);
		
		// get first record from collection
		if ($records != null && $records->count() > 0)
		{
			$recIt = $records->getRecordIterator();
			$record = & $recIt->getCurrent();
			if ($propName)
				$this->props->setProperty($propName,$record);
			return $record;
		}
		return null;
	}
	// get a record from DB and sets it to a property
	function & getRecords($view,$propName,$media='DB')
	{
		// get data source plugin
		$ds = $this->props->getPlugIn($media);
		
		// get records
		$records = & $ds->getRecords($view, $this->error);

		// return record collection if ok and set property
		if ($records != null && $records->count() > 0)
		{
			$this->props->setProperty($propName,$records);
			return $records;
		}
		return null;
	}

	// redirect to another page
	function redirectTo($propName,$urlParams="")
	{
		header("Location:".$this->props->getProperty($propName,$propName).$urlParams);
	}

	function getUrlParams()
	{
		return $_SERVER['QUERY_STRING'];
	}
}

?>
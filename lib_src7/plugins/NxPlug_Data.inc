<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB."NxData_Record.inc");
require_once(NX_LIB."NxData_RecordInputSource.inc");

class NxPlug_Data
{
	var $app;
	
	function  NxPlug_Data(& $app)
	{
		$this->app = & $app;
	}

	function &getEmptyRecord(& $recDesc, &$errorCB)
	{
		// $recDesc = $this->getRecDesc($dbview);
		$d=array();
		$ris = new NxData_RecordInputSource($recDesc);
		$ris->initRecord($d,"default",$errorCB);
		return $ris;
	}

	function &getEmptyRecordFromView($view)
	{
		// get dataset description
		$recDesc = & $this->getRecDesc($view);
		if ($recDesc == null)
			return null;
			
		$d=array();
		$errorCB = new NxErrorCB();
		$ris = new NxData_RecordInputSource($recDesc);
		$ris->initRecord($d,"default",$errorCB);
		if ($errorCB->isError())
			return null;
		else
			return $ris;
	}
	
	// NB. by convention viewName should include "db." prefix
	function & getRecDesc($viewName,$check=false,$errorUrl=null)
	{
		// get database view (set of fields to display)		
		$dbview = $this->app->getProperty($viewName.'.db',null);
		
		// for compatibility with previous versions
		if (!isset($dbview))
			$dbview = $this->app->getProperty('db.'.$viewName,$viewName);
			
		if (!isset($dbview))
		{
			// error: coming from a direct URL (bookmark or "previous/forward" button)
			$errorDesc = array(
				"msg" => "Error: undefined table view name: ".$viewName,
				"button" => "console",
				"button.url" =>$errorUrl
			);
			
			$this->app->runView("view.message",$errorDesc);
			return null;
		}
		
		// get record desc
		$properties = array();
		require(NX_DATA.$dbview."_fprops.inc");
		// TODO: copy or reference ?
		$fprops = & $properties["fprops"];
		$rprops =  @$properties["rprops"];
		$recDesc = new NxData_RecordDesc($rprops,$fprops,$properties["falias"],
										 $this->app,$properties["frole"]);
		return $recDesc;
	}

}

?>
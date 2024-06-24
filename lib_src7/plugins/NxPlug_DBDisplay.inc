<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** application server plugin, manage the display of a record from the database.
  * @dependencies propoerty bag (NxProperties) and datbase plugin (NxApp_DB)
  */
class NxPlug_DBDisplay
{
	var $props;
	var $db;
	
	function  NxPlug_DBDisplay(&$props)
	{
		$this->props = & $props;
		$this->db = & $props->getPlugin("DB");
	}
	
	/** 
	  * retreive one/a set of records and display it/them.
	  * In case of error, display a message through the "message" view.
	  *
	  * @parameter dbViewName database set of fields to display
	  * @parameter htmlViewName html view name for displaying records.
	  * @parameter fmethod which field method (format) to use 
	  *            (toHTML, toForm, toDB, toString or toObject)
	  * @parameter recordURL base url for the display of a single record
	  * @parameter listURL url for the display of the lsit of records
	  *
	  * @return operation success
	  */
	function runDisplayQuery($view,$fmethod="toHTML",$recordURL,$listURL)
	{
		$app = & $this->props;
		$db  = & $this->db;
		$data = & $app->getPlugin("Data");

		$errorCB = new NxErrorCB();
		
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return $errorCB->fatal("can't read rec desc",__FILE__,__LINE__);
		
		// get recordset LIMIT
		$maxNbRecords = $app->getProperty("db.".$view. ".nbRecordPerPage",null);
		$indexFirst = $app->getProperty("db.".$view. ".indexFirstRecord",null);
		
		// get records from DB
		$records = $db->getDBRecords($recDesc,$errorCB,$indexFirst,$maxNbRecords);
		if( $records != null )
		{
			// display table
			$viewProps = array(
				"records" => $records,
				"fmethod" => "toHTML",
				"recordURL" => $recordURL,
				"listURL"  => $listURL,
				"button.redirect" => "back",
				"button.url" => $listURL
			);
			
			if ($records->count() > 0)
				$viewProps["isEmpty"] = "false";
			else 
			{
				$viewProps["isEmpty"] = "true";
				$viewProps["msg"] = $app->getString("Empty table");
			}
			$app->runView("view.".$view,$viewProps);
		}
		else
		{
			// database error
			$viewProps = array(
				"msg" => "Error can't display ".$view." : " . $errorCB->getErrorMsg(), 
				"button.redirect" => "back",
				"button.url" => $listURL
			);				
			$app->runView("view.message",$viewProps);
			return false;
		}		
		
		return true;
	}	
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxApplication.inc');

/** Application: List records from a database table
  * properties:
  * $appProps = array(
  *		"table" => "News",
  *		"nbRecordPerPage" => 15);
  */
class NxApp_DisplayTable extends NxApplication
{
	function NxApp_DisplayTable($props,&$parentDesc)
	{
		$this->NxApplication("NxApp_DisplayTable",$props,$parentDesc);
		$this->ui 	= $this->getPlugIn("HtmlUI");
		$this->data = $this->getPlugIn("Data");
		$this->db = $this->getPlugIn("DB");
		$this->dispRec = $this->getPlugIn("DBDisplay");
	}
	
	function run()
	{		
		switch($this->operation)
		{	
			// display a single record
			case "dispItem":
				$this->dispRec->runDisplayQuery(
					"db.view.dispRecord",
					"dispRecord",
					"toHTML",
					$this->getUrl("","dispItem"),
					$this->getUrl("","list")
					);
				break;

			case "list":
			default:
				
				$this->setProperty("viewRecord.url",$this->getUrl("","dispItem"));
				$this->dispRec->runDisplayQuery(
					"db.view.listRecords",
					"listRecords",
					"toHTML",
					$this->getUrl("","dispItem"),
					$this->getUrl("","list")
					);
				break;
		}

		// store current state in session
		$this->saveState();
	}
}

?>

<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_DIS_LOGIC."NxApp_DisplayTable.php");

/** Application: manages a database table (creation/deletion)
  * properties:
  * $appProps = array(
  *		"table" => "News",
  *  	"databaseAdmin" => "databaseAdmin", // main console view
  * 	"errorMsg" => "errorMsg",  			// error message view
  *		"message"  => "message",			// message view
  *		"nbRecordPerPage" => 15);
  */

class NxApp_ManageTable2 extends NxApp_DisplayTable
{
	function NxApp_ManageTable2($props,&$parentDesc)
	{
		$this->NxApplication("NxApp_ManageTable2",$props,$parentDesc);
		$this->ui 	= & $this->getPlugIn("HtmlUI");
		$this->data = & $this->getPlugIn("Data");
		$this->session = & $this->getPlugIn("Session");
		$this->dispRec = & $this->getPlugIn("DBDisplay");
		$this->editRec = & $this->getPlugIn("DBEdit");
	}
		
	function run()
	{	
		$this->setProperty("console.url",$this->getUrl("","console"));
		$this->setProperty("newRecord.url",$this->getUrl("","newRec"));
		$this->setProperty("viewRecord.url",$this->getUrl("","dispItem"));
		$this->setProperty("editRecord.url",$this->getUrl("","updateRec"));
		$this->setProperty("deleteRecord.url",$this->getUrl("","deleteRec"));

		$cancelUrl = $this->getProperty("application.from_page",$this->getUrl("","console"));
		
		switch($this->operation)
		{	
			case "newRec":
				switch($this->step)
				{
					case "end":
						// step 4: store record in db
						if ($this->checkPreviousStep("[][start]","console",""))
						{
							if ($this->editRec->runSaveRecord("newRec",true,null,"post","record_edit",false))
							{
								$this->finish();
								$this->redirectTo("application.to_page");
							}
							$this->finish();
							return;
						}
						break;
					case "start":
					default:
						// step1: display empty record
						$this->startOp();
						$this->editRec->runEditRecord("newRec", "empty",
								$cancelUrl,
								$this->getUrl("end"));
						break;
				}
				break;

			case "error":
				$this->redirectTo("application.to_page");
				break;
			case "updateRec":
				switch($this->step)
				{
					case "confirm":
						// step2: display record + confirm button
						if ($this->checkPreviousStep("[][modify]","console",""))
							$this->editRec->runConfirmInputRecord("updateRec");
						break;
					case "modify":
						// step 3: display/modify record preloaded from session		
						if ($this->checkPreviousStep("confirm","console",""))
							$this->editRec->runEditRecord("updateRec", "session",
									$this->getUrl("","console"), 
									$this->getUrl("confirm"));
						break;
					case "end":
						// step 4: store record in db
						if ($this->checkPreviousStep("[][start]","error",""))
						{
							if ($this->editRec->runSaveRecord("updateRec",false,null,"post","record_edit",false))
							{
								$this->finish();
								$this->redirectTo("application.to_page");
							}
							else
								$this->finish();
							return;
						}
						else
						{
							$this->finish();
						}
						break;
					case null:
					default:
						// step1: display empty record
						$this->startOp();
						$this->editRec->runEditRecord("updateRec", "db",
								$cancelUrl,
								$this->getUrl("end"));
						break;
				}
				break;

			// delete a single record
			case "deleteRec":
				switch($this->step)
				{
					case "end":
						if ($this->checkPreviousStep("","console",null))
						{
							// step 4: store record in db
							if($this->editRec->runDeleteRecord("deleteRec"));
							{
								$this->finish();
								$this->redirectTo("application.to_page");
							}
							$this->finish();
							return;
						}
						break;
					case null:
					default:
						// step1: display empty record
						$this->startOp();
						$this->editRec->runConfirmDeletion("deleteRec");
						break;
				}
				break;

			// display a single record
			case "dispItem":
				$this->startOp();
				$this->dispRec->runDisplayQuery(
					"dispRecord",
					"toHTML",
					$this->getUrl("","dispItem"),
					$this->getUrl("","list")
					);
				$this->finish();
				break;

			case "list":
				$this->startOp();
				$this->setProperty("viewRecord.url",$this->getUrl("","dispItem"));
				$this->dispRec->runDisplayQuery(
					"listRecords",
					"toHTML",
					$this->getUrl("","dispItem"),
					$this->getUrl("","list")
					);
				$this->finish();
			break;

			case "console":
			default:			
				$this->startOp();
				$this->dispRec->runDisplayQuery(
					"console",
					"toHTML",
					$this->getUrl("","updateRec")."&key=",
					$this->getUrl("","console")
					);
				$this->finish();
				break;
		}

		// store current state in session
		$this->saveState();
	}
}

?>

<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxApplication.inc');

class table_display extends NxApplication
{
	function table_display($props,&$parentDesc)
	{
		$this->NxApplication('table.display',$props,$parentDesc);
	}

	/* @desc create a state machine with one operation (publish) and 2 steps (invoice and process).
		operation publish: publish the micro site in two steps:
			- invoice: compute and displays an invoice with all pages to publish (ie that have been modified)
			- process: publish all modified pages and sends an email to the client with invoice.
	*/
	function run()
	{
		$this->setProperty("console.url",$this->getUrl("","console"));
		$this->setProperty("newRecord.url",$this->getUrl("","newRec"));
		$this->setProperty("viewRecord.url",$this->getUrl("","dispItem"));
		$this->setProperty("editRecord.url",$this->getUrl("","updateRec"));
		$this->setProperty("deleteRecord.url",$this->getUrl("","deleteRec"));
		
		switch($this->operation)
		{	
			case "newRec":
			switch($this->step)
			{
				case "process":
					// step 4: store record in db
					if ($this->checkPreviousStep("[][input]","display",""))
					{
						if ($this->runLogic('table.newRec.process'))
						{
							$this->redirectTo('page.to_page');
						}
						$this->finish();
						return;
					}
					break;
				// case "input":
				default:
					// step1: display empty record for input
					$this->step='input';			
					$this->startOp();
					$this->runLogic('table.newRec.input',array('nextStep'=>$this->getUrl('process')));
					break;
			}
			break;
			
			case "updateRec":
				switch($this->step)
				{						
					// end when no confirmation is required
					case 'process':
						// step 4: store record in db
						if ($this->checkPreviousStep("input","display",""))
						{
							if ($this->runLogic('table.updateRec.process'))
								$this->redirectTo('page.from_page');
							$this->finish();							
							return;
						}
						break;
					default:
						// step1: display empty record
						$this->step='input';
						$this->startOp();
						$this->runLogic('table.updateRec.input',
							array('nextStep'=>$this->getUrl('process')));
						break;
				}
				break;
			/*
			case "editRec":
				$this->editRec = & $this->getPlugIn("DBEdit");
				switch($this->step)
				{
					case "confirm":
						// step2: display record + confirm button
						if ($this->checkPreviousStep("[input][modify]","display",""))
							$this->runLogic('table.updateRec.confirm',
									array(	'backStep'=>$this->getUrl('modify'),
											'nextStep'=>$this->getUrl('process')));
						break;
					case "modify":
						// step 3: display/modify record preloaded from session		
						if ($this->checkPreviousStep("confirm","display",""))
							$this->runLogic('table.updateRec.modify',
									array(	'backStep'=>$this->fromPage,
											'nextStep'=>$this->getUrl('confirm')));																break;
					// end when confirmation is required
					case 'process':
						// step 4: store record in db
						if ($this->checkPreviousStep('[confirm][input]','display',''))
						{
							if ($this->runLogic('table.updateRec.process'))
							{
								$this->redirectTo('page.to_page');
							}
							$this->finish();
							return;
						}
						break;
					// starting point when confirmation is required
					default:
						// step1: display empty record
						$this->step='input';
						$this->startOp();
						$this->runLogic('table.updateRec.input',
							array('nextStep'=>$this->getUrl('process')));
						break;
				}
				break;
				*/			
				
			// operation: deleteRec.
			// delete a single record
			case 'deleteRec':
				switch($this->step)
				{
					case 'process':
						// step2: process deletion
						if ($this->checkPreviousStep('confirm','display',null,'page.message.view'))
						{
							// step 4: store record in db
							$this->runLogic('table.deleteRec.process');
							$this->finish();
							return true;
						}
						break;
					default:
						// step1: display record to delete with confirmation button
						$this->step='confirm';			
						$this->startOp();
						$this->runLogic('table.deleteRec.confirm',
										array('nextStep'=>$this->getUrl('process')));
						break;
				}
				break;
			
			// case "console":
			default:
				$this->operation = 'display';
				$this->step='';			
				if (!$this->runLogic('table.display.process', 
						array(
							'fmethod' => 'toHTML',
							'recordURL'=> $this->getUrl('','updateRec').'&key=',
							'listURL'  => $this->getUrl(),					
							'backStep' => $this->fromPage,
							)))
					$this->finish();
					return true;
				break;
		}
		
		$this->saveState();
		return true;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxApplication.inc');

class table_console extends NxApplication
{
	function table_console($props,&$parentDesc)
	{
		$this->NxApplication('table.console',$props,$parentDesc);
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
		
		// operation: deleteRec.
		// delete a single record
		$this->operation='deleteRec';
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
		
		$this->saveState();
		return true;
	}
}

?>
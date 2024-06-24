<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxApplication.inc');

class user_password_forgotten extends NxApplication
{
	function user_password_forgotten($props,&$parentDesc)
	{
		$this->NxApplication('user.password.forgotten',$props,$parentDesc);
	}

	/* @desc create a state machine with one operation (publish) and 2 steps (invoice and process).
		operation publish: publish the micro site in two steps:
			- invoice: compute and displays an invoice with all pages to publish (ie that have been modified)
			- process: publish all modified pages and sends an email to the client with invoice.
	*/
	function run()
	{
		// operation: deleteRec.
		// delete a single record
		$this->operation='forgotten';
		switch($this->step)
		{
			case 'process':
				// step2: process deletion
				// if ($this->checkPreviousStep('[start][process]','','','page.message'))
				{
					// step 4: store record in db
					$this->runLogic('user.password.forgotten.process',array('backStep'=>$this->getUrl('start')));
					$this->finish();
					return true;
				}
				break;
			default:
				// step1: display record to delete with confirmation button
				$this->step='start';			
				$this->startOp();
				$this->runLogic('user.password.forgotten.start',
								array('nextStep'=>$this->getUrl('process')));
				break;
		}
		
		$this->saveState();
		return true;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_updateRec_confirm extends NxPage
{
	function table_updateRec_confirm($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		// get record from HTTP data source
		$records = $this->getRecords($this->view,'records','HTTP_POST');
		
		// check fields
		if ($this->error->isError())
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
			$this->runView('page.view', array(
				// "records" => $records,
				"fmethod" => "toForm",
				'submit.url' => $this->thisUrl,
				'cancel.url' => $this->backUrl,	
				"msg" 	=> $errorCB->getErrorMsg()
			));
		}
		else
		{
			// ok: store record in session (for update if requested),
			// display record and propose "back" and "confirm" buttons
			
			// store record in session
			$this->putRecords($records,$this->view,'insert','session');
	
			// display record for validation
			$this->runView('page.view', array(
				// "records" => $records,
				"fmethod" => "toHTML",
				'submit.url' => $this->nextUrl,
				'cancel.url' => $this->backUrl,			
				"msg" 	=> $this->getString($props->getProperty($this->view.".msg","confirm data",true))
			));		
		}
		
		return true;
	}
}
?>
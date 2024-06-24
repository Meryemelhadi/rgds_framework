<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_updateRec_process extends NxPage
{
	function table_updateRec_process($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}

	// process record deletion
	function run()
	{
		// get record from HTTP data source
		$records = $this->getRecords($this->view,'records','session');
				
		if ($this->error->isError() == false)
			$this->putRecords($records,$this->view,'insert','DB');
				
		// check result
		if ($this->error->isError())
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
			$this->runView('page.view',
				array(
				// "records" => $records,
				"fmethod" => "toForm",
				'submit.url' => $this->thisUrl,
				'cancel.url' => $this->backUrl,			
				'msg' 	=> $this->getString($this->getProperty($view.".msg","edit record",true))
			));
			
			return false;
		}		

		return true;
	}
}
?>
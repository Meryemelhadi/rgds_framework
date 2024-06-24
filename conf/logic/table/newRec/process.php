<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_newRec_process extends NxPage
{
	function table_newRec_process($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}

	// process record deletion
	function run()
	{
		// get record from HTTP data source
		$records = $this->getRecords($this->view,'records','http_post');

		// insert new record in database
		if ($this->error->isError() == false)
			$this->putRecords($records,$this->view,'insert','DB');

		// check result
		if ($this->error->isError())
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
			$this->runView('page.message',
				array(
				"fmethod" => "toHTML",
				'button.url' => $this->backUrl,
				'button.redirect' => 'back',
				'msg' 	=> $this->error->getErrorMsg()));
				
			// cleanup record resources (files)
			$records->onDelete($this->error);
			
			return false;
		}		

		return true;
	}
}
?>
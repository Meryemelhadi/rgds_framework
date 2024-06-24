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
		$records = $this->getRecords($this->view,'records','http_post');

		// update record in database
		if ($this->error->isError() == false)
			$this->putRecords($records,$this->view,'update','DB');
				
		// display an error message if pb.
		if ($this->error->isError())
		{
			// record with an error message
			$this->runView('page.message.view',array(
				'msg' => $this->error->getErrorMsg(),
				"button.redirect" => "back",
				"button.url" => $this->backUrl
				)
			);
			
			return false;
		}		

		return true;
	}
}
?>
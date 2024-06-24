<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_updateRec_input extends NxPage
{
	function table_updateRec_input($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		// load record from DB
		$this->getRecords($this->view,'records','DB');
		
		if (!$this->error->isError())
		{
			// ok: display record
			$this->runView('page.view',
				array(
				"fmethod" => "toForm",
				// "records" => $records,
				'submit.url' => $this->nextUrl,
				'cancel.url' => $this->backUrl,	
				'msg' 	=> $this->getString($this->getProperty($this->view.".msg","edit record",true))
			));
		}
		else
		{
			// record with an error message
			$this->runView('page.message.view',array(
				'msg' => $this->error->getErrorMsg(),
				"button.redirect" => "back",
				"button.url" => $this->backUrl
				)
			);
		}
				
		return true;
	}
}
?>
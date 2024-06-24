<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_newRec_input extends NxPage
{
	function table_newRec_input($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	/** 
	  * retreive one/a set of records from a datasource and display it/them.
	  * In case of error, display a message through the "message" view.
	  *
	  * input property set:
			'fmethod' => 'toHTML',
			'recordURL'=> $this->getUrl('','updateRec').'&key=',
			'listURL'  => $this->getUrl(),					
			'backStep' => $this->fromPage,
	  *
	  * @return operation success
	  */
	function run()
	{
		// get empty record and store it in records property
		$this->getRecords($this->view,'records','no_data');

		if (!$this->error->isError())
		{
			$this->runView('page.view',
				array(
				"fmethod" => "toForm",
				'submit.url' => $this->nextUrl,
				'cancel.url' => $this->backUrl,			
				'msg' 	=> $this->getString($this->getProperty($this->view.".msg","edit record",true))
			));
		}
		else
		{
			// record with an error message
			$this->runView('page.message',array(
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
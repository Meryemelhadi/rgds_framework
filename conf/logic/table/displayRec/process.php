<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_displayRec_process extends NxPage
{
	function table_displayRec_process($props,&$parentDesc)
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
		// get record from DB data source
		$records = $this->getRecords($this->view,'records','DB');

		if (! $this->error->isError())
		{
			// display table
			$viewProps = array(
				"fmethod" => "toHTML",
				'back.url' => $this->nextUrl,
				'msg' 	=> $this->getString($this->getProperty($this->view.".msg","display record",true))
			);
					
			$this->runView('page.view',$viewProps);
			return true;
		}
		else
		{
			// database error
			$this->runView('page.message.view', array(
				"msg" => "Error can't display ".$view." : " . $errorCB->getErrorMsg(), 
				"button.redirect" => "back",
				"button.url" => $this->backUrl
				)
			);
			return false;
		}
	}
}
?>
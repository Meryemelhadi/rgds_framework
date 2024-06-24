<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_deleteRec_confirm extends NxPage
{
	function table_deleteRec_confirm($props,&$parentDesc)
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

		// check if record still exist (could have been erased in the meantime)
		if ($this->error->isError() || $records->count() < 1)
			$this->error->addError($this->getString('Page expired'), 'Cannot delete this record, no longer exist','warning');
			
		// case 1:  ok
		if (!$this->error->isError())
		{
			// display record and propose "back" and "confirm" buttons
			//$this->setProperty($view.".submit.url",$this->getProperty('nextStep'));			
	
			// display record + message
			$this->runView('page.view',
				array(
					'records' => $records,
					'fmethod' => 'toHTML',
					'submit.url' => $this->nextUrl,
					'cancel.url' => $this->backUrl,
					'msg' 	=> $this->getString($this->getProperty($view.'.msg','confirm deletion',true))
				)
			);		
		}
		// case 2:  DB error
		else
		{
			$this->runView('page.message', array(
				'msg' => "Error can't delete record :".$this->error->getErrorMsg(), 
				'button.redirect' => "back",
				'button.url' => $this->backUrl
				)
			);
			return false;
		}
		
		return true;
	}
}
?>
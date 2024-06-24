<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class table_deleteRec_process extends NxPage
{
	function table_deleteRec_process($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}

	// process record deletion
	function run()
	{
		// get record from DB data source
		$records = $this->getRecords($this->view,null,'DB');
			
		if ($records->count() < 1 || $this->error->isError())
		{
			$this->error->addError($this->getString('record not found'), 
						$this->error->getErrorMsg() ,'error');
		}
		else
		{
			// delete record from db
			$this->deleteRecords($this->view, $records,'DB', true/*delete resources*/);
		}

		// check result
		
		// error : display msg
		if ($this->error->isError())
		{
			$this->runView('page.message',array(
					'msg' => "Database error: ". $this->error->getErrorMsg(), 
					'button.redirect' => 'back',
					'button.url' => $this->backUrl));
			return false;			
		}
		// ok : case 1, with message display
		else if ($this->getProperty($this->view.'.isOkMsg',true)==true)
		{
			$this->runView('page.view',array(
					'msg' => $this->getString('Record deleted'), 
					'button.redirect' => 'ok',
					'button.url' => $this->nextUrl));

			return false; // return false to avoid redirection by application
		}
		/*
		// ok : case 2, no display => redirection to other page
		else 
		{
			$this->redirectTo('nextStep');
		}
		*/
		return true;
		
	}
}
?>
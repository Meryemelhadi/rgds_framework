<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class content_data extends NxPage
{
	function content_data($props,&$parentDesc)
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
		$records = $this->getRecords($this->view,'records');

		if (! $this->error->isError())
		{
			// display table
			$viewProps = array(
				// 'records' => $records,
				'back.url' => $this->backUrl,
				'button.redirect' => 'back'
			);
			
			if ($records->count() > 0)
				$viewProps['isEmpty'] = 'false';
			else 
			{
				$viewProps['isEmpty'] = 'true';
				$viewProps['msg'] = $this->getString('Empty table');
			}
			
			$this->runView('page.view',$viewProps);
			return true;
		}
		else
		{
			// database error
			$this->runView('page.message', array(
				"msg" => "Error can't display ".$this->view." : " . $this->error->getErrorMsg(), 
				"button.redirect" => "back",
				"button.url" => $this->backUrl
				)
			);
			return false;
		}
	}
}
?>
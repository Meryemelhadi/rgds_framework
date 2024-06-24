<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// manage search results
class NxPlug_Search
{
	var $props;
	
	function  NxPlug_Search(&$props)
	{
		$this->props = & $props;
	}
	
	function runSearchForm($view,$searchResURL=null, $slotKey="record_search")
	{
		$props = & $this->props;
		$data = & $props->getPlugin("Data");
		$ui = & $this->props->getPlugin("HtmlUI");

		// get dataset description
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		$errorCB = new NxErrorCB();
		$records = $data->getEmptyRecord($recDesc,$errorCB);

		if($searchResURL===null)
		{
			$searchResURL = $props->getProperty($view.".resultPage");
		}
		
		// set button urls
		$props->setProperty($view.".submit.url",$searchResURL);
		$props->setProperty($view.".form.method","GET");
		
		// display record to edit
		$viewProps = array(
			"records" => $records,
			"fmethod" => "toForm",
			"formHeader" => $ui->getFormHeader($recDesc, $searchResURL,null,null,"GET"),
			"formFooter" => "</form>",
			"submitButton" => $ui->getSubmitButton($view.".submit"),
			'submit.url' => $searchResURL
		);

		$props->runView($view,$viewProps);
		return true;
	}	

	// display results and propose new search
	function runSearchResults($view)
	{
		$props = & $this->props;
		$data = & $props->getPlugin("Data");
		$db = & $this->props->getPlugin("DB");

		// get dataset description
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		// get record desc
		$errorCB = new NxErrorCB();
		$records =& $db->getDBRecords($recDesc,$errorCB,null,null);
					
		// check fields
		if (!$errorCB->isError())
		{
			// display record to edit
			$viewProps = array(
				"records" => $records,
				"fmethod" => "toHTML"
			);
	
			// display record + message
			$props->runView("view.".$view,$viewProps);		
		}
		else
		{
			// error: coming from a direct URL (bookmark or "previous/forward" button)
			$errorDesc = array("msg" => "Error: ".$errorCB->getErrorMsg(),
								"button.redirect" => $view.".ok");	
			$props->runView("view.message",$errorDesc);
		}
			
		return true;
	}
}
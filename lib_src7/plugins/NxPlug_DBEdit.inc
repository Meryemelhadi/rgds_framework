<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_DBEdit
{
	var $app;
	
	function  NxPlug_DBEdit(&$app)
	{
		$this->app = & $app;
		$this->db = & $app->getPlugin("DB");
	}
	
	/** 
	  * retreive one/a set of records and display it/them.
	  * In case of error, display a message through the "message" view.
	  *
	  * @parameter dbViewName database set of fields to display
	  * @parameter htmlViewName html view name for displaying records.
	  * @parameter method where
	  *            (toHTML, toForm, toDB, toString or toObject)
	  * @parameter recordURL base url for the display of a single record
	  * @parameter listURL url for the display of the lsit of records
	  *
	  * @return operation success
	  */
	function runEditRecord(	$view, $media, $cancelUrl, $nextStepUrl, 
							$errorMsg=null,$records=null, $slotKey="record_edit")
	{
		$app   = & $this->app;
		$props = & $this->app;
		$data = & $app->getPlugin("Data");
		$ui = & $app->getPlugin("HtmlUI");

		// get dataset description
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		// get records from media
		if ($records == null)
		{
			$errorCB = new NxErrorCB();
			switch (strtolower($media))
			{
				case "post":
					$records = new NxData_HttpForm("POST",$recDesc, $errorCB);
					break;
				case "db": //update
					$records = $this->db->getDBRecords($recDesc,$errorCB,null,null);
					break;
				case "session":
					require_once(NX_LIB.'NxData_Session.inc');
					$records = new NxData_SessionInput($slotKey, $recDesc, $errorCB);	
					break;
				case "empty":
				default:
					$records = $data->getEmptyRecord($recDesc,$errorCB);
					break;
			}
		}

		if ($errorMsg == null)
		{
			// set button urls
			$props->setProperty($view.".submit.url",$nextStepUrl);
			$props->setProperty($view.".cancel.url",$cancelUrl);
			
			// display record to edit
			$viewProps = array(
				"records" => $records,
				"fmethod" => "toForm",
				"formHeader" => $ui->getFormHeader($recDesc, $nextStepUrl),
				"formFooter" => "</form>",
				"submitButton" => $ui->getSubmitButton($view.".submit"),
				"cancelButton" => $ui->getButton($view.".cancel",$cancelUrl),
				"msg" 	=> $props->getString($props->getProperty($view.".msg","edit record",true)),
				"submit.url" =>$nextStepUrl
			);
	
			// $app->runView('view.'.$view,$viewProps);
			$app->runView('view.'.$view,$viewProps);
		}
		else
		{
			// error
			$viewProps = array(
				'msg' 	=> $errorMsg,
				'button.redirect'=>'back'
				);

			// record with an error message
			$app->runView('view.message',$viewProps);
		}
				
		return true;
	}	
	
	function runConfirmInputRecord($view, $slotKey="record_edit")
	{
		$app   = & $this->app;
		$props = & $app;
		$db    = & $this->db;
		$data  = & $app->data;
		$ui    = & $app->ui;
		$session = & $app->session;
		
		// get record desc
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		// get record HTTP
		$errorCB = new NxErrorCB();
		require_once(NX_LIB."NxData_HttpForm.inc");
		$records = new NxData_HttpForm("POST",$recDesc, $errorCB);
		
		// check fields
		if ($errorCB->isError())
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
			$this->runEditRecord($view,null,
								 $app->getUrl("","console"), 
								 $app->getUrl("confirm"), 
								 $errorCB->getErrorMsg(),
								 $records);
		}
		else
		{
			// ok: store record in session (for update if requested),
			// display record and propose "back" and "confirm" buttons
			
			// store record in session
			$record = $records->getRecord();
			$session->storeRecord($record,$slotKey);

			// display record to edit
			$viewProps = array(
				"records" => $records,
				"fmethod" => "toHTML",
				"submitButton" => $ui->getButton($view.".confirm",$app->getUrl("end")),
				"cancelButton" => $ui->getButton($view.".modify",$app->getUrl("modify")),
				"msg" 	=> $props->getString($props->getProperty($view.".msg","confirm data",true))
			);
	
			// simple record
			$app->runView('view.'.$view,$viewProps);		
		}
		
		return true;
	}
	
	function runSaveRecord($view, $isNew=true, $records=null,$media="session", $slotKey="record_edit",$dispMsg=true)
	{	
		$app   = & $this->app;
		$props = & $app;
		$db    = & $this->db;
		$data  = & $app->data;
		$ui    = & $app->ui;
		$session = & $app->session;

		
		// get record from data source
		$errorCB = new NxErrorCB();

		if ($records == null)
		{
			$input = $this->app->getPlugIn("RecordInput");
			$records =& $input->getRecords($view, $media, $errorCB, $slotKey);
		}
			
		if ($errorCB->isError() == false)
		{
			// store record in database		
			if ($isNew)
			{
				$dbOp = "insert";
				$msg =  $props->getString("Record added successfully");
			}
			else
			{
				$dbOp = "update";
				$msg =  $props->getString("Record updated successfully");
			}
			
			$recDesc =& $records->getRecDesc();
			$db->addDBRecords($recDesc, $errorCB, $records,$dbOp);
		}
				
		// check result
		if ($errorCB->isError())
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
//			$this->runEditRecord('view.'.$view,null,
			$this->runEditRecord($view,null,
								 $app->getUrl("","console"), 
								 $app->getUrl("end"), 
								 $errorCB->getErrorMsg(),
								 $records);
		}
		else if ($dispMsg)
		{
			$msg = $props->getString($props->getProperty($view.".msg","record saved",true));
			$MessageProps = array(
					"msg" => $msg, 
					"button.redirect" => $view.".ok",
				);				
			$app->runView('view.message',$MessageProps);
		}

		return true;
	}

	function runConfirmDeletion($view)
	{
		$app   = & $this->app;
		$props = & $app;
		$db    = & $this->db;
		$data  = & $app->data;
		$ui    = & $app->ui;
		$session = & $app->session;
		
		// get record desc
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		// get record desc
		$errorCB = new NxErrorCB();
		$records = $db->getDBRecords($recDesc,$errorCB,null,null);
		
		if ($records->count() < 1)
			$errorCB->addError($props->getString("Page expired"), "Cannot delete this record, no longer exist","warning");
		// check fields
		if (!$errorCB->isError())
		{
			// display record and propose "back" and "confirm" buttons
			$props->setProperty($view.".submit.url",$app->getUrl("end"));			
			$params=$props->getProperty($view.".submit.params",null);

			// display record to edit
			$viewProps = array(
				"records" => $records,
				"fmethod" => "toHTML",
				"submitButton" => $ui->getButton($view.".submit",null,$params),
				"cancelButton" => $ui->getButton($view.".cancel"),
				"msg" 	=> $props->getString($props->getProperty($view.".msg","confirm deletion",true))
			);
	
			// display record + message
			$app->runView('view.'.$view,$viewProps);		
		}
		else
		{
			// error: coming from a direct URL (bookmark or "previous/forward" button)
			$errorDesc = array("msg" => "Error: ".$errorCB->getErrorMsg(),
								"button.redirect" => $view.".ok");	
			$app->runView('view.message',$errorDesc);
		}
		
		return true;
	}
	
	function runDeleteRecord($view,$isMsg=true)
	{	
		$app   = & $this->app;
		$props = & $app;
		$db    = & $this->db;
		$data  = & $app->data;
		$ui    = & $app->ui;
		$session = & $app->session;

		// get record desc
		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return false;
		
		// get record desc
		$errorCB = new NxErrorCB();
		$records = $db->getDBRecords($recDesc,$errorCB,null,null);
		if ($errorCB->isError() || $records->count() < 1)
		{
			$errorCB->addError($props->getString("record not found"), $errorCB->getErrorMsg() ,"error");
		}
		else
		{
			// delete record from db		
			$dbOp = "delete";
			$errorCB = new NxErrorCB();
			$db->executeDBQuery($view, "deleteRecord", $errorCB);

			// delete resources (files etc.)
			for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
			{
				$record = & $recIt->getCurrent();
				$record->onDelete($errorCB);
			}
		}

		// check result
		if ($errorCB->isError())
		{
			$MessageProps = array(
					"msg" => "Database error: ". $errorCB->getErrorMsg(), 
					"button.redirect" => $view.".ok",
				);				
			$app->runView('view.message',$MessageProps);
			return false;			
		}
		else if ($isMsg)
		{
			$MessageProps = array(
					"msg" => $props->getString("Record deleted"), 
					"button.redirect" => $view.".ok",
				);				
			$app->runView('view.message',$MessageProps);
		}
		return true;
			
	}
}
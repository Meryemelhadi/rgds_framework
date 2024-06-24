<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'module/NxApplication.inc');

/** Application: List records from a database table
  * properties:
  * $appProps = array(
  *		"table" => "News",
  *		"nbRecordPerPage" => 15);
  */
class NxApp_updatePassword extends NxApplication
{
	function NxApp_updatePassword($props,&$parentDesc)
	{
		$this->NxApplication("NxApp_updatePassword",$props,$parentDesc);
	}
	
	function requestPassword($view,$errorMsg=null)
	{
		$dbEdit = $this->getPlugIn("DBEdit");
		
		if ($errorMsg!== null)
			$this->setProperty($view.".msg",$errorMsg);
			
		$dbEdit->runEditRecord($view, "empty",
							$this->getProperty("page.from_page"), 
							$this->getUrl("validate"),
							null);
	}
	
	function validatePassword($view="validate")
	{
		$recInput = $this->getPlugIn("RecordInput");

		// get form content
		$errorCB = new NxErrorCB();
		$records = & $recInput->getRecords("validate", "POST", $errorCB);
		if ($errorCB->isError())
		{
			return $this->requestPassword("entryError",$errorCB->getErrorMsg());
		}
		
		// check old password
		$record =& $records->getRecord();
		$old = $record->getProperty("password.current",null,false,'String');
		$new_1 = $record->getProperty("password",null,false,'String');
		$new_2 = $record->getProperty("password.new2",null,false,'String');
		if ($old === null)
		{
			$msg = $this->getString("check current password");
		}
		else if ($new_1 === null || $new_2 === null || $new_1 !== $new_2)
		{
			$msg = $this->getString("check new password");
		}
		else
		{
			// get current password
			$records = & $recInput->getRecords("read", "db", $errorCB);
			if ($records !== null && $records->count() > 0)
			{
				$record =& $records->getCurrent();
				$curr_pass = $record->getProperty("password",null,false,'String');
				
				// check entered password
				if ($old !== $curr_pass)
					$msg = $this->getString("check current password");
				else
				{
					// ok, just need to save it now
					
					// read record from POST with right DB format (including key)
					$records = & $recInput->getRecords("read", "POST", $errorCB);
	
					// save it to db
					$dbEdit = $this->getPlugIn("DBEdit");
					return $dbEdit->runSaveRecord("save", false, $records, null, null ,true);
				}
			}
			else
			{
				$msg =& $errorCB->getErrorMsg();
			}
		}
		
		// error => request password again
		$this->requestPassword("entryError",$msg);
		return false;
	}
	
	function run()
	{		
		switch($this->step)
		{	
			// display a single record
			case "validate":
				if ($this->validatePassword("validate") === true)
				{
					// $this->redirectTo("application.to_page");
					$this->finish();
					return;
				}
				
				break;

			case "request":
			default:
				$this->requestPassword("request");		
				break;
		}

		// store current state in session
		$this->saveState();
	}
}

?>

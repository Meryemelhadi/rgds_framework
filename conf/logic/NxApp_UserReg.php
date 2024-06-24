<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 * NB. one of the first Nx Application. Should be rewritten and split up in dynamic steps.
 */

 require_once(NX_LIB."NxData_HttpForm.inc");
require_once(NX_LIB.'module/NxApplication.inc');

class NxApp_UserReg extends NxApplication
{
	function NxApp_UserReg($props,&$parentDesc)
	{
		$this->NxApplication("NxApp_UserReg",$props,$parentDesc);
		$this->ui 	= $this->getPlugIn('HtmlUI');
		$this->data = $this->getPlugIn('Data');
		$this->session = $this->getPlugIn('Session');

		$this->dispRec = $this->getPlugIn('DBDisplay');
		$this->editRec = $this->getPlugIn('DBEdit');
	}
	
	function requestDetails()
	{
		return $this->editRec->runEditRecord("register.".$this->operation.".start", "empty",
					$this->getProperty("application.from_page","/"),
					$this->getUrl("confirm"));
	}

	function modifyDetails()
	{
		return $this->editRec->runEditRecord("register.".$this->operation.".modify", "session",
								$this->getProperty("application.from_page","/"), 
								$this->getUrl("confirm"));
	}
	
	function checkNewLoginId($view, & $records, &$errorCB)
	{
		// get user id from record
		$record = $records->getRecord();
		$userID = $record->getProperty('user.userID');
		
		$userID=preg_replace(array('/[ \t]+/','/[^a-zA-Z-_0-9]/'),array('_',''),strtolower(trim($userID)));
		
		// get all records that match this id
		$db = $this->getPlugIn("DB");
		$db->setRootProperty("user.exist.userID",$userID); // property as a query parameter
		$existIDRecs = $db->getRecords($view.'.checkid',$errorCB);
		if ($existIDRecs && $existIDRecs->count() == 0)
		{
			// if none, ok
			return true;
		}
		
		// else try to add country to the id and check if ID exists
		$country = $record->getProperty("user.country",null,true,"Store");
		if ($country != null)
		{
			// extract code and remove leading marker
			$country = strtolower(ltrim($country,'?'));
			
			$idCandidate=$userID."_".$country;
			$db->setRootProperty("user.exist.userID",$idCandidate); // property as a query parameter
			$existIDRecs = $db->getRecords($view.'.checkid', $errorCB);
			if ($existIDRecs->count() == 0)
			{
				// if none, add error: login ID already exists, please try another one (suggestion: countryid), return error
				$msg=$this->getString("User id already taken")." $userID ".
						sprintf($this->getString("Suggested id: %s"),$idCandidate);
				$errorCB->addError($msg);
				return false;
			}
		}
		
		// no suggestion, add error login ID already exists, please try another one.
		$msg=$this->getString("User id already taken");
		$errorCB->addError($msg);
		return false;
	}

	function runConfirmInputRecord($view)
	{
		$app   = & $this;
		$props = & $app;
		$db    = & $app->db;
		$data  = & $app->data;
		$ui    = & $app->ui;
		$session = & $app->session;
				
		// get record desc
		$recDesc =& $data->getRecDesc($view,true);
		
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
			$record = $records->getRecord();
			$session->storeRecord($record,"record_edit");
			$props->setProperty('back.url',$app->getUrl("modify"));
			$this->editRec->runEditRecord($view,null,
								 $this->getProperty("application.from_page","/"), 
								 $app->getUrl("confirm"), 
								 $errorCB->getErrorMsg(),
								 $records);
			return true;
		}

		// check if entered id already exists in the database
		if ($this->checkNewLoginId($view,$records,$errorCB) == false)
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
			$this->editRec->runEditRecord($view,null,
								 $this->getProperty("application.from_page","/"), 
								 $app->getUrl("confirm"), 
								 $errorCB->getErrorMsg(),
								 $records);
			return true;
		}
		
		// ok: store record in session (for update if requested),
		// display record and propose "back" and "confirm" buttons
		
		// store record in session
		$record = $records->getRecord();
		$record->setField('user.userID',$props->getProperty('user.exist.userID'));
		$session->storeRecord($record,"record_edit");
		
		// create records set from modified record
		include_once(NX_LIB.'NxData_ArrayRecords.inc');
		$records2=new NxData_ArrayRecords();
		$records2->addRecord($record);

		// display record to edit
		$viewProps = array(
			"records" => $records2,
			"fmethod" => "toHTML",
			// new version
			'submit.url' => $app->getUrl("send"), 
			'cancel.url' => $app->getUrl("modify"),
			// deprecated
			"submitButton" => $ui->getButton($view.".confirm",$app->getUrl("send")),
			"cancelButton" => $ui->getButton($view.".modify",$app->getUrl("modify")),
			"msg" 	=> $props->getString($props->getProperty($view.".confirm.msg","confirm data",true))
		);

		// simple record
		$app->runView('view.'.$view,$viewProps);		
		
		return true;
	}

	function requestConfirm()
	{
		
		return $this->runConfirmInputRecord("register.".$this->operation.".confirm");
	}
	
	function send()
	{
		// load email and password plugins
		$this->pwd 	 = $this->getPlugIn('Password');
		$this->email = $this->getPlugIn('Email');
		
		// get record descriptor for system fields and password
		$viewUpdate = "register.".$this->operation.".complete";
		$recDescUpdate = & $this->data->getRecDesc($viewUpdate,true);
		if (!$recDescUpdate)
			return false;
		
		// get password length from the data description
		$fdesc = & $recDescUpdate->getFieldDescByAttribute("role","login.password");
		if ($fdesc == null)
		{
			// error: coming from a direct URL (bookmark or "previous/forward" button)
			$errorDesc = array(
				"msg" => "Error: dataset doesn't include required password field in ".$viewUpdate,
				"button"	=> "back"
			);
			
			$this->runView('view.message',$errorDesc);
			return false;
		}
		$passwdLen = $fdesc->getProperty("sizeGen",8,false);

		// create password ("easy to remember" type)
		global $passwd;
		$passwd = $this->pwd->createEasyPassword($passwdLen);
		
		// create record with automatically completed fields
		$errorCB = new NxErrorCB();
		$records_update = $this->data->getEmptyRecord($recDescUpdate, $errorCB);
		$record_update = $records_update->getRecord();
		
		// load record from session (automatic completion of additional fields such as passwd)
		$recDesc = & $this->data->getRecDesc("register.".$this->operation.".confirm",true);
		if (!$recDesc)
			return false;
			
		$records_edit = $this->session->loadRecords("record_edit", $recDesc, $errorCB);
		$record_edit = & $records_edit->getRecord();
		
		// update it
		$record_edit->updateWith($record_update);
		
		// save the newly updated record back in session (NB. updated by automatic completion)
		$this->session->storeRecord($record_edit,"record_edit");

		// set properties to be used in email template
		$userID = $record_edit->getProperty("user.userID");
		$this->setProperty('user',$userID);
		$this->setProperty('password',$passwd);
		
		// send an email to user with password and details
		if ($this->email->sendEmail("register.".$this->operation.".email",$records_edit,
				$this->getUrl("login"), 
				$this->getProperty("register.".$this->operation.".email.cancelURL"), // cancel
				$this->getUrl("tryagain")  // check email
				) !== false)
		{
			// display login form
			return $this->editRec->runEditRecord("register.".$this->operation.".login", "empty",
									$this->getProperty("application.from_page","/"), 
									$this->getUrl("validateLogin"));
		}
	}

	function validateLogin()
	{
		// get record descriptor
		$view = "register.".$this->operation.".validateLogin";
		$recDesc = & $this->data->getRecDesc($view,true);
		if (!$recDesc)
			return false;

		// get HTTP record
		$errorCB = new NxErrorCB();
		require_once(NX_LIB."NxData_HttpForm.inc");
		$records = new NxData_HttpForm("POST",$recDesc, $errorCB);

		// load stored record from session
		$recDescInput = & $this->data->getRecDesc("register.".$this->operation.".save",true);
		if (!$recDesc)
			return false;
			
		$errorCB = new NxErrorCB();
		$records_edit = $this->session->loadRecords("record_edit", $recDescInput, $errorCB);
		$record_edit = $records_edit->getRecord();		
		
		// check fields
		if ($errorCB->isError())
		{
			// error in record => need to reenter record value:
			// - display error
			// - display record to edit
			// - submit to current step
			$this->runEditRecord($view,null,
								 $this->getProperty("application.from_page","/"), 
								 $app->getUrl("confirm"), 
								 $errorCB->getErrorMsg(),
								 $records);
			return false;
		}

		$record_input = $records->getRecord();

		// get password from reference record and user input
		$fdescPasswd = & $recDesc->getFieldDescByAttribute("role","login.password");
		$passwdFname = $fdescPasswd->getName();
		$passwd = $record_edit->getField($passwdFname);
		$passwd_input = $record_input->getField($passwdFname);

		// get login ids from reference record and user input
		$fdescId = & $recDesc->getFieldDescByAttribute("role","login.id");
		$uidFname    = $fdescId->getName();
		$uid    = $record_edit->getField($uidFname);
		$uid_input    = $record_input->getField($uidFname);
		
		// check that password and ids are the same
		if (($passwd->toString() == $passwd_input->toString()) && ($uid->toString() == $uid_input->toString()))
		{
			// create user file space
			$us = $this->getPlugIn('UserSpace');
			$us->createUserSpace($uid->toString());
			
			// store user record in database
			$this->db = $this->getPlugIn('DB');
			$this->editRec->runSaveRecord("register.".$this->operation.".save", true,$records_edit);
			
			// auto login of the user
			global $_NX_site;
			$userPlug =& $_NX_site->getPlugIn("User");
			$userPlug->login(null,null,$uid->toString(),$passwd->toString());
			
			// run welcome view
			/*
			$props = array("records"=>$records_edit);
			$this->runView("register.".$this->operation.".success",$props);
			*/
			return true;
		}
		else 
		{
			// error => display login form
			$this->editRec->runEditRecord("register.".$this->operation.".errorlogin", "empty",
							$this->getProperty("application.from_page","/"), 
							$this->getUrl("validateLogin"));
			return false;
		}
	}
	
	function login()
	{
		return $this->editRec->runEditRecord("register.".$this->operation.".login", "empty",
						$this->getProperty("application.from_page","/"), 
						$this->getUrl("validateLogin"));
	}
	
	function run()
	{
		switch($this->operation)
		{	
			case "newUser":
			default:
				$this->operation = "newUser";
				switch($this->step)
				{
					case "confirm":
						if (!$this->requestConfirm())
							$this->finish();
						break;
						
					case "modify":
						// step 3: display/modify record preloaded from session		
						if (!$this->modifyDetails())
							$this->finish();
						break;
						
					case "send":
					case "tryagain":
						// step 4: store record in db
						if ($this->checkPreviousStep("[confirm][send][tryagain]"))
						{
							if ($this->isPageRefresh())
								$this->login();
							else 
								$this->send();
						}
						break;

					case "validateLogin":
						// step 4: store record in db
						if ($this->checkPreviousStep("[send][tryagain][validateLogin]"))
							if ($this->validateLogin())
							{
								//$this->redirectTo("application.to_page","/");
								$this->finish();
								return true;
							}
						break;
						
					case "start":
					default:
						if (!$this->requestDetails())
							$this->finish();
						break;
				}
			break;
		}
		
		$this->saveState();
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxLogin_FormAuth
{
	function __construct(&$props)
	{
		$this->props  = & $props;
		$this->props->data = $props->getPlugin("Data");
	}
	
	function & login()
	{
		// get record descriptor for login form
		$view = "account.login";

		trace("start","login");
		require_once(NX_LIB."NxData_Record.inc");
		$recDesc = & $this->props->data->getRecDesc($view,true);
		if (!$recDesc)
		{
			trace("can't get recdesc","login");
			return false;
		}

		// get HTTP record
		$errorCB = new NxErrorCB();

		require_once(NX_LIB."NxData_HttpForm.inc");
		$records = new NxData_HttpForm("POST",$recDesc, $errorCB);
		$record_input = $records->getRecord();

		// get login ids from user input
		$uid    = $record_input->getFieldByAttribute("role","login.id");
		trace("userID is [".$uid->toString()."]","login");
		
		// get password from user input
		$passwd = $record_input->getFieldByAttribute("role","login.password");
		trace("passwd is [".$passwd->toString()."]","login");

		if ($uid != null && $uid->toString()!="" && $passwd != null)
		{
			trace("returning user data as object","login");
			return new NxUser_Data($uid->toString(),$passwd->toString(),$this->props);
		}
		else
		{
			trace("userID or passwd missing","login");
			
			// no user or password, request it
			// no return from here
			$this->requestLogin();
		}
	}

	function requestLogin()
	{
		trace("requesting login","login");
		
		// display form and exit
		$props = & $this->props;

		// set properties for login form
		// $props->setProperty("view.loginForm","view.msg_record_edit");
		// $props->setProperty("db.loginForm","login");
		// $props->setProperty("loginForm.msg", "Please enter your company ID and password:");
		$props->setProperty("loginForm.submit.redirect","submit");
		$props->setProperty("loginForm.cancel.redirect" ,"cancel");
		
		// load required plugins
		$props->ui = $props->getPlugin("HtmlUI");
		$props->editRec = $props->getPlugin("DBEdit");

		// execute the display of record in edit mode
		$props->editRec->runEditRecord("loginForm", "empty", "/", "?");
		exit();
	}

	// display a login error and redirect to the right page as appropriate
	function loginError($error, $user)
	{
		trace("login error: $error","login");
		
		// display error
		$props = array( "msg"=>"Login error: ".$error." on protected page.", 
						"button.redirect" => "ok",
						"ok.url" => "/");
						
		$this->props->runView("page.loginError", $props);
		exit();
	}
}

?>
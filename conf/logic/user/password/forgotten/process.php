<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'module/NxPage.inc');

class user_password_forgotten_process extends NxPage
{
	function user_password_forgotten_process($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		// get record from database (NB. query based on user name + email)
		if (!($userRecord = $this->getRecord($this->view,'user.details','DB')))
		{
			// empty set
			$msg = $this->getString($this->getProperty($this->view.'.not_found.msg',''));
			
			$this->runView('page.error.view', array(
				"msg" => $msg, 
				"button.redirect" => "back",
				"button.url" => $this->backUrl
				)
			);
			return false;		
		}
		else if($this->error->isError()) 
		{
			// database error
			$this->runView('page.error.view', array(
				"msg" => "Error can't display ".$view." : " . $errorCB->getErrorMsg(), 
				"button.redirect" => "back",
				"button.url" => $this->previousUrl
				)
			);
			return false;
		}
		
		// extract user name and password from DB
		$this->addProperties($userRecord->getProperties());
		
		/*
		$userID   = $userRecord->getProperty('user.userID','unknown user');
		$password = $userRecord->getProperty('user.password','');
		$userEmail = $userRecord->getProperty('user.email','');
		
		// put user details in properties for use by views
		$this->setProperty('user.userID',$userID);
		$this->setProperty('user.password',$password);
		$this->setProperty('user.email',$userEmail);
		*/
		
		// get mail content: extract user data from record
		$mailRec = $this->getRecord($this->view.'.content',$this->view.'email','xdocs2');

		// adds current properties to current set
		$this->addProperties($mailRec->getProperties());
		
		// apply a view on html body
		// $this->runView('page.view',array('msg'=>$mailRec->getProperty('page.text','empty doc')));		
		
		// send message
		$this->putRecords($mailRec,$this->view.'.send',$this->error,'insert');
		
		// run view
		$this->runView('page.view',array('msg'=>$this->getString('password sent')));		
	}
}

?>
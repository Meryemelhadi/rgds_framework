<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'module/NxPage.inc');

class user_password_forgotten_start extends NxPage
{
	function user_password_forgotten_start($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		// get empty record (user name + email)
		$this->getRecords($this->view,'records','no_data');
		
		// get message
		$msgKey=$this->getProperty($this->view.'.msg','forgotten password prompt');
		$msg = $this->getString($msgKey);
		
		// run view
		$this->runView('page.view',array(
			'msg'=>$msg, 
			'fmethod'=>'toForm',
			'submit.url'=>$this->nextUrl));		
	}
}

?>
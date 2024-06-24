<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'module/NxPage.inc');

class user_signout extends NxPage
{
	function user_signout($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		// loggof							
		$userPlug =& $this->getPlugIn("User");
		$userPlug->logoff();

		// set message						
		$this->setProperty("msg",$this->getString("logout"));

		// run view
		$this->runView("page.view");		
	}
}

?>
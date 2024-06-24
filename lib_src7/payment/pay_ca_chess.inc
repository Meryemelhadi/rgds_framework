<?php
define('CUR_DOLLAR','DOLLARD');
define('CUR_EUROS','EUR');

class NxPG_CA_Chess 
{
	
	function NxPG_CA_Chess($urlBackOK="/nx/payment/onOK.php", $urlBackErr="/nx/payment/onErr.php")
	{
		$this->urlBackOK=$urlBackOK;
		$this->urlBackErr=$urlBackErr;
	}
	
	function doPayment($amount,$currency,$transID,$lang="en")
	{
		/* payment specific protocol :
			- call server
			- returns to one of the
		*/
	}
	
	function onOk($transID)
	{
		
	}
	
	function onFail($transID)
	{
	}
	
}

?>
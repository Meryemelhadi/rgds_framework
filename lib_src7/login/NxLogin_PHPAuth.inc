<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// Basic HTTP login interface

class NxLogin_PHPAuth
{
	function __construct(&$props)
	{
		$this->props  = & $props;
	}
	
	function & login()
	{
		// is it the first time a login is requested here for the session ?
		if (isset($_SESSION["_nxuser_requested_"]))
			// no, so check if we got something from the browser
			if(isset($_SERVER["PHP_AUTH_USER"]) && isset($_SERVER["PHP_AUTH_PW"]))
				return new NxUser_Data($_SERVER["PHP_AUTH_USER"],$_SERVER["PHP_AUTH_PW"],$this->props);
		
		// first time => request user/passwd (and tick a flag saying next time we can check the auth variables)
		$_SESSION["_nxuser_requested_"] = true;
		$this->requestLogin();
		return null;
		
		// no return from here
	}

	function requestLogin()
	{
		header( 'WWW-Authenticate: basic realm="Site login"' ); 
		header( 'HTTP/1.0 401 Unauthorized'); 
		$this->loginError("restricted page access", null);
		return null;
	}

	// display a login error and redirect to the right page as appropriate
	function loginError($error, $user)
	{
		print( "<html><body>Login error: ".$error." on protected page.</body></html>" ); 
		exit();
	}
}
?>

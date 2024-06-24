<?
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// php 3 compatibility layer

// server variables
global $HTTP_SESSION_VARS;
if (!isset($_SESSION))
	$_SESSION = & $HTTP_SESSION_VARS;

// http authentication
if (!isset($_SERVER["PHP_AUTH_USER"]))
{
	$_SERVER["PHP_AUTH_USER"] = & $PHP_AUTH_USER;
	$_SERVER["PHP_AUTH_PW"] = & $PHP_AUTH_PW;
}

?>
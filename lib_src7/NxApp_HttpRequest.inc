<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxApp_HttpRequest
{
	function getParam($name,$deft=null)
	{
		// get new state from HTTP params (GET or POST)
		if (isset($_GET[$name]))
		{
			return $_GET[$name];
		}
		else if (isset($_POST[$name]))
		{
			return $_POST[$name];
		}
		else if (isset($_COOKIE[$name]))
		{
			return $_COOKIE[$name];
		}
		else
		{
			return $deft;		
		}
	}

	function getParam_GET($name,$deft=null)
	{
		if (isset($_GET[$name]))
		{
			return $_GET[$name];
		}
		else
		{
			return $deft;		
		}
	}

	function getParam_POST($name,$deft=null)
	{
		if (isset($_POST[$name]))
		{
			return $_POST[$name];
		}
		else
		{
			return $deft;		
		}
	}

	function getParam_COOKIE($name,$deft=null)
	{
		if (isset($_COOKIE[$name]))
		{
			return $_COOKIE[$name];
		}
		else
		{
			return $deft;		
		}
	}
}

global $NX_REQUEST;
$NX_REQUEST = new NxApp_HttpRequest();

?>
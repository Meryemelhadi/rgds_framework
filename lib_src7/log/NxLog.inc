<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class _NxLog 
{
	function error($msg,$loc='',$level=9)
	{
	   // save to the error log, and e-mail me if there is a critical user error
	   _NxLog::writeLog(" user error: '$msg'\n");
	}

	// error handler triggered by PHP (error or warning)
	function onPHPError($errno, $errmsg, $filename, $linenum, &$vars) 
	{
		// error with @
		if (error_reporting() & $errno ==0) return;
			
	   // timestamp for the error entry
	   $dt = date("d/m/y H:i:s ");
	
	   // define an assoc array of error string
	   // in reality the only entries we should
	   // consider are 2,8,256,512 and 1024
	   $errortype = array (
	               1   =>  "Error",
	               2   =>  "Warning",
	               4   =>  "Parsing Error",
	               8   =>  "Notice",
	               16  =>  "Core Error",
	               32  =>  "Core Warning",
	               64  =>  "Compile Error",
	               128 =>  "Compile Warning",
	               256 =>  "User Error",
	               512 =>  "User Warning",
	               1024=>  "User Notice"
	               );
	   // set of errors for which a var trace will be saved
	   $user_errors = array(E_USER_ERROR, E_USER_WARNING, E_USER_NOTICE);
	   $err = "";
	   $err = $dt.$err.$errortype[$errno]."($errno): '$errmsg' in $filename on line:$linenum\n";
	
	   // save to the error log, and e-mail me if there is a critical user error
	   _NxLog::writeLog($err);
	}	
	
	// error handler triggered by PHP (error or warning)
	function log($msg,$loc='',$level=5) 
	{
		if ($level < NS_LOG_LEVEL)
			return;
				
	   _NxLog::writeLog(" log: '$loc::$msg'\n");
	}

	function writeLog($err)
	{
	   $err=date("d/m/y H:i:s ").$err;
	   
	   if (!@error_log($err, 3, NX_BASE_LOG.NX_LOG_FILE))
	   {
	   		if (!is_file(NX_BASE_LOG."phperrors.log"))
	   		{
				$fp=fopen(NX_BASE_LOG."phperrors.log", 'w');
				fwrite($fp,"opening log");
				fclose($fp);
	   		}
	   		error_log("LOG FILE CREATION", 3, NX_BASE_LOG."phperrors.log");
	   		error_log($err, 3, NX_BASE_LOG."phperrors.log");
	   }
	}
}

if (!defined('traceLev'))
	define('traceLev',true);

function trace($msg,$loc='')
{
	if (traceLev)
	{
		echo "$loc::$msg<br>";
	}
}

// ======= LOG =======
if (!defined('NS_LOG_LEVEL'))
	define('NS_LOG_LEVEL',6);
	
define('NX_DEBUG',1);
define('NX_COMPILE_EVENT',2);
define('NX_PORTAL',7);

function nxLog($msg,$loc='',$level=5)
{
//	_NxLog::log($msg,$loc,$level);
}

// ======= ERRORS =======
function nxError($msg,$loc='',$level=9)
{
	_NxLog::error($msg,$loc,$level);
}

// ------- redirect errors to our own logger -------
// user defined error handling function
function nxErrorHandler ($errno, $errmsg, $filename, $linenum, $vars) 
{
	_NxLog::onPHPError($errno, $errmsg, $filename, $linenum, $vars);
}

// handle errors
set_error_handler('nxErrorHandler');

?>
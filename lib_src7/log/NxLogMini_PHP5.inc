<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class _NxLog 
{
	static function error($msg,$loc='',$level=9)
	{
	   // save to the error log, and e-mail me if there is a critical user error
	   _NxLog::writeLog("USER ERROR: '$loc::$msg'\n");
	}

	// error handler triggered by PHP (error or warning)
	static function onPHPError($errno, $errmsg, $filename, $linenum, &$vars) 
	{
	   // if ($errno>=2048) return;

		// error with @
		if (error_reporting() & $errno ==0) return;
			
	   // timestamp for the error entry
	   $dt = date("d/m/y H:i:s ");
	
	   // define an assoc array of error string
	   // in reality the only entries we should
	   // consider are 2,8,256,512 and 1024
	   $errortype = array (
	               1   =>  "ERROR",
	               2   =>  "WARNING",
	               4   =>  "PARSING ERROR",
	               8   =>  "NOTICE",
	               16  =>  "Core ERROR",
	               32  =>  "Core WARNING",
	               64  =>  "Compile ERROR",
	               128 =>  "Compile WARNING",
	               256 =>  "User ERROR",
	               512 =>  "User WARNING",
	               1024=>  "User NOTICE",
	               2048=>"E_STRICT (PHP5)",

	               4096=>"E_RECOVERABLE_ERROR (PHP5)",  
	               2047=>"E_ALL",
	               6143=>"E_ALL",
	               8191=>"E_ALL",
	               );
	   // set of errors for which a var trace will be saved
	   $user_errors = array(E_USER_ERROR, E_USER_WARNING, E_USER_NOTICE);
	   $err = "";
	   if ($errno==8 || $errno==2048)
	   		return;
	   		
		$etype = isset($errortype[$errno])?$errortype[$errno] : $errno;
	   $err = $dt.$err.$etype."($errno): '$errmsg' in $filename on line:$linenum\n";
	
	   // save to the error log, and e-mail me if there is a critical user error
	   _NxLog::writeLog($err);
	}	
	
	// error handler triggered by PHP (error or warning)
	static function log($msg,$loc='',$level=5) 
	{
		if ($level < NS_LOG_LEVEL)
			return;
				
	   _NxLog::writeLog("\n log: '$loc::$msg'\n");
	}

	static function writeLog($err)
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
}

function nxLogStart($msg,$loc='',$level=5)
{
	return false;
}

function nxLogStop($msg,$loc='',$level=5)
{
	return false;
}

// ======= ERRORS =======
function nxError($errmsg,$loc='',$level=9)
{
	_NxLog::error($errmsg,$loc,$level);
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
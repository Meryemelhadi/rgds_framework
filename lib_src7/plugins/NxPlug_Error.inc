<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_Error
{
	var $props;
	
	function  NxPlug_Error(&$props)
	{
		$this->props = & $props;
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

	   /*
	   $err = "<errorentry>\n";
	   $err .= "\t<datetime>".$dt."</datetime>\n";
	   $err .= "\t<errornum>".$errno."</errornum>\n";
	   $err .= "\t<errortype>".$errortype[$errno]."</errortype>\n";
	   $err .= "\t<errormsg>".$errmsg."</errormsg>\n";
	   $err .= "\t<scriptname>".$filename."</scriptname>\n";
	   $err .= "\t<scriptlinenum>".$linenum."</scriptlinenum>\n";
	
	   if (function_exists("wddx_serialize_value"))
		   if (in_array($errno, $user_errors))
		       $err .= "\t<vartrace>".wddx_serialize_value($vars,"Variables")."</vartrace>\n";

	   $err .= "</errorentry>\n\n";
	   */
	   $err = "";
	   $err = $dt.$err.$errortype[$errno]."($errno): '$errmsg' in $filename on line:$linenum\n";
	
	   // for testing
	   // echo $err;
	
	   // save to the error log, and e-mail me if there is a critical user error
	   $this->writeLog($err);
	   /*
	   if ($errno == E_USER_ERROR)
	       mail("phpdev@example.com","Critical User Error",$err);
	    */
	}	
	
	// error handler triggered by PHP (error or warning)
	function onUserError($errmsg) 
	{
	   // timestamp for the error entry
	   $dt = date("d/m/y H:i:s ");
	
	   $err = "";
	   $err = $dt." user error: '$errmsg'\n";
	
	   // save to the error log, and e-mail me if there is a critical user error
	   $this->writeLog($err);
	}	

	// error handler triggered by PHP (error or warning)
	function log($msg) 
	{
	   // timestamp for the error entry
	   $dt = date("d/m/y H:i:s ");
	
	   $err = "";
	   $err = $dt." log: '$msg'\n";
	
	   // save to the error log, and e-mail me if there is a critical user error
	   $this->writeLog($err);
	}

	function writeLog($err)
	{
	   if (!@error_log($err, 3, NX_BASE_LOG."phperrors.log"))
	   {
	   		if (!is_file(NX_BASE_LOG."phperrors.log"))
	   		{
				$fp=fopen($f, 'w');
				fwrite($fp,"opening log");
				fclose($fp);
	   		}
	   		error_log("LOG FILE CREATION", 3, NX_BASE_LOG."phperrors.log");
	   		error_log($err, 3, NX_BASE_LOG."phperrors.log");
	   }
	}
}

?>
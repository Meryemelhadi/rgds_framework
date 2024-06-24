<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxPlug_Cookie
{
	function __construct(&$props)
	{
		$this->props =& $props;
	}

	function setCookie($name,$varval,$expire=-1000, $path="/",$domain=null,$secure=0) 
	{
		$n = "_". strtr($name, array('-' => '_', ' ' => '_', "."=>"_")); 
		// $data = base64_encode(serialize($varval));
		$data = $varval;
		
		$COOKIE = "Set-Cookie: $n=$data";
		if (isset($expire) && ($expire > 0))
		{
			if (ereg("MSIE", getenv("HTTP_USER_AGENT"))) 
			{
				header('P3P: CP="NOI ADM DEV PSAi COM NAV OUR OTRo STP IND DEM"');
				$time = mktime()+$expire;
				$date = gmdate("D, d-M-Y H:i:s \G\M\T", ($time));	
			} 
			else 
				$date = time()+$expire;
			
			$COOKIE .= "; EXPIRES=". $date;
		}
			
		if (isset($domain))
			$COOKIE .= "; DOMAIN=$domain"; 
			
		if (isset($path))
			$COOKIE .= "; PATH=$path";

		if (isset($secure) && $secure>0) 
			$COOKIE .= "; SECURE";
			
		header($COOKIE,false);
		return true;
	}
	
	function &getCookie($name,$dft=null)
	{
		$n = "_". strtr($name, array('-' => '_', ' ' => '_', "."=>"_")); 
		if (isset($_COOKIE["$n"]))
			return $_COOKIE["$n"];
		return $dft;
		return unserialize(base64_decode($_COOKIE["$n"]));
		// return $_COOKIE[$n];
	}
	
	function deleteCookie($name)
	{
		$n = "_". strtr($name, array('-' => '_', ' ' => '_', "."=>"_")); 
		$this->setCookie("$n","",-3600*48,"/");
	}

}
?>
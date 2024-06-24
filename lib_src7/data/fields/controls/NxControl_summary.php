<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_summary
{
	function __construct()
	{
	}
	
	function toHTML($value,&$f)
	{		
		$fname = $f->desc->getProperty('field', NULL, false);
		$fValue = $f->record->$fname;
	
		$length = $f->desc->getProperty('length', NULL, false);
		if( trim($length) == '') { 
			$length = 50; 
		}
		
		$value = $fValue;
		
		if( strlen($fValue) >= $length )
			$value = utf8_encode(substr (utf8_decode($value), 0, $length).' (...)');
		
		return $value;
	}
}

?>
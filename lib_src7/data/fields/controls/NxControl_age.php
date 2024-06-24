<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_age
{
	function __construct()
	{
	}
	
	function toHTML($value,&$f)
	{	
		$fname = $f->desc->getProperty('field', NULL, false);
		$fValue = $f->record->$fname;

		if( $fValue->isOk() === true ) {
			$dateOfBirth = $fValue->year->value.'-'.$fValue->month->value.'-'.$fValue->day->value;
			return floor(abs(strtotime(date("Y-m-d")) - strtotime($dateOfBirth)) / (365*60*60*24));
		}
		
		return '';
	}
}

?>
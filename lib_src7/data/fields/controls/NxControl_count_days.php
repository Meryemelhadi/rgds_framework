<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_count_days
{
	function __construct()
	{
	}
	
	function toHTML($value,&$f)
	{	
		$fname = $f->desc->getProperty('date1', NULL, false);
		$oDate1 = $f->record->$fname;
		$date1 = $oDate1->object;
		
		$fname = $f->desc->getProperty('date2', NULL, false);
		$oDate2 = $f->record->$fname;
		$date2 = $oDate2->object;

		if( !$oDate1->isOk() || !$oDate2->isOk() ) {
			return '-';
		}		

		//Calcul des timestamp
		$timestamp1 = mktime(12,0,0,$date1['month'] ,$date1['day'],$date1['year']); 
		$timestamp2 = mktime(12,0,0,$date2['month'] ,$date2['day'] ,$date2['year']);
		
		$prefix = '';
		
		if( $timestamp1 > $timestamp2 ) $prefix = '-';

		return $prefix.abs($timestamp2 - $timestamp1)/86400;
	}
}

?>
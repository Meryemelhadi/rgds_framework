<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_average_per_working_day
{
	function __construct()
	{
	}
	
	function toHTML($value,&$f)
	{
		$desc=&$f->desc;
		$fstart = $desc->getProperty('field_date_start',null,false);
		$fend = $desc->getProperty('field_date_end',null,false);

		$record = $f->record;
		$date_start = $record->$fstart;
		$date_end = $record->$fend;

		include_once(NX_XLIB.'lib/rh/conge/workflow.inc');
		$rh_demand = new rh_conge_demand();
		$nbWorkingDays = $rh_demand->countWorkingDays($date_start, $date_end,$halfStartDate=0,$halfEndDate=0,$nbHols);
		
		if ($nbWorkingDays==0)
			$nbWorkingDays = 1;

		$average = sprintf('%2.1f (%d %s)',$value / $nbWorkingDays,$nbWorkingDays,$desc->getString('days'));
		return $average;
	}
}

?>
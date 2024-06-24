<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

// data source for getting empty records. Same as "no_data"
require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_csv extends NxDS_Source
{
	function  NxDS_csv(&$props,$instance=false)
	{
		parent::__construct($props);
		$this->instance = $instance;
	}

	function loadConfig($connexion) 
	{
		if($connexion && $connexion!='default')
        	$cnxName='.'.$connexion;
        else
        	$cnxName='';

		if (
			is_file($f=NX_CONF.'ds/csv'.$cnxName.NX_HOST_SWITCH.'.inc') || 
			is_file($f=NX_CONF.'ds/csv'.NX_HOST_SWITCH.'.inc')
			)
		{
			// load connexion parameters
			include($f);
		}
		else
		{
			$errorCB->addError("cant find csv driver for ".$cnxName);
			return;
		}

		return $properties;
	}

	function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		// get file path
		$filepath = $props->getProperty('path');

		// get record description
		if (($recDesc = & $this->getRecDesc($view,$errorCB))==null)
			return null;
		
		// get columns names from dml
		$colNames = array_keys($recDesc->fdescs);

		$row = 1;
		$records=array();
		if (($handle = fopen($filepath, "r")) !== FALSE) 
		{
			if (($header = fgetcsv($handle, 1000, ",")) === FALSE)
				break;

			while (($data = fgetcsv($handle, 1000, ",")) !== FALSE)
			{
				$num = count($data);
				for ($c=0; $c < $num; $c++) 
				{
					$record[$colNames[$c]]=$data[$c];
				}
				$records[]=$record;
				$row++;
			}
			fclose($handle);
		}

		return new NxData_ArrayRecords($records,$recDesc);
	}	

	function getRecordCount($view, &$errorCB)
	{		
		// check presence of fields ?
		// $_POST[]
		return 1;
	}

	// add/modify records with view name
	function putRecords(&$records,$view, &$errorCB,$dbOp=null)
	{	
		return true;
	}
	
	// ds interface
	function deleteRecords($view, &$records, &$errorCB)
	{
		return true;
	}
	
}

?>
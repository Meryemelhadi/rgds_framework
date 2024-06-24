<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

// data source for getting empty records. Same as "no_data"
require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_files extends NxDS_Source
{
	function  NxDS_files(&$props,$instance=false)
	{
		parent::__construct($props);
		$this->instance = $instance;
	}

	function loadConfig($connexion) 
	{
		$properties = array();

		if($connexion && $connexion!='default')
        	$cnxName='.'.$connexion;
        else
        	$cnxName='';

		if (
			is_file($f=NX_CONF.'ds/files'.$cnxName.NX_HOST_SWITCH.'.inc') || 
			is_file($f=NX_CONF.'ds/files'.$cnxName.'.inc') || 
			is_file($f=NX_CONF.'ds/files'.NX_HOST_SWITCH.'.inc')
			)
		{
			// load connexion parameters
			include($f);
		}
		else
		{
			nxError("cant find csv driver for ".$cnxName,'DS_FILES');
			return;
		}

		if ($properties)
			$this->addProperties($properties);

		return $properties;
	}

	function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		$this->loadConfig($this->instance);

		if(($path=$props->getProperty('files.path',null))==null)
			$path=$this->getProperty('files.path',NX_CONF.'imports/');

		if(($re = $props->getProperty('files.pattern','*.csv'))==null)
			$re = $this->getProperty('files.pattern','*.csv');

		if(($levels = $props->getProperty('files.levels',1))==null)
			$levels = $this->getProperty('files.levels',1);

		$fs = new NxPlug_FileSys($this);
		$files = $fs->listFilesRegEx($path,$re,$levels);
		$records = array();
		$base = $fs->formatPath($path);
		foreach($files as $file)
		{
			$local = str_replace(array($base,'\\'),array('/','/'),$file);
			$records[] = array('path'=>$file,'base'=>$base,'local'=>$local);
		}

		if (($recDesc = & $this->getRecDesc($view,$errorCB,$props))==null)
			return new NxData_ArrayRecords($records,$this->recDesc);
		else
			return new NxData_ArrayRecords($records);
	}	

	function getRecordCount($view, &$errorCB)
	{		
		$this->loadConfig($this->instance);

		$path=$this->getProperty('import.path',NX_CONF.'imports/');
		$re = $this->getProperty('import.file_pattern','*.csv');
		$levels = $this->getProperty('import.levels',1);

		$fs = new NxPlug_FileSys($this);
		$files = $fs->listFilesRegEx($path,$re,$levels);
		
		return count($files);
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
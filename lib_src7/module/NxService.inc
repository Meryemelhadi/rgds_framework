<?php

include_once(NX_LIB."module/NxModule.inc");

class NxService extends NxModule 
{
	function __construct($path)
	{
		// load resources for service
		$site=&NxSite::getProps();
		$props=$site->loadResource("services/{$path}",$this->error ,false);
		parent::__construct($props,$site);
		
		// declare method table
		$this->methodTable =& $this->getProperty('methods');
	}
	
	function & getDataAsArray(&$records, $flatten=false, $remapWith="role",$fmethod="toObject")
	{
		$recs = array();
		
		// loop on all records (pages) and get link parameter, text and tooltip
		if ($records!=null)
			for ($recIt = $records->getRecordIterator(),$i=0; $recIt->isValid(); $recIt->next(),$i++)
			{
				$record = & $recIt->getCurrent();
				for ($fieldIt = $record->getFieldIterator(); $fieldIt->isValid(); $fieldIt->next())
				{
					$field = & $fieldIt->getCurrent();
					$role = $field->desc->getProperty($remapWith,null,false);
					if ($role !== null)
						$recs[$i][$role]=$field->$fmethod();						
				}
			}

		// flatten array if requested
		if (count($recs)==1 && $flatten)
			return $recs[0];
		else
			return $recs;
	}	
}

?>
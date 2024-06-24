<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxData_Output
{
	var $fmethod;
	var $recDesc;
	
	function __construct(&$recDesc, $dataFormat)
	{
		$this->$recDesc = & $recDesc;
		
		switch($dataFormat)
		{
			case "html":
				$this->fmethod = "toHTML";
				break;
			case "form":
				$this->fmethod = "toForm";
				break;
			case "store":
				$this->fmethod = "toObject";
				break;
			case "db":
				$this->fmethod = "toDB";
				break;
			default:
				break;
		}
	}
	
	// functions to overload according to output media
	function writeField(&$field)
	{
		$fmethod = $this->fmethod;
		echo $field->$fmethod();
	}
	
	function writeHeader()
	{
		echo "";
	}
	
	function writeFooter()
	{
		echo "";
	}

	function writeRecord(&$record)
	{
		$this->writeHeader();
		$fieldIterator = $record->getFieldIterator();
		for ($fieldIterator->reset(); $fieldIterator->isValid(); $fieldIterator->next())
		{
			$field = & $fieldIterator->getCurrent();
			$this->writeField($field);
		}
		$this->writeFooter();
	}
}

?>

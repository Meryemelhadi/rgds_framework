<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

// data source for getting empty records. Same as "no_data"
require_once(NX_LIB.'ds/NxDS_Source.inc');
require_once(NX_LIB.'ds/alfresco/NxAlfresco.inc');

class NxDS_alfresco extends NxDS_Source
{
	function  NxDS_alfresco(&$props,$instance=false)
	{
		parent::__construct($props);
		$this->instance = $instance;
		$this->driver = new NxAlfresco($props,$instance);
	}

	function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		switch($ds)
		{
			case 'alfresco.users':
				$data = $this->driver->getUsers();
				break;
			case 'alfresco.spaces':
				$path = $props->getProperty('path');
				$data = $this->driver->getSpaces();
				break;
			case 'alfresco.folders':
				$data = $this->driver->getFolders();
				break;
			case 'alfresco.groups':
				$data = $this->driver->getGroups();
				break;
			default:
				return null;
		}


		// get record description
		if (($recDesc = & $this->getRecDesc($view,$errorCB))==null)
			return null;
		
		// get columns names from dml
		$colNames = array_keys($recDesc->fdescs);

		require_once(NX_LIB.'ds/excel/reader.php');

		// ExcelFile($filename, $encoding);
		$data = new Spreadsheet_Excel_Reader();

		// Set output Encoding.
		global $NX_CHARSET;
		$data->setOutputEncoding($NX_CHARSET);

		$data->read($filepath);

		$records = array();
		$nsheets = count($data->sheets);
		for ($sh = 0; $sh <= $nsheets ; $sh++) 
		{
			$shName = $data->boundsheets[$sh]['name'];

			$sheet = $data->sheets[$sh]['cells'];
			$nRows = $data->sheets[$sh]['numRows'];
			if ($nRows > 1)
			{
				// get header
				$header	= $sheet[1];
				$header[] = 'sheet_name';

				// get rows
				for ($r = 2; $r <= $nRows; $r++) 
				{
					$num = $data->sheets[$sh]['numCols'];

					// add field with sheet name
					for ($c = 1; $c <= $num; $c++) {
						$record[$colNames[$c-1]]=htmlspecialchars($sheet[$r][$c],ENT_QUOTES);
					}
					$record['sheet_name']=htmlspecialchars($shName,ENT_QUOTES);

					$records[]=$record;
				}
			}
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
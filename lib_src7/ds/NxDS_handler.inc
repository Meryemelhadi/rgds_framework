<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// Data source for MySQL

require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_handler extends NxDS_Source
{
	function  __construct(&$props,$instance=false)
	{
		parent::__construct($props);
		$this->instance = $instance;
	}

	function initHandler($handlerN,$package='')
	{		
		if (!$handlerN)
			return false;

		$className=strtr($handlerN,'.','_');
		$aname=explode('.',$handlerN);
		$type=array_shift($aname);
		$name=implode('/',$aname);

		if (!class_exists($className))
			if($f = $this->getConfFile($type,$name.'.inc',$package))
				include_once($f);
			else
			{
				nxError("DS::HANDLER cant find file for class $className");
				return false;
			}
							
		if (class_exists($className))
			return new $className($GLOBALS['current_page'],$this->instance);

		nxError("DS::HANDLER cant find class definition $className in file $f");

		return false;
	}

	// ========= data source interface =======
	function getRecords($view, &$errorCB,$data=null,$props=null)
	{
		if (($recDesc = $this->getRecDesc($view,$errorCB,$props))==null)
			return null;

		// if defined package in view, uses it
		$package=$recDesc->getProperty('package','',false);
					
		$h = $recDesc->getProperty('handler',null,false);
		$aH = explode('@',$h);
		if(count($aH)>1)
		{
			$h = $aH[0];
			$package=$aH[1];
		}
					
		// get handler object
		$handler=$this->initHandler($h,$package);
		if ($handler)
			return $handler->getRecords($recDesc,$errorCB,$data,$props);

		// defaults to db if no handler

		// get recordset LIMIT
		$maxNbRecords = $this->getProperty("{$view}.db.nbRecordPerPage",15);
		$indexFirst = $this->getProperty("{$view}.db.indexFirstRecord",0);

		// get db object
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		return $db->getDBRecords($recDesc,$errorCB, $indexFirst, $maxNbRecords,$data);

	}

	function getRecordCount($view, &$errorCB,$props=null)
	{
		if (!($recDesc = $this->getRecDesc($view,$errorCB,$props)))
			return false;
		
		// get handler object
		$handler=$this->initHandler($recDesc->getProperty('handler',null,false));
		if (!$handler)
			return false;
			
		return $handler->countRecords($recDesc,$errorCB);
	}

	// add/modify records with view name
	function putRecords(&$records,$view,&$errorCB,$dbOp='insert',$props=null)
	{
		if (!($recDesc = $this->getRecDesc($view,$errorCB,$props)))
			return false;

		// get handler object
		$handler=$this->initHandler($recDesc->getProperty('handler',null,false));
		if (!$handler)
			return false;
						
		return $handler->addRecords($recDesc,$errorCB,$records,$dbOp);
	}
	
	// add/modify records with view name
	function putRecord(&$record,$view,&$errorCB,$dbOp='insert',$props=null)
	{
		if (!($recDesc = $this->getRecDesc($view,$errorCB,$props)))
			return false;
			
		// get handler object
		$handler=$this->initHandler($recDesc->getProperty('handler',null,false));
		if (!$handler)
			return false;

		return $handler->addRecord($recDesc,$errorCB,$record,$dbOp);
	}

	function deleteRecords($view, &$records, &$errorCB,$props=null)
	{
		if (!($recDesc =&$this->getRecDesc($view,$errorCB,$props)))
			return false;
			
		// get handler object
		$handler=$this->initHandler($recDesc->getProperty('handler',null,false));
		if (!$handler)
			return false;

		if ($props==null)
			$props=&$this;
			
		if ($records && $records->count()>0)
		{
			// delete resources
			$records->onDelete($errorCB);
			
			// delete record
			$handler->deleteRecord($recDesc, $errorCB,$props,$records->getCurrent());
		}
		else
			$handler->deleteRecord($recDesc, $errorCB,$props,null);
	}	
}

?>
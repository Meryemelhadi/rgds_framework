<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// Data source for MySQL

require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_db extends NxDS_Source
{
	var $db;
	
	function  __construct(&$props,$instance=false)
	{
		parent::__construct($props);
		$this->instance = $instance;
	}

	// ========= data source interface =======
	function getRecords($view, &$errorCB,$data=null,$props=null,$query=null)
	{
		if (($recDesc = $this->getRecDesc($view,$errorCB,$props))==null)
			return null;
		
		// get recordset LIMIT
		$maxNbRecords = $this->getProperty("{$view}.db.nbRecordPerPage",15);
		$indexFirst = $this->getProperty("{$view}.db.indexFirstRecord",0);

		// get db object
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		$records = $db->getDBRecords($recDesc,$errorCB, $indexFirst, $maxNbRecords,$data,$query);

		// clone them for getting in memory collection
		// useful for text-choice managament
		if ($records && $records->count() < 500)
		{
			$records = $records->clone_it($errorCB,false);
		}

		return $records;
	}

	function getRecordCount($view, &$errorCB,$props=null)
	{
		if (!($recDesc = $this->getRecDesc($view,$errorCB,$props)))
			return false;
		
		// get db object
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;
			
		return $db->countRecords($recDesc,$errorCB);
	}

	// add/modify records with view name
	function putRecords(&$records,$view,&$errorCB,$dbOp='insert',$props=null)
	{
		if (!($recDesc = $this->getRecDesc($view,$errorCB,$props)))
			return false;
			
		// get db object
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;
			
		if ($dbOp=='insert') 
			$this->_currentIID=null;
		return $db->addDBRecords($recDesc,$errorCB,$records,$dbOp);
	}
	
	// add/modify records with view name
	function putRecord(&$record,$view,&$errorCB,$dbOp='insert',$props=null)
	{
		if (!($recDesc=$this->getRecDesc($view,$errorCB,$props)))
			return false;
			
		// get db
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		if ($dbOp!='update') 
			$this->_currentIID=null;

		$res = $db->addDBRecord($recDesc,$errorCB,$record,$dbOp);

		// extended record?
		if ($dsView2=$recDesc->getProperty('extendsdWithDs',null,false))
		{
			nxLog("Store extended record with $ds2","DS_DB");
			$this->putSubRecord($record,$dbOp,$props,$dsView2,null,'db');
		}

		return $res;
	}

	// embedded record
	function putSubRecord($record,$op,$props,$ds2,$ctxt=null,$media=null)
	{
		if (!$ctxt && isset($GLOBALS['current_page']))
		{
			$ctxt = $GLOBALS['current_page'];
		}

		if(!$media)
			$media='db';

		return $ctxt->putRecord($record,$ds2,$op,$media,$format='Object',$props);
	}

	function deleteRecords($view, &$records, &$errorCB,$props=null)
	{
		// get db
		$errorCB->clear();
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		if ($props==null)
			$props=&$this;
			
		if ($records && $records->count()>0)
		{
			// delete resources
			$records->onDelete($errorCB);
			
			// delete from DB (NB. the SQL request from that db view is going to delete them all..)
			$db->executeDBQuery($view, 'deleteRecord', $errorCB,true,$props,true,$records->getCurrent());
		}
		else
			$db->executeDBQuery($view, 'deleteRecord', $errorCB,true,$props,true);
	}

	// management of last insert ID for embedded subforms
	var $_savedIID = array();
	var $_currentIID = null;

	function saveLastInsertID() {
		$errorCB = new NxErrorCB();

		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		$this->_savedIID[] = $db->getLastInsertID($id);
		$this->_currentIID=null;

		return $id;
	}

	function restoreLastInsertID() {
		$this->_currentIID = array_pop($this->_savedIID);
	}
	
	function getLastInsertID(&$id,&$errorCB) {
		// if ($this->_currentIID)	return $this->_currentIID;

		// get db object
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		return $db->getLastInsertID($id);
	}

	function query($query,&$errorCB,$withData=false) {
		// if ($this->_currentIID)	return $this->_currentIID;

		// get db object
		$db = $this->getDB($errorCB,$this->instance);
		if ($errorCB->isError())
			return false;

		if($withData)
			return $db->queryWithData($query);
		else
			return $db->query($query);
	}
}
?>
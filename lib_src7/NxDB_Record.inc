<?php

require_once(NX_LIB . "NxData_Record.inc");

// record collection
class NxDB_Records extends NxData_Records
{
    var $db;
    function __construct(&$db, &$recDesc, $resultSet) 
    { 
    	parent::__construct($recDesc);
    	$this->db =& $db;
    	$this->resultSet = $resultSet;
    	$this->recDesc=&$recDesc;
    }
    
    function count() 
    { 
    	return $this->db->numRows($this->resultSet);  
    }

    function getRecordIterator() 
    {
        return new NxDB_RecordIterator($this->db,$this->resultSet,$this->recDesc);
    }

    function &getCurrent()	
    { 
    	$it = new NxDB_RecordIterator($this->db,$this->resultSet,$this->recDesc);
    	return $it->getCurrent();
    }
}

// record iterator
class NxDB_RecordIterator
{
    var $db;
	var $fields;
	var $resultSet;

    function __construct(&$db,&$resultSet,&$recDesc) { 
		$this->db =& $db; 
		$this->resultSet = &$resultSet;
    	$this->recDesc=&$recDesc;        
    	/* $this->className=$recDesc->getClass(); */

		$this->reset();
		/*
		$numRow = $this->db->numRows();
		if ($numRow)
			$this->reset();
		else
			$this->fields = null;		
		*/
	}

	function reset() { $this->db->resetRecords($this->resultSet); $this->next(); }

    function next()  
	{ 
		$this->fields = $this->db->queryFields($this->resultSet); 
	}

    function isValid() 		{ return $this->fields != null;  }
    function &getCurrent()	{ 
    	$record = new NxData_Record($this->recDesc);
		// $record->inMemory = false;
    	$record->initFromArray($this->fields, 'db', $this->db->errorCB,'db');

/*		if($this->className && $this->className != get_class($record) && class_exists($this->className))
		{
			$res = new $this->className();
			$res->setRecord($record);
			return $res->record;
		}
*/
    	return $record; 
    }
    // function &getFieldIterator() { return new NxData_DbFieldIterator($this->record, $this->nbfields); }
}

?>
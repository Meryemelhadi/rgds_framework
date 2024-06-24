<?php

require_once(NX_LIB . 'NxData_Record.inc');

// record collection
class NxData_ArrayRecords extends NxData_Records
{
    var $records;
    var $format;
    var $count;
    
    function __construct($records=null, $recDesc=null, $format='array') 
    { 
    	// copy records data
    	if (isset($records) && is_array($records) && count($records)>0)
    	{
    		if (isset($records[0]) && (is_array($records[0]) || isset($records[0]->fields)))
    			// array of array => records
    			$this->records=$records;
    		else
    		{
    			// probably a single record
    			$this->records=array($records);
    		}
    	}
		else
			$this->records = array(); 

		$this->count = count($this->records);

    	// if no data descriptor, create one from data    	
    	if (!$recDesc) 
    	{
	    	if ($this->count==0)
	    		$recData = array();
	    	else
	    		$recData =& $this->records[0];
	    	
    		$recDesc = $this->createRecDesc($recData);
    	}
		
    	parent::__construct($recDesc);
    	$this->format = $format;
    }
    
    // returns properties from first record
   	function &getProperties($flatten=false)
	{
	 	return $this->records[0];
	}

    
    function count() 
    { 
    	return $this->count;
    }

    function getRecordIterator() 
    {
        return new NxData_ArrayRecordIterator($this);
    }
    
    function &item($index) {
    	if (isset($this->records[$index]))
    		return $this->records[$index];
    	
    	return null;
    }
    
    function addRecord(&$record) 
    {
    	/*
    	if (is_a($record,'NxData_Record'))
    		// copy from record object
    		$this->records[]=$record->getRecordData();
    	else
    		// simple array 		
    		$this->records[]=$record;
    	*/
    	// store the record in its own format (raw data or well formed record)
   		$this->records[]=$record;   	
   		$this->count++;
    }

    function getCurrent()	
    { 
    	$it = new NxData_ArrayRecordIterator($this);
    	return $it->getCurrent();
    }
    
    // create a record descriptor by using data in the array
    function createRecDesc(&$recData)
	{
		$fprops=array();
		$falias=array();
		$rprops=array();
		$frole=array();
		
		foreach ($recData as $ufname => $val)
		{
			$fprops[] =array("name"=>$ufname, "srcName"=>$ufname, "ufname"=>$ufname, "type"=>"text", "label"=>$ufname);
			$falias[$ufname]=$ufname;
		}
		
		$pageDesc=null;
		
		return new NxData_RecordDesc($rprops, $fprops, $falias, $pageDesc, $frole);
	}
}

// record iterator
class NxData_ArrayRecordIterator
{
	var $records; // array of records
	var $recDesc;
	var $curRec; // current index
	var $format; // data format (db, form, array)

    function __construct(&$records) { 
		$this->records = $records->records;
		$this->recDesc = $records->getRecDesc();
		$this->format = $records->format;
    	/* $this->className=$this->recDesc->getClass(); */
		$this->curRec=0;
	}

	function reset() { $this->curRec=0; return true;}
    function next()  { $this->curRec++; }
    function isValid() 	{ return $this->curRec < count($this->records);  }
    
    function &getCurrent()	
    { 
    	$rec = &$this->records[$this->curRec];

    	if (!is_array($rec))
		{
    		// we suppose its a record class..
    		return $rec;
		}
    		
    	// $record is an array of raw data => rebuild the record
    	$record = new NxData_Record($this->recDesc);
    	$errorCB = new NxErrorCB();
    	$record->initFromArray($rec, $this->format, $errorCB);
    	if ($errorCB->isError()) 
    		nxError("can't get current array record","NxData_ArrayRecordIterator");
    		
    	return $record; 
    }
}

?>
<?php
require_once(NX_LIB . 'framework.inc');
require_once(NX_LIB . 'NxData_ArrayRecords.inc');

class PersistentObject
{
	var $record;
	var $ctxt;

	function __construct($ds=null,$media='db',$params=null)
	{		
		if ($ds)
			$this->load($ds,$media,$params);
	}

	function setRecord($record)
	{
		if ($record)
			$record->setObject($this);
			
		$this->record=$record;
	}

	function getRecDesc() {
        if($this->record)
			return $this->record->getRecDesc();
		return null;
	}

	function isOk() {
		return ($this->record!=null);
	}

	function load($ds,$media='db',$params=null)
	{
		$ctxt = Framework::getContext();
		// $ctxt->addProperties($params);
		// $parent=null;
		$props=new NxProperties($params,$ctxt);
		$this->setRecord($ctxt->getRecord($ds,null,$media,false,null,$props));
	}

	function store($ds,$op='insert',$media='db',$params=null)
	{
		$ctxt = Framework::getContext();
		$parent=null;
		$props=new NxProperties($params,$ctxt);
		$ctxt->putRecord($this->record,$ds,$op,$media,'Object',$props);
	}

	function delete($ds,$media='db',$params=null)
	{
		$ctxt = Framework::getContext();
		$parent=null;
		$props=new NxProperties($params,$ctxt);
        if($this->record)
                $records=$this->record;
        else
                $records=null;
        
		$ctxt->deleteRecords($ds, $records,$media, $deleteRes=true,$props);
	}

	/* magic getter: returns field */
	function __get($fname) {
		if ($this->record==null) 
			return null;

		return $this->record->getField($fname);
	}
	/* magic setter : set field value*/
	function __set($fname,$value) {
		if ($this->record==null) 
			return null;
		$this->record->setField($fname,$value);
	}

	function getField($fname)
	{
		if ($this->record==null) 
			return null;

		return $this->record->getField($fname);
	}
	function getFieldValue($fname,$format='html',$deftVal=null,$opts=null)
	{
		if ($this->record==null) 
			return null;

		return $this->record->getFieldValue($fname,$format,$deftVal);
	}

	function setFieldValue($fname,$value,$format='db')
	{
		if ($this->record==null) 
			return null;

		return $this->record->setField($fname,$value);
	}

	function getLastInsertId($media='db') {
		$ctxt = Framework::getContext();
		return $ctxt->getLastInsertID($media);
	}
}

class PersistentCollection
{
	var $records;
	var $preload;

	function __construct($ds=null,$media='db',$params=null,$data=null,$preload=true)
	{		
		$this->preload=$preload;

		if ($ds)
			$this->load($ds,$media,$params,$data);
	}

	function isOk() {
		return ($this->records!=null);
	}

	function load($ds,$media='db',$params=null,$data=null)
	{
		$ctxt = Framework::getContext();
		$parent=null;
		$props=new NxProperties($params,$ctxt);
		$this->records=$ctxt->getRecords($ds,null,$media,false,$data,$props);

		// transform to record array
		if ($media=='db' && $this->preload===true)
		{
			if ($this->records!=null)
				$this->records = $this->records->getRecordArray();
			else {
				nxError("ORM : cant load DS : $ds",'DB');
			}
		}
	}

	function store($ds,$op='insert',$media='db',$params=null)
	{
		$ctxt = Framework::getContext();
		$ctxt->putRecords($this->records,$ds,$op,$media,new NxProperties($params,$ctxt));
	}

	function delete($ds,$media='db',$params=null)
	{
		$ctxt = Framework::getContext();
		$props=new NxProperties($params,$ctxt);
		$ctxt->deleteRecords($ds, $this->records,$media, $deleteRes=true,$props);
	}

	function getRecordArray() {
		if (!$this->records)
			return null;
		
		// iterate on current record collection and add cloned records to new collection
		$ar=array();
		for ($iterRec = $this->records->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$ar[]=$iterRec->getCurrent();
		}

		return $ar;
	}

	function getObjectArray($class='PersistentObject') {
		if (!$this->records)
			return null;
		
		// iterate on current record collection and add cloned records to new collection
		$ar=array();
		for ($iterRec = $this->records->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$o=new $class();
			$o->setRecord($iterRec->getCurrent());
			$ar[]=$o;
		}

//		return $this->objects = $ar;
		return $ar;
	}

	function onEach($cb,$class='PersistentObject') {
		if (!$this->records)
			return null;
		
		// iterate on current record collection and add cloned records to new collection
		$i = 0;
		for ($iterRec = $this->records->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$rec = $iterRec->getCurrent();
			if($class)
			{
				$o=new $class();
				$o->setRecord($rec);
				$cb($o,$i);
			}
			else
				$cb($rec,$i);
		}
	}

	/* TPL records compatibility */
	function apply($class) {
		$this->getObjectArray($class);
		return $this;
	}

	function getRecordIterator() 
    {
		if (!$this->records)
			return null;

		return $this->records->getRecordIterator();
    }
	function count() 
    {
		if (!$this->records)
			return null;

		return $this->records->count();
    }
	function isPaged() 
    {
		if (!$this->records)
			return null;

		return $this->records->isPaged();
    }
	function getPageContext() 
    {
		if (!$this->records)
			return null;

		return $this->records->getPageContext();
    }

	/* magic getter : allow toi acess record as array or records property */
	function __get($fname) {
		switch($fname)
		{
			case 'array':
				return $this->getRecordArray();
			case 'records':
				return $this->records;
			case 'objects':
				return 	$this->getObjectArray();
			case 'iterator':
				// $this->objects = null;				
				return $this->getRecordIterator();
			default:
				// if (isset($this->objects)) return $this->objects;
				return $this->getObjectArray($fname);
		}
	}
	/* magic setter : build collection from array, records or object records */
	function __set($fname,$recs) 
	{
		switch($fname)
		{
			case 'objects':
				$a=array();
				foreach($recs as $o)
				{
					$a[] = $o->record;
				}
				$this->records = new NxData_ArrayRecords($a);
				// $this->objects = $recs;
				break;
			case 'array':
				$this->records = new NxData_ArrayRecords($recs);
				// $this->objects = null;				
				break;
			case 'records':
				$this->records = $recs;
				// $this->objects = null;				
				break;
			default:
		}
	}
}

?>
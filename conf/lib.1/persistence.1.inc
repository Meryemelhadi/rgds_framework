<?php

class PersistentObject
{
	var $record;
	var $ctxt;

	function PersistentObject($context=null,$ds,$media='db',$params=null)
	{		
		$this->ctxt=$context;
		$this->load($ds,$media,$params);
	}

	function load($ds,$media='db',$params=null)
	{
		if ($params)
			$props = new NxProperties($params,$this->ctxt);
		else
			$props=null;

		$this->record=$this->ctxt->getRecord($ds,null,$media,false,$props);
	}

	function store($ds,$media='db',$params=null)
	{
		$this->ctxt->putRecord($ds,null,$media,false,$this->record);
	}

	function getField($fname,$format='html',$deftVal=null,$opts=null)
	{
		if ($this->record==null) 
			return null;

		return $this->record->getFieldValue($fname,$format,$deftVal);
	}

}

class PersistentCollection
{
	var $records;
	var $ctxt;

	function PersistentCollection($context=null,$ds,$media='db',$params=null)
	{		
		$this->ctxt=$context;
		$this->load($ds,$media,$params);
	}

	function load($ds,$media='db',$params=null)
	{
		if ($params)
			$props = new NxProperties($params,$this->ctxt);
		else
			$props=null;

		$this->records=$this->ctxt->getRecords($ds,null,$media,false,$props);
	}

	function store($ds,$media='db',$params=null)
	{
		$this->ctxt->putRecords($ds,null,$media,false,$this->records);
	}

	function getRecordArray() {
		if ($this->records)
			$this->records->getRecordArray();

		return null;
	}
}

?>
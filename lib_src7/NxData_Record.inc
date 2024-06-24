<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'data/fields/NxDataFields.php');

class NxData_RecordDesc extends NxProperties
{
	var $fdescs;
	var $falias;
	var $frole;

	function __construct(&$rprops, &$fprops, &$falias, &$pageDesc, &$frole)
	{
		parent::__construct($rprops,$pageDesc);
		
	 	$this->fdescs = array();
	 	$this->falias = $falias;
	 	$this->frole  = $frole;

		if($configName=$this->getProperty('config',null,false))
		{
			$this->config = $config = $this->getConfig($configName);			
		}
		else
			$this->config = $config = null;
	 	
	 	foreach($fprops as $fprop)
	 	{
 			// create field descriptor from properties
 			$fdesc = new NxFieldDesc($fprop,$this,$config);

	 		// insert field in record
 			$fname = $fdesc->getName();
	 		$this->fdescs[$fname] = $fdesc;
	 	}

	}
	
	function getConfig($name) {
		include_once(NX_LIB.'NxConfig.inc');
		// $this->ctxt = Framework::getContext();
		$package=$this->getProperty("package",'',false);
		$config = NxConfig::getConfig($this,$name,$package);
		return $config->getProperty(trim($this->getProperty('table'),'_'));
	}

	// get a field desc by name
	function getFieldDesc($fname)
	{
	 	return $this->fdescs[$fname];	
	}
	
	// get a field desc by alias name (or source name)
	function getFieldDescByAlias($falias,$lookup="falias")
	{
		$a = $this->$lookup;
		if (isset($a[$falias]))
	 		return $this->desc[$a[$falias]];
	 		
	 	return null;
	}
	
	// get a field name by alias name (or source name)
	function getFieldNameByAlias($falias,$lookup="falias")
	{
		$a = $this->$lookup;
		if (isset($a[$falias]))
	 		return $a[$falias];
	 		
	 	return null;
	}
	
	// get a field desc by attribute presence or value.
	function getFieldDescByAttribute($atName, $atVal=null)
	{
	 	foreach($this->fdescs as $fdesc)
	 	{
	 		if (($v = $fdesc->getProperty($atName,null,false)) != null)
	 			if ($atVal == null || $v==$atVal)
	 				return $fdesc;
	 	}
	 	return null;
	}

	/* magic getter */
	function __get($fname) {
		switch($fname)
		{
			case 'view': return $this->getViewNameFull();
			case 'table': return $this->getTableName();
			case 'aspect': return $this->initAspect();
			case 'class': return $this->getClass();
			case 'fields':return $this->fdescs;
			default: if(isset($this->fdescs[$fname])) return $this->fdescs[$fname];
		}
		return null;
	}

	// get and load class
	function getClass()
	{
		$className=$this->getProperty("class",null,false);
		if ($className)
			return $this->loadClass($className);
		return null;
	}
	
	// dataset view name
	function getViewName()
	{
		return $this->getProperty("name",null,false);
	}

	function getViewNameFull()
	{
		$package=$this->getProperty("package",'',false);
		if ($package)
			$package='@'.$package;
		return $this->getProperty("name",null,false).$package;
	}
	
	// dataset table name
	function getTableName()
	{
		return $this->getProperty("name",null,false);
	}
	
	// actual SQL table
	function getSQLTableName()
	{
		return $this->getProperty("table",null,false);
	}

	function initAspect($aspect=null)
	{		
		if($aspectN = $this->getProperty('aspect',$aspect,false))
		{
			$className = $this->loadClass($aspectN);
			if($className)
				return new $className($this,$GLOBALS['current_page']);

			/*
			$className=strtr("aspects_records_{$aspectN}",'.','_');
			if (!class_exists($className) &&
				($f = $this->parent->getConfFile('aspects/records',strtr($aspectN,'.','/').'.inc')))
				include_once($f);
								
			if (class_exists($className))
				return new $className($this,$GLOBALS['current_page']);
			*/
		}
		return null;
	}
}

class NxData_Record
{
	var $fields;
	var $recDesc;
	var $recordPrefix;
	var $aspect;
	var $inMemory;
	var $lastInsertID=0;
  //  var $object=null;
	
	function __construct(&$recDesc)
	{
		$this->recDesc = & $recDesc;
		$this->fields = array();
		$this->recordPrefix='';
		$this->inMemory = true;
	}
	
	// a prefix to be used when generating form fields
	function setPrefix($s) {
		$this->recordPrefix=$s;
	}
	function getPrefix() {
		return $this->recordPrefix;
	}

	function setObject($o) {
		$this->object=$o;
	}

	function getObject($class=null) {
		if (!isset($this->object))
		{
			$class=$class?$class:$this->recDesc->getClass();
			if (!$class)
				return null;

			$object = new $class();
			$object->setRecord($this);
			return $object;
		}
		
		return $this->object;
	}
	
	function initAspect($recDesc=null,$aspect=null) {
		if (!$recDesc)
			$recDesc=$this->recDesc;
			
		$this->aspect=$recDesc->initAspect($aspect);
	}
	
	function initFromArray(&$data, $dataFormat, &$errorCB, $media=null)
	{
		$fdescs = & $this->recDesc->fdescs;
		$fields = & $this->fields;
		foreach(array_keys($fdescs) as $fname)
		{
			$fdesc = & $fdescs[$fname];
			$fields[$fname] = $fdesc->createField($data,$dataFormat,$errorCB,$this);
			$fields[$fname]->attachRecord($this);
		}

		// extends record?
		if ($dsView2=$this->recDesc->getProperty('extendsdWithDs',null,false))
		{
			nxLog("Extend record with $ds2","DS_DB");
			$this->extendsdWithDs($dsView2,null,$media);
		}
	}

	// merge record with other record
	function extendsdWithDs($ds2,$ctxt=null,$media=null) {
		if (!$ctxt && isset($GLOBALS['current_page']))
		{
			$ctxt = $GLOBALS['current_page'];
		}

		nxLog("Extend record with $ds2","DS_DB");
		if(!$media)
			$media='db';

		$rec2 = $ctxt->getRecord($ds2,$propName=null,$media,$addProps=false,$data=$this,$props=null);
		$this->updateWith($rec2,$override='no',$append='yes');

		return $rec2;
	}
	
	// update record with other record
	function updateWith(&$other,$override='yes',$append='yes')
	{
		$fields = & $this->fields;
		$fdescs = & $this->recDesc->fdescs;
		foreach($other->fields as $ofield)
		{
			// add/update fields and recdesc
			$fname = & $ofield->getName();
			if ((!isset($fields[$fname]) && $append=='yes') || 
				(isset($fields[$fname]) && $override=='yes'))
			{
				// no overlap and append or override
				$fields[$fname] = $ofield;
				$fdescs[$fname] = $ofield->desc;
				$fields[$fname]->attachRecord($this);
			}
			else
			{
				// overlap empty field
				if (isset($fields[$fname]) && ($override=='empty') && ($fields[$fname]->toStatus()=='empty'))
				{
					$fields[$fname] = $ofield;
					$fdescs[$fname] = $ofield->desc;
					$fields[$fname]->attachRecord($this);
				}
			}
		}
		return $this;
	}

	function getRecDesc()
	{
		return $this->recDesc;
	}
	
	// function to overload. Returns number of records in collection
	function count() {return 1;}
	
	function &getCurrent() 
	{
		return $this;
	}
		
	function getRecordData() { return $this->fields; }
	
	function getField($fname)
	{
		if (!isset($this->fields[$fname]))
		{
			if (($fname=$this->recDesc->getFieldNameByAlias($fname))==null)	
				return null;
		}

		return $this->fields[$fname];
	}

	/* magic getter */
	function __get($fname) {
		if ($fname=='object')
			return $this->object = $this->getObject();
		return $this->getField($fname);
	}
	/* magic getter */
	function __set($fname,$value) {
		if ($fname=='object')
			return $this->object = $value;
		$this->setField($fname,$value);
	}

	function setField($fname,$value)
	{
		if (!isset($this->fields[$fname]))
		{
			if (($fname=$this->recDesc->getFieldNameByAlias($fname))==null)	
				return null;
		}
		$this->fields[$fname]->setValue($value);
	}

	function fieldCount($all=false)
	{
		$count = count($this->fields);

		if (!$all)
			foreach ($this->fields as $f)
				if($f->isHidden())
					$count--;	
				
		return $count;
	}
	
	function getFieldValue($fname,$format,$deftVal,$opts=null)
	{
		if (!isset($this->fields[$fname]))
		{
			if (($fname=$this->recDesc->getFieldNameByAlias($fname))==null)	
				return $deftVal;
		}

		$fmethod = "to".$format; // ex: toDB, toHTML, toForm
		$f = $this->fields[$fname];
		return $f->$fmethod($opts);
	}
	
	// get a field by attribute
	function getFieldByAttribute($atName, $atVal=null)
	{
	 	foreach($this->fields as $f)
	 	{
	 		if (($v = $f->getProperty($atName,null,false)) != null)
	 			if ($atVal == null || $v==$atVal)
	 				return $f;
	 	}
 		return null;
	}

	// property like interface: field name == property name, fallback on rec desc
	// search by field name, alias and role
	function & getFieldBy($name,$deft=null,$lookup="alias")
	{
		$fname = $this->recDesc->getFieldNameByAlias($name,"f{$lookup}");
		if ($fname!==null && isset($this->fields[$fname]))
			return $this->fields[$fname];
		return null;
	}
	
	function setFieldsProperty($fnames,$prop,$value)
	{
		$recDesc = $this->getRecDesc();
		$fDescs = $recDesc->fields;
        
        if($fnames=='*')
            $fnames=array('*');
		
		foreach($fnames as $k=>$fname) 
		{
			// match all
			if($fname == '*')
			{
				foreach($fDescs as $fname => $desc){
					$fDescs[$fname]->setProperty($prop,$value);
				}
				break;
			}

			if((string)$k=='class')
			{
				foreach($fDescs as $fname2 => $desc)
				{
					if($desc->getProperty('class') == $fname)
						$fDescs[$fname2]->setProperty($prop,$value);
				}
			}

			// csv list
			if(strpos($fname,','))
			{
				$fnames = explode(',',$fname);
				foreach($fnames as $fname2)
				{
					if(isset($fDescs[$fname2]))
					{
						$fDescs[$fname2]->setProperty($prop,$value);
					}
				}
			}
			// single field name
			elseif(isset($fDescs[$fname]))
			{
				$fDescs[$fname]->setProperty($prop,$value);
			}
		}
	}

	// property like interface: field name == property name, fallback on rec desc
	// search by field name, alias and role
	// bug : format never set...
	function getProperty($name,$deft=null,$cascade=true,$format="HTML",$opts=null)
	{
		$fname = $this->recDesc->getFieldNameByAlias($name,"falias");
		if ($fname===null)
			$fname = $this->recDesc->getFieldNameByAlias($name,"frole");
			
		if (isset($this->fields[$fname]))
		{
			$fmethod = "to".$format; // ex: toDB, toHTML, toForm
			$f = & $this->fields[$fname];
			return $f->$fmethod($opts);
		}
		
		if ($cascade)
			return $this->recDesc->getProperty($name,$deft);
		else
			return $deft;
	}

	// export fields as properties
	function getProperties($cascade=false,$format='HTML')
	{
		$format = 'to'.$format;
		$props=array();
	 	foreach($this->fields as $f)
	 	{
	 		if ($f->is_transient())
	 			continue;
	 			
	 		$v=$f->$format(null,'');
              			$props[$f->getName()]=$v;

	 		// add alias (ie http url param name)
	 		if (($p=$f->desc->getProperty("alias",null,false))!==null)
	 			$props[$p]=$v;

	 		// add property name 
	 		if (($p=$f->desc->getProperty("isProp",null,false))!==null)
	 			$props[$p]=$v;

	 		// add property name in raw form
	 		if (($p=$f->desc->getProperty("isObject",null,false))!==null)
	 			$props[$p]=$f->toObject();
	 	}
	 	
	 	return $props;
	}

	
	// export fields as properties
	function saveAsProperties(&$props,$cascade=false,$format="HTML")
	{
		$format = 'to'.$format;
		
	 	foreach($this->fields as $f)
	 	{
	 		if ($f->is_transient())
	 			continue;

	 		$v=$f->$format(null,'');
 			$props->setProperty($f->getName(),$v);

	 		// add alias (ie http url param name)
	 		if ($p=$f->desc->getProperty("alias",null,false))
	 			$props->setProperty($p,$v);

	 		// add property name 
	 		if ($p=$f->desc->getProperty("isProp",null,false))
	 			$props->setProperty($p,$v);
	 	}
	}
	
	function getFieldIterator()
	{
		return new NxData_FieldIterator($this);
	}
	
	// get key for this record (as defined in data view)
	function getDataArray($format='HTML')
	{
		$a=array();
		$method="to$format";
		
		$fields=$this->fields; // copy because otherwise it breaks something in the array...bug ?
	 	foreach($fields as $f)
			$a[$f->getName()]=$f->$method();
		
		return $a;
	}
	
	// get key for this record (as defined in data view)
	function getUrlKey($all=false)
	{
		$paramKey = '';
		$fields=$this->fields; // copy because otherwise it breaks something in the array...bug ?
	 	foreach($fields as $f)
			if ($all || $f->isKey())
				$paramKey .= '&amp;'.$f->toUrlParam();
		
		return $paramKey;
	}
	
	// call each field with new key
	function onNew($oid,&$errorCB)
	{
		$this->lastInsertID=$oid;

	 	foreach($this->fields as $f)
	 	{
	 		$f->onNew($oid,$errorCB);
	 	}
	 	
		if (isset($this->aspect))
			return $this->aspect->onNew($isInsert,$errorCB,$this);
	}
                                                
	function onBeforeSave($isInsert, &$errorCB)
	{
		if (isset($this->aspect))
			return $this->aspect->onBeforeSave($isInsert,$errorCB,$this);
			
		return true;
	}
	
	function onSave($isInsert, &$errorCB)
	{
		if (isset($this->aspect))
			$this->aspect->onSave($isInsert,$errorCB,$this);

		foreach($this->fields as $f)
	 	{
	 		$f->onSave($isInsert,$errorCB);
	 	}
	}

	// gives a chance for fields to delete their resources (related files)
	function onDelete(&$errorCB)
	{
		if (isset($this->aspect))
			$this->aspect->onBeforeDelete($errorCB,$this);
			
	 	foreach($this->fields as $f)
	 	{
	 		$f->onDelete($errorCB);
	 	}
	}
	function onAfterDelete(&$errorCB)
	{
		if (isset($this->aspect))
			$this->aspect->onDelete($errorCB,$this);
	}
	
	//	returns a cloned version of this record. Each fields can clone its own resources (such as files).
	// Useful for getting different versions of records/content in CMS.
	function & clone_it(&$errorCB,$deepCopy=true)
	{
		// create and fillup new field array
	 	$record = new NxData_Record($this->recDesc);
		$fields = array();
	 	foreach($this->fields as $n=>$f)
	 	{
	 		$fields[$n] = $f->clone_it($record,$errorCB,$deepCopy);
	 	}
	 	
	 	// return a new record based on this new array
	 	$record->fields = $fields;
		$record->inMemory = true;

	 	return $record; 	
	}

	function saveLastInsertID() {}
	function restoreLastInsertID() {}

}

class NxData_FieldIterator
{
	 var $it;	
	 var $record;
	 var $i;

	 function __construct(&$record)
	 {
	 	$this->record = & $record;
	 	$this->it = array_keys($record->fields);
	 	$this->i = 0;
	 }
	 
	// field iterator (simple version: use array cursor)
	 function next()    	{ $this->i++; }
	 function reset()       { $this->i=0;}
	 function getCurrent() { return $this->record->getField($this->it[$this->i]); }
	 function isValid()	    { return $this->i < count($this->it); }
}


class NxData_Records
{
	var $recDesc;
	
	var $totalRecords;
	var $pageOffset;
	var $recordsPerPage;
	
	function __construct(&$recDesc)
	{
		$this->recDesc = & $recDesc;
	}

	function getRecDesc() { 
		return $this->recDesc; 
	}
	
	function getProperty($name,$deft=null,$cascade=true) {
		return $this->recDesc->getProperty($name,$deft,$cascade); 
	}	
	
	// function to overload. Returns number of records in collection
	function count() {return 0;}
	
	/**
	 * @return void
	 * @param $totalRecords total number of records (ie when no limit)
	 * @param $pageOffset offset of first record of the collection in the total list of records
	 * @param $recordsPerPage max number of records per page
	 * @param $pageAlias parameter name to use for selecting a page (get,post, cookie)
	 * @desc create the counters to be used in display of records by page (like in google)
	 */
	function setPageCounters($totalRecords,$pageOffset,$recordsPerPage,$pageAlias)
	{
		// get total number of pages
		if($recordsPerPage==0)
		{
            nxError('page counters are not defined');
            return false;
		}       
                
		$nbPages = (int)($totalRecords/$recordsPerPage);
		if ($nbPages*$recordsPerPage < $totalRecords)
			$nbPages++;
			
		$this->pages=array(
		"currentPageOffset" => $pageOffset,
		"recordsPerPage"=>$recordsPerPage,
		
		// parameter name to use (GET or POST)
		"pageAlias" => $pageAlias,

		// total records in all pages
		"totalRecords" => $totalRecords,
		
		// record window
		'firstRecord'=> $pageOffset*$recordsPerPage,
		'lastRecord' => ($pageOffset+1)*$recordsPerPage,
		
		// get total number of pages
		"nbPages" => $nbPages
		);		
		// get current page index
	}
	
	
	function getPageContext()
	{
		if (isset($this->pages))
			return $this->pages;
		else
			return null;
	}
	
	function isPaged() 
	{
		return isset($this->pages);
	}
	
	// clone a collection of records (will clone resources as well)
	function clone_it(&$errorCB,$deepCopy=true)
	{
		include_once('NxData_ArrayRecords.inc');
		
		// create a new array of records
		$records2 = new NxData_ArrayRecords(null,$this->getRecDesc());
		
		// iterate on current record collection and add cloned records to new collection
		for ($iterRec = $this->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$record =& $iterRec->getCurrent();
			$record2 =& $record->clone_it($errorCB,$deepCopy);
			$records2->addRecord($record2);
		}
		
		if (isset($this->pages))
			$records2->pages = $this->pages;
		
		return $records2;
	}

	// get records as array records object
	function getRecordArray()
	{
		// create a new array of records
		require_once(NX_LIB."NxData_ArrayRecords.inc");
		$records2 = new NxData_ArrayRecords(null,$this->getRecDesc());
		if (isset($this->pages))
			$records2->pages = $this->pages;
		
		// iterate on current record collection and add cloned records to new collection
		for ($iterRec = $this->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$records2->addRecord($iterRec->getCurrent());
		}
		
		return $records2;
	}
	
	function getDataArray($format='HTML') {
		$ar=array();
		
		// iterate on current record collection and add cloned records to new collection
		for ($iterRec = $this->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$rec=$iterRec->getCurrent();
			$ar[]=$rec->getDataArray($format);
		}
		return $ar;
	}
	
	function onDelete(&$errorCB)
	{
		for ($iterRec = $this->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$record =& $iterRec->getCurrent();
			$record->initAspect();
			$record->onDelete($errorCB);
		}
	}
	function onAfterDelete(&$errorCB)
	{
		for ($iterRec = $this->getRecordIterator(); $iterRec->isValid(); $iterRec->next())
		{
			$record =& $iterRec->getCurrent();
			$record->initAspect();
			$record->onAfterDelete($errorCB);
		}
	}

}
?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxPlug_DB extends NxProperties 
{
	var $app;
	var $db;
	
	function  NxPlug_DB(&$props)
	{
		$properties=array();
		parent::__construct($properties,$props);
		$this->props = &$props;
	}
	
	// Query API : functions called when building SQL queries
	function & getParam($name,$deft=null)
	{
		return $this->props->getParam($name,$deft);
	}

	// Query API : functions called when building SQL queries
	function & getValue($name,$deft=null)
	{
		return $this->props->getValue($name,$deft);
	}

	// Query API : functions called when building SQL queries
	function & matchList($fname,$vlist,$op,$comp)
	{
		$s='';
		$sep='';
		foreach (explode(',',$vlist) as $v)
		{
			$s.=" $sep $fname $comp '$v'";
			$sep=$op;
		}
		
		if ($s!='')
			$s=" ($s) ";
			
		return $s;
	}

	// returns the db object 
	function &getDB(&$errorCB)
	{
		if (!isset($this->db))
		{
			require_once(NX_LIB."ds/db/NxDB_Database.inc");
//			require_once(NX_LIB."NxData_Database.inc");
			$this->db = new NxDB_Database($this->props,$errorCB);
			if ($errorCB->isError())
				return false;
		}
		return $this->db;
	}
	
	// ds interface
	function deleteRecords($view, &$records, &$errorCB)
	{
		$this->executeDBQuery($view, 'deleteRecord', $errorCB);
	}
	
	function executeDBQuery($view, $queryType, &$errorCB, $tryAll=false)
	{
		$dbview = $this->props->getProperty("$view.db",null);
		if ($dbview==null)
			$dbview = $this->props->getProperty("db.".$view,null);

		// get the query
		$properties=array();
		// parent::__construct($properties,$this->props);
		nxLog("getting query:{$dbview}_{$queryType}",'MYSQL');
		require(NX_DB.$dbview."_".$queryType.".inc");
		$queries = $properties["query"];

		// execute the query
		$db = & $this->getDB($errorCB);
		if ($errorCB->isError())
			return false;

		if (is_array($queries))
		{
			foreach ($queries as $query)
			{
				$db->query($query);
				if ($errorCB->isError() && $tryAll == false)
					return false;
			}
			if ($errorCB->isError())
				return false;
		}
		else
		{
			$db->query($queries);
			if ($errorCB->isError())
				return false;
		}

		return true;
	}

	function & getRecords($view, &$errorCB)
	{
		$data = & $this->props->getPlugin("Data");

		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return $errorCB->fatal("can't read rec desc",__FILE__,__LINE__);
		
		// get recordset LIMIT
		$maxNbRecords = $this->props->getProperty("db.".$view. ".nbRecordPerPage",null);
		$indexFirst = $this->props->getProperty("db.".$view. ".indexFirstRecord",null);

		return $this->getDBRecords($recDesc,$errorCB, $indexFirst, $maxNbRecords);
	}

	function & getRecordCount($view, &$errorCB)
	{
		$data = & $this->props->getPlugin("Data");

		$recDesc = & $data->getRecDesc($view);
		if ($recDesc == null)
			return $errorCB->fatal("can't read rec desc",__FILE__,__LINE__);
		
		return $this->countRecords($recDesc,$errorCB);
	}
	
	// read records and returns an array of properties where keys are found in a field attribute given as param
	function & getRecordsAndRemap(&$view, &$errorCB, $remapWith="role",$fmethod="toHTML")
	{
			$errorCB = new NxErrorCB();
			$records =& $this->getRecords($view,$errorCB);
			if ($errorCB->isError())
				return null;
				
			$recs = array();
			
			// loop on all records (pages) and get link parameter, text and tooltip
		//	if ($records!=null && $records->count() > 0)
			if ($records!=null)
				for ($recIt = $records->getRecordIterator(),$i=0; $recIt->isValid(); $recIt->next(),$i++)
				{
					$record = & $recIt->getCurrent();
					$paramKey="";
					for ($fieldIt = $record->getFieldIterator(); $fieldIt->isValid(); $fieldIt->next())
					{
						$field = & $fieldIt->getCurrent();
						$role = $field->desc->getProperty($remapWith,null,false);
						if ($role !== null)
						{
							$recs[$i][$role]=$field->$fmethod();
						}
						
						if ($field->isKey())
							$paramKey .= "&amp;".$field->toUrlParam();
					}
					$recs[$i]["keyParam"]=$paramKey;
				}
				
			return $recs;
	}
	
	// ============ DATABASE ============
	function & getDBRecords(&$recDesc, &$errorCB, $pageOffset=null, $maxNbRecords=null)
	{	
		$dbview = $recDesc->getViewName();

		// get the query
		$queryType = "select";
		
		if ($pageOffset!= null && $maxNbRecords!=null)
			$recordOffset = $pageOffset*$maxNbRecords;
		else
			$recordOffset = 0;
			
		$properties=array("recordOffset"=>$recordOffset, "nbRecordsPerPage"=>$maxNbRecords);
		// parent::__construct($properties,$this->props);
		nxLog("getting query:{$dbview}_selectRecord",'MYSQL');
		require(NX_DB.$dbview."_selectRecord.inc");
		$query = $properties["query"];

		// get the db
		$db = & $this->getDB($errorCB);
		if ($errorCB->isError())
			return false;

		$db->query(preg_replace(array('/ AND 0=0/','/LIMIT 0,-1/'),'',$query));
		if ($errorCB->isError())
		{
			return null;
		}
				
		$records =& $db->getRecords($recDesc);
		
		// when nb of records is limited, get total number of records in DB (ie query w/o limit)
		if (isset($properties["pageOffset"]))
		{
			$count = $this->countRecords($recDesc, $errorCB);
			if ($errorCB->isError())
				nxError('DB Query failed: cant count nb of records for view '.$recDesc->getViewName(),"PLUG_DB");
			else
				// set page counters in the record collection
				$records->setPageCounters($count,$properties["pageOffset"],$properties["nbRecords"],$properties["pageAlias"]);
		}
		
		return $records;
	}

	function & countRecords(&$recDesc, &$errorCB)
	{	
		$dbview = $recDesc->getViewName();
// echo "<br/>view is: ".$dbview;

		// get the query
		$queryType = "count";

		//nxLog("getting query:{$dbview}_countRecord",'MYSQL');
		require(NX_DB.$dbview."_countRecord.inc");
		$query = $properties["query"];

		// get db object
		$db = & $this->getDB($errorCB);
		if ($errorCB->isError())
			return false;

		//  trace($query);
		$resultSet=$db->query($query);
		if ($errorCB->isError())
		{
			 // trace($errorCB->getErrorMsg()); // debug
			return null;
		}
		
		// get count of all records in special field "nbrecords"
		$fields=$db->queryFields($resultSet);
		return $fields["nbrecords"];
	}
	
	// add/modify records with view name
	function putRecords($view, &$errorCB, $records,$dbOp='insert')
	{
		$data = & $this->props->getPlugin("Data");
		$recDesc =&$data->getRecDesc($view);
		return $this->addDBRecords($recDesc,$errorCB,$records,$dbOp);
	}
		
	// add/modify records with recDesc
	function addDBRecords(&$recDesc, &$errorCB, $records,$dbOp="insert")
	{
		//$data = & $this->props->getPlugin("Data");
		$dbview = $recDesc->getViewName();

		// get db
		// get db object
		$db = & $this->getDB($errorCB);
		if ($errorCB->isError())
			return false;

		// store records in the record set
		for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
		{
			$record = & $recIt->getCurrent();

			// get the query
			$properties=array();
			nxLog("getting query:{$dbview}_{$dbOp}",'MYSQL');
			require(NX_DB.$dbview."_".$dbOp."Record.inc");
			
			// execute it
			$query = $properties["query"];
			
			nxLog("addDBRecords query:".$query,"DB");
			$db->query($query);
		}
		return true;
	}	

	// add/modify a record (to/from same data source)
	function addDBRecord(&$record, &$errorCB, $dbOp="insert")
	{
		$recDesc = & $record->getRecDesc();
		$dbview = $recDesc->getViewName();

		// get db
		$db = & $this->getDB($errorCB);
		if ($errorCB->isError())
			return false;

		// get the query
		$properties=array();
		nxLog("getting query:{$dbview}_{$dbOp}Record",'MYSQL');
		require(NX_DB.$dbview."_".$dbOp."Record.inc");
		
		// execute it
		$query = $properties["query"];
		$db->query($query);
	}	
	
	// private functions
	function getWhereClause($qfname,$values,$relation="LIKE",$sep="OR",$prefixVal="%",$suffixVal="%",$default=true)
	{
		if ($values===null || $values==='')
			if ($default)
					 return '0=0';	 
			else
					 return '0=1';	 
		
		if (!is_array($values))
			$values=explode('|',$values);

		$clause = "(";
		$_sep="";
		foreach ($values as $v)
		{
			$clause.= $_sep."$qfname $relation '$prefixVal".$v."$suffixVal'";
			$_sep=" $sep ";
		}
		$clause.= ")";
			
		return $clause;
	}
		
}

?>
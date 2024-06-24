<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/* 
 * Datasource: manage data coming from HTTP POST/GET/COOKIE variables.
 * Author: Franck Yvetot
 * Copyright NxFrontier
 */

/*
usage:

// get field container
var $params = array(
	"db.vendor" => "MySQL",
	"db.user" => "john",
	"db.password" => "axcv",
	"db.host" => "localhost",
	"db.database" => "database1",
	"query" => "select * from users"
	);
var $db = new NxData_Database($params);

// loop on records
for (var $recIt = $db->getRecordIterator();
	recIt->isValid();
	recIt->next())
	{
		// loop on fields
		for (var $fieldIt = $recIt->getFieldIterator();
			$fieldIt->isValid();
			$fieldIt->next())
			{
				// use field value
				print $fieldIt->getValue();				
			}
		
	}

*/

// ============== PREDEFINED CALLBACKS ====================
// default error handler
// require_once(NX_LIB."NxApp_ErrorCB.inc");
require_once(NX_LIB . "NxDB_Record.inc");

// ============== DATABASE HANDLER ====================
// todo: get rid of this plugin and replace by ds...
if (!class_exists('NxDB_MySQL'))
{
	class NxDB_MySQL
	{
		function __construct() {}
		function connect($host, $user, $pass) {	return mysql_connect ($host, $user, $pass); }
		function close($linkid)               { return mysql_close($linkid); }
		function free_result(&$qresult)       { 
			if(!is_resource($qresult))
					nxError('free_result:bad result set','MSQL');
			mysql_free_result($qresult); }
		function query($querystr, $linkid)    { 
			nxLog("SQL:QUERY[$querystr]","MYSQL");
			return mysql_query($querystr, $linkid); 
	//		nxLog('result:'.mysql_info(),'MYSQL');
		}
		function select_db($dbname, $linkid)  {	return @mysql_select_db ($dbname, $linkid); }
		function num_rows (&$result)		  { 
			if(!is_resource($result))
					nxError('num_rows:bad result set','MYSQL');
			return mysql_num_rows($result); 
		}
		function & fetch_array(&$result,$result_type) { 
			if(!is_resource($result))
					nxError('fetch_array:bad result set','MYSQL');
			return mysql_fetch_array($result,$result_type);	
		}
		function reset_results(&$result)	  {
			nxLog('reset_result:'.$result,'MYSQL');
			if(!is_resource($result))
					nxError('reset_results:bad result set','MYSQL');
			nxLog('num_rows:'.mysql_num_rows($result),'MYSQL');
			@mysql_data_seek($result, 0);
		}
		function getLastInsertID($linkid,&$id) { 
			if (mysql_affected_rows($linkid)<=0) {
				nxError('getLastInsertID:cant get auto ID as previous insert failed','MYSQL');
				return false; 
			}
			$id=mysql_insert_id($linkid);
			return true;
		}
		
		function isTableExist($dbLink, $database, $tableName)
		{
			$tables = array();
			$tablesResult = mysql_list_tables($database, $dbLink);
			while ($row = mysql_fetch_row($tablesResult)) 
				$tables[] = $row[0];
			return(in_array(strtolower($tableName), $tables));
		}
		
		function & error()					  { return mysql_error(); }
	}
}

//================= DATABASE OPERATIONS ================

// database Wrapper
class NxData_Database 
{
	var $db_linkid = NULL; 
	var $db_qresult = NULL; 
	var $errorCB = NULL;
	var $_db;
	
	function __construct(&$propDesc, &$errorCB) 
	{ 
		$this->setErrorCB($errorCB);

		$dbVendor = $propDesc->getProperty("db.vendor",null,true);
		if (isset($dbVendor))
		{
			if ($dbVendor == "MySQL")
			{
				$this->_db = new NxDB_MySQL();
			}
			else
			{
				die("no database type specified");
			}
		}

		$host = $propDesc->getProperty("db.host","localhost",true);
		if( ($passwd 	= $propDesc->getProperty("db.password",null,true))!= null &&
			($user 		= $propDesc->getProperty("db.user",null,true))!= null &&
			($database	= $propDesc->getProperty("db.database",null,true))!= null)
		{
			$this->init ($host, $user, $passwd, $database);
		}
		else
		{
			$errorCB->addError("missing database admin parameters");
		}
	}   

	function init($host, $user, $pass, $dbname = "") 
	{ 
		$this->db_linkid = $this->_db->connect ($host, $user, $pass); 

		if ($this->db_linkid == NULL || 
			($dbname != "" && !$this->selectDB($dbname)))
		{
			nxError("error connecting to DB on [".$host."] db=[".$dbname."]");
			$this->error("error connecting to DB on [".$host."] db=[".$dbname."]");
			return false;
		}
		nxLog("connected to db:$dbname@$host",'MYSQL');
		return true;
	}   

	function isConnected()
	{
		return ($this->db_linkid != false);
	}
	
	function close() 
	{ 
		if (is_resource($this->db_qresult))
			@$this->_db->free_result($this->db_qresult); 
			
		if (is_resource($this->db_linkid))
			return $this->_db->close ($this->db_linkid); 
			
		return true;
	}   

	function createDB($dbname)
	{
		$querystr = "CREATE DATABASE ".$dbname;
		if (!$this->_db->query ($querystr, $this->db_linkid))
		  return $this->error("failed DB creation :["+$querystr+"]"); // failed
		else
			return true;
	}

	function deleteDB($dbname)
	{
		$querystr = "DROP DATABASE ".$dbname;
		if (!$this->_db->query ($querystr, $this->db_linkid) )
		  return $this->error("failed DB deletion :["+$querystr+"]"); // failed
		else
			return true;
	}

	function selectDB($dbname) 
	{ 
		return @$this->_db->select_db($dbname, $this->db_linkid);
	}    

	function query($querystr) 
	{ 
		$this->db_qresult = $this->_db->query ($querystr, $this->db_linkid); 
		if (!$this->db_qresult)
		  return $this->error("failed query:[".$querystr."]"); // failed

		if (is_resource($this->db_qresult) && 
			$this->_db->num_rows ($this->db_qresult) == 0)
			return false;
		else
			return $this->db_qresult;
	}
	
	function getLastInsertID(&$id) {
		if (is_resource($this->db_linkid))
			return $this->_db->getLastInsertID($this->db_linkid,$id); 
			
		return false;
	}

	function numRows($results = NULL)
	{
		if (!isset($results))
			$results = $this->db_qresult;

		if (is_resource($results))
			return $this->_db->num_rows ($results);
		else
			return 0;
	}

	function resetRecords(&$results)
	{
		$this->_db->reset_results($results, 0);
	}
	
	function queryFields(&$results)
	{
		if (!isset($results))
			$results = & $this->db_qresult;

		return $this->_db->fetch_array($results,MYSQL_ASSOC);
	}   
	
	// Meta data
	function listDBs()
	{
		/*
		$this->db_qresult = $result = $this->_db->query ($querystr, $this->db_linkid); 
		if (!$result)
		  return $this->error("failed query:["+$querystr+"]"); // failed

		return queryResults($this->db_qresult,$cb);
		*/
	}

	// setting error CB
	function setErrorCB(& $err)
	{
		$this->errorCB = & $err;
	}

	// logging error
	function error($msg)
	{
		$this->errorCB->addError("database error: try later or contact the site web master",$msg.":".$this->_db->error());
		return false;
	}

	//=============== ITERATOR API ========================
	function isOk()
    {
		return $this->isConnected();
    }

	function &getRecords(&$recDesc) 
    {
    	$this->recDesc = & $recDesc;
        return new NxDB_Records($this,$recDesc,$this->db_qresult);
    }

	function &getRecordIterator() 
    {
        return new NxDB_RecordIterator($this,$this->db_qresult);
    }
}

?>

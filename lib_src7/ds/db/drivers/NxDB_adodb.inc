<?php 
require_once(NX_LIB . "NxDB_Record.inc");

class NxDB_adodb
{
	var $db;
	var $db_qresult = NULL;
	
	function __construct() {
		include (NX_LIB . "adodb/adodb.inc.php");
		$this->db = NewADOConnection ( 'mysql' );
		$this->db->SetFetchMode(ADODB_FETCH_ASSOC);
		$this->db->debug = false;
		$ADODB_COUNTRECS = true;
	}
	function connect($host, $user, $pass) {	return $this->db->connect ($host, $user, $pass); }
	function close($linkid)               { 
		if (is_resource($linkid))
			return $this->db->Close ();
		return false; 
	}
	function free_result(&$qresult)       { 
		if($this->isRSOk($qresult))
			$this->db->_close (); 
		return true;
	}
	
	function query($querystr, $linkid)    { 
		nxLog("SQL:QUERY[$querystr]","MYSQL");
		
		return $this->db->Execute($querystr);
	}
	
	function start_trans($linkid)    { 
		nxLog("SQL:START TRANS","MYSQL");
		return $this->db->BeginTrans(); 
	}
	function commit($linkid)    { 
		nxLog("SQL:COMMIT","MYSQL");
		return $this->db->CommitTrans(); 
	}
	function rollback($linkid)    { 
		nxLog("SQL:ROLLBACK","MYSQL");
		return $this->db->RollbackTrans(); 
	}
	function select_db($dbname, $linkid)  {	return $this->db->SelectDB ( $dbname ); }
	function num_rows ($result)		  { 
		if(!isRSOk($result))
		nxLog('num_rows:bad result set','MYSQL');
		return $results->FieldCount();
	}
	function & fetch_array(&$result,$result_type) { 
		if(!isRSOk($result))
		nxLog('fetch_array:bad result set','MYSQL');
		$res= $results->GetRowAssoc(false);
		return $res;
			
	}
	function reset_results(&$result)	  {
		nxLog('reset_result:'.$result,'MYSQL');
		if(!isRSOk($result))
				nxError('reset_results:bad result set','MYSQL');
		$num_rows=$results->FieldCount();
		nxLog('num_rows:'.$num_rows,'MYSQL');
		if ($num_rows>0)
			$results->_seek ( $row= 0 );
	}
	function getLastInsertID($linkid,&$id) { 
		if ($this->db->_affectedrows()<=0) {
			nxLog('getLastInsertID:cant get auto ID as previous insert failed','MYSQL');
			return false; 
		}
		$id=$this->db->_insertid();
		return true;
	}
	
	function isTableExist($dbLink, $database, $tableName)
	{
		$tables = array();
		$tables = $this->db->MetaTables('TABLES');
		return(in_array(strtolower($tableName), $tables));
	}
	
	function & errno()					  { return $this->db->ErrorMsg(); }		
	function & error()					  { return $this->db->ErrorNo(); }
	function & isRSOk($rs) {
		return is_array($rs->fields);
	} 
}
?>
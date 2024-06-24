<?php 
class NxDB_MySQL
{
	function __construct() {}
	function connect($host, $user, $pass) {
		return mysql_connect ($host, $user, $pass); 
	}
	function close($linkid)               { 
		if (is_resource($linkid))
			return mysql_close($linkid);
		return false; 
	}
	function free_result(&$qresult)       { 
		if(is_resource($qresult))
			mysql_free_result($qresult); 
		return true;
	}
	
	function query($querystr, $linkid)    { 
		nxLog("SQL:QUERY[$querystr]","MYSQL");
		return mysql_query($querystr, $linkid); 
//		nxLog('result:'.mysql_info(),'MYSQL');
	}
	
	function start_trans($linkid)    { 
		nxLog("SQL:START TRANS","MYSQL");
		return mysql_query('BEGIN', $linkid); 
	}
	function commit($linkid)    { 
		nxLog("SQL:COMMIT","MYSQL");
		return mysql_query('COMMIT', $linkid); 
	}
	function rollback($linkid)    { 
		nxLog("SQL:ROLLBACK","MYSQL");
		return mysql_query('ROLLBACK', $linkid); 
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
		$num_rows=mysql_num_rows($result);
		nxLog('num_rows:'.$num_rows,'MYSQL');
		if ($num_rows>0)
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
	
	function & errno()					  { return mysql_errno(); }		
	function & error()					  { return mysql_error(); }
}
?>
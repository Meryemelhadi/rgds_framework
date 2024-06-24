<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/


// ============== DATABASE HANDLER ====================
// todo: write other handlers for oracle, odbc etc.
class NxDB_MySQL_driver
{
	function __construct() {}
	function init($properties) {
		if( ($host 		= @$properties['MySQL.host'])!= null &&
			($passwd 	= @$properties['MySQL.password'])!== null &&
			($user 		= @$properties['MySQL.user'])!= null &&
			($database	= @$properties['MySQL.database'])!= null)
		{
			$this->db_linkid = $this->connect ($host, $user, $passwd); 
			if ($this->db_linkid == NULL || 
				($database != "" && !$this->select_db($database, $this->db_linkid)))
			{
				nxError("error connecting to DB on [".$host."] db=[".$database."]");
				$this->error("error connecting to DB on [".$host."] db=[".$database."]");
				return false;
			}

			$tz = date_default_timezone_get();
			if(!$tz)
				$tz = $this->getTimeZone();

			global $NX_CHARSET;
			if($NX_CHARSET=='utf-8' && defined('NX_DB_UTF8') && NX_DB_UTF8==true)
			{
				mysql_set_charset('utf8', $this->db_linkid);
				mysql_query("SET character_set_results = 'utf8', character_set_client = 'utf8', character_set_connection = 'utf8', character_set_database = 'utf8', character_set_server = 'utf8', time_zone='$tz'");
			}
			else
				mysql_query("time_zone='$tz'");

			nxLog("connected to db:$database@$host with resource {$this->db_linkid} [TZ = $tz]",'MYSQL');
			return $this->db_linkid;
		}
	}

	function getTimeZone() {
		$now = new DateTime();
		$mins = $now->getOffset() / 60;
		$sgn = ($mins < 0 ? -1 : 1);
		$mins = abs($mins);
		$hrs = floor($mins / 60);
		$mins -= $hrs * 60;
		$offset = sprintf('%+d:%02d', $hrs*$sgn, $mins);

		return $offset;

		// return "SET time_zone='$offset'";
	}

	function connect($host, $user, $pass) {	return mysql_connect ($host, $user, $pass,true); }
	function is_connected($db_linkid)
	{
		return ($db_linkid != false);
	}

	function close($linkid)               { 
		if (is_resource($linkid))
			return mysql_close($linkid);
		return false; 
	}
	function real_escape_string($s,$linkid) {
		return mysql_real_escape_string($s);
	}
	function free_result(&$qresult)       { 
		if(is_resource($qresult))
			mysql_free_result($qresult); 
		return true;
	}
	function query($querystr, $linkid)    { 
		nxLog("SQL:QUERY[$querystr] / $linkid","MYSQL");

		$res = mysql_query($querystr, $linkid); 
		if (is_resource($res))
			return $res;
		return null;

//		return mysql_query($querystr, $linkid); 
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
	function fetch_array(&$result,$result_type=MYSQL_ASSOC) { 
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
	
	function errno($r=null)					  { return $r ? mysql_errno($r) : mysql_errno(); }		
	function error($r=null)					  { return $r ? mysql_error($r) : mysql_error(); }
}

?>
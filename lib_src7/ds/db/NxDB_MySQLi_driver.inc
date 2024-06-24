<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/


// ============== DATABASE HANDLER ====================
// todo: write other handlers for oracle, odbc etc.
class NxDB_MySQLi_driver
{
	function __construct() {}
	function init($properties) {
		if( ($host 		= @$properties['MySQL.host'])!= null &&
			($passwd 	= @$properties['MySQL.password'])!== null &&
			($user 		= @$properties['MySQL.user'])!= null &&
			($database	= @$properties['MySQL.database'])!= null)
		{
			$this->db_linkid = $this->connect($host, $user, $passwd,$database); 

			if ($this->db_linkid == NULL)
			{
				nxError("error connecting to DB on [".$host."] db=[".$database."] ".mysqli_connect_error());
				// $this->error("error connecting to DB on [".$host."] db=[".$database."]");
				return false;
			}

			$tz = date_default_timezone_get();
			if(!$tz)
				$tz = $this->getTimeZone();

			global $NX_CHARSET;
			if($NX_CHARSET=='utf-8' && defined('NX_DB_UTF8') && NX_DB_UTF8==true)
			{
				mysqli_set_charset($this->db_linkid,'utf8');

				mysqli_query($this->db_linkid,"SET character_set_results = 'utf8', character_set_client = 'utf8', character_set_connection = 'utf8', character_set_database = 'utf8', character_set_server = 'utf8', time_zone='$tz'");
			}
			else
				mysqli_query($this->db_linkid,"time_zone='$tz'");

			nxLog("connected to db:$database@$host [TZ = $tz]",'MYSQLi');
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

	function connect($host, $user, $pass,$database=null) {	
		return mysqli_connect($host, $user, $pass,$database); 
	}

	function is_connected($db_linkid)
	{
		return ($db_linkid != false);
	}

	function real_escape_string($s,$linkid) {
		return mysqli_real_escape_string($linkid,$s);
	}

	function close($linkid)               { 
		if ($linkid)
			return mysqli_close($linkid);
		return false; 
	}
	function free_result($qresult)       { 
		if($qresult)
			mysqli_free_result($qresult); 
		return true;
	}
	function query($querystr, $linkid)    { 
		nxLog("SQL:QUERY[$querystr]","MYSQLi");
	
		return mysqli_query($linkid,$querystr); 
//		nxLog('result:'.mysql_info(),'MYSQLi');
	}
	
	function start_trans($linkid)    { 
		nxLog("SQL:START TRANS","MYSQLi");
		return mysqli_begin_transaction($linkid); 
	}
	function commit($linkid)    { 
		nxLog("SQL:COMMIT","MYSQLi");
		return mysqli_commit($linkid); 
	}
	function rollback($linkid)    { 
		nxLog("SQL:ROLLBACK","MYSQLi");
		return mysqli_rollback($linkid); 
	}
	
	function select_db($dbname, $linkid)  {	
		return mysqli_select_db ($linkid,$dbname); 
	}
	
	function num_rows ($result)		  { 
		if(!$result)
		{
			nxError('num_rows:bad result set','MYSQLi');
			return 0;
		}

		return mysqli_num_rows($result); 
	}

	function fetch_array(&$result,$result_type=MYSQLI_ASSOC) { 
		if($result)
			return mysqli_fetch_array($result,$result_type);	

		nxError('fetch_array:bad result set','MYSQLi');
		return null;
	}

	function reset_results(&$result)	  {
		nxLog('reset_result','MYSQLi');
		if(!$result)
		{
			nxError('reset_results:bad result set','MYSQLi');
			return false;
		}

		$num_rows=mysqli_num_rows($result);
		nxLog('num_rows:'.$num_rows,'MYSQLi');
		if ($num_rows>0)
			mysqli_data_seek($result, 0);
	}
	function getLastInsertID($linkid,&$id) { 
		if (mysqli_affected_rows($linkid)<=0) {
			nxError('getLastInsertID:cant get auto ID as previous insert failed','MYSQLi');
			return false; 
		}
		$id=mysqli_insert_id($linkid);
		return true;
	}
	
	function isTableExist($dbLink, $database, $tableName)
	{
		$tables = array();
		$tablesResult = mysqli_query($dbLink,'SHOW TABLES FROM '.$database);
		while ($row = mysqli_fetch_row($tablesResult)) 
			$tables[] = $row[0];
		return(in_array(strtolower($tableName), $tables));
	}
	
	function errno($r=null)					  { return $r ? mysqli_errno($r) : mysqli_errno($this->db_linkid); }		
	function error($r=null)					  { return $r ? mysqli_error($r) : mysqli_error($this->db_linkid); }
}

?>
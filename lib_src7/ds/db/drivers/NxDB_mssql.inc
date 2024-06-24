<?php 
class NxDB_mssql 
{
	function __construct() {}
	function connect($host, $user, $pass) {	return mssql_connect ($host, $user, $pass); }
	function close($linkid)               { 
		if (is_resource($linkid))
			return mssql_close($linkid);
		return false; 
	}
	function free_result(&$qresult)       { 
		if(is_resource($qresult))
			mssql_free_result($qresult); 
		return true;
	}
	function & mssql_now($sql){
       //$this->sysDate = 'convert(datetime,convert(char,GetDate(),102),102)';
	    $sql = str_replace('NOW()', 'GetDate()', $sql);
	    //$sql = str_replace('NOW()', $this->sysDate, $sql);    
        return $sql;
    
}
	function & mssql_oid($sql){
if (strpos(strtolower($sql), 'insert') === false){
        return $sql;
    }
    else
    {
	    $sql = str_replace('_oid,', '', $sql);
	    $sql = str_replace("VALUES('',", "VALUES(", $sql);
	    
        return $sql;
    }
}
	function & mssql_limit_query($sql){
    if (strpos(strtolower($sql), ' limit ') === false){
        return $sql;
    }

    //Extract out the limit and order by clauses.
    if (preg_match('/LIMIT\s*(([0-9]+),)?\s*([0-9]+)/', $sql, $matches) ){
        $skip = $matches[2];
        $n = $matches[3];

        $start = strpos(strtolower($sql), ' order by ');
        if ($start !== false){
            $orderby = array();
            $thisorder = array();

            $order = trim(substr($sql, $start+strlen(' order by ')));
            $pos = strpos($order, ' ');

            while ($pos !== false){
                $word = substr($order, 0, $pos);
                if ($word{strlen($word)-1} == ','){
                    if (strtolower($word) == 'asc' || strtolower($word) == 'desc'){
                        $thisorder[1] = $word;
                        $orderby[] = $thisorder;
                        $thisorder = array();
                    }
                    else {
                        $word = substr($word, 0, -1);
                        $thisorder[0] = $word;
                        $thisorder[1] = 'ASC';
                        $orderby[] = $thisorder;
                        $thisorder = array();
                    }
                }
                else if (strtolower($word) == 'asc' || strtolower($word) == 'desc'){
                    $thisorder[1] = $word;
                    $orderby[] = $thisorder;
                    $thisorder = array();
                }
                else if (strtolower($word) == 'having' || strtolower($word) == 'limit'){
                    if (!empty($thisorder[0])){
                        if (empty($thisorder[1])){
                            $thisorder[1] = 'asc';
                        }
                        $orderby[] = $thisorder;
                    }
                    break;
                }
                else {
                    $thisorder[0] = $word;
                }
                $order = trim(substr($order, $pos));
                $pos = strpos($order, ' ');
            }
        }

        $base_sql = preg_replace('/LIMIT\s*(([0-9]+),)?\s*([0-9]+)/', '', $sql);
        
        if (empty($skip)){
            $sql = preg_replace('/^\s*SELECT\s/', 'SELECT TOP '.$n.' ', $base_sql);
            return $sql;
        }
        else {
            $outer_order = '';
            $inner_order = '';
            foreach ($orderby as $val){
                $outer_order .= $val[0].' '.$val[1].', ';
                $inner_order .= $val[0].' '.invertOrder($val[1]).', ';
            }

            if (empty($outer_order)){
                $outer_order = 1;
            }
            else {
                $outer_order = substr($outer_order, 0, -2);
            }

            if (empty($inner_order)){
                $inner_order = 1;
            }
            else {
                $inner_order = substr($inner_order, 0, -2);
            }

            $sql  = "SELECT * FROM (\n";
            $sql .= "   SELECT TOP ".$n." * FROM (\n";
            $sql .= "       ".preg_replace('/^\s*SELECT\s/', 'SELECT TOP '.($n+$skip).' ', $base_sql)."\n";
            $sql .= "   ) As table_inner ORDER BY ".$inner_order." \n";
            $sql .= ") As table_outer ORDER BY ".$outer_order."\n";
            return $sql;
        }
    }
    return false;
}

	function & mssql_identity($sql)
	{
   		define('TABLE_REGEX','([]0-9a-z_\:\"\`\.\@\[-]*)');
		preg_match("/INTO\s+".TABLE_REGEX."/is", $sql, $tableName);
		$tableName = $tableName[1];
		$sql = preg_replace('/^\s*INSERT\s/', 'SET IDENTITY_INSERT '.$tableName.' ON INSERT ', $sql);
		return $sql;
	
	}

	function query($querystr, $linkid)    { 
		$query = $this->mssql_limit_query($querystr);
		$query = $this->mssql_now($query);
		$query = $this->mssql_oid($query);
		
		//$query = $this->mssql_identity($query);
		
		nxLog("SQL:QUERY[$query]","mssql");
		return mssql_query($query, $linkid); 
		nxLog('result:'.mssql_info(),'mssql');
	}
	
	
	function start_trans($linkid)    { 
		nxLog("SQL:START TRANS","mssql");
		return mssql_query('BEGIN', $linkid); 
	}
	function commit($linkid)    { 
		nxLog("SQL:COMMIT","mssql");
		return mssql_query('COMMIT', $linkid); 
	}
	function rollback($linkid)    { 
		nxLog("SQL:ROLLBACK","mssql");
		return mssql_query('ROLLBACK', $linkid); 
	}
	function select_db($dbname, $linkid)  {	return @mssql_select_db ($dbname, $linkid); }
	function num_rows (&$result)		  { 
		if(!is_resource($result))
				nxError('num_rows:bad result set','mssql');
		return mssql_num_rows($result); 
	}
	function & fetch_array(&$result,$result_type) { 
		if(!is_resource($result))
				nxError('fetch_array:bad result set','mssql');
		return mssql_fetch_array($result,$result_type);	
	}
	function reset_results(&$result)	  {
		nxLog('reset_result:'.$result,'mssql');
		if(!is_resource($result))
				nxError('reset_results:bad result set','mssql');
		$num_rows=mssql_num_rows($result);
		nxLog('num_rows:'.$num_rows,'mssql');
		if ($num_rows>0)
			@mssql_data_seek($result, 0);
	}
	function getLastInsertID($linkid,&$id) { 
		if (mssql_affected_rows($linkid)<=0) {
			nxError('getLastInsertID:cant get auto ID as previous insert failed','mssql');
			return false; 
		}
		$id=mssql_insert_id($linkid);
		return true;
	}
	
	function isTableExist($dbLink, $database, $tableName)
	{
		$tables = array();
		$tablesResult = mssql_list_tables($database, $dbLink);
		while ($row = mssql_fetch_row($tablesResult)) 
			$tables[] = $row[0];
		return(in_array(strtolower($tableName), $tables));
	}
	
	function & errno()					  { return mssql_get_last_message(); }		
	function & error()					  { return mssql_get_last_message(); }
}
?>
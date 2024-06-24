<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/


require_once (NX_LIB . "NxDB_Record.inc");

// ============== PREDEFINED CALLBACKS ====================
// default error handler


// ============== DATABASE HANDLER ====================
// todo: write other handlers for oracle, odbc etc.

//================= DATABASE OPERATIONS ================


// database Wrapper
class NxDB_Database {
	var $db_linkid = NULL;
	var $db_qresult = NULL;
	var $errorCB = NULL;
	var $_db;
	var $props;
	
	function __construct(&$propDesc, &$errorCB, $connexion = '') {
		$this->setErrorCB ( $errorCB );
		$this->props = &$propDesc;
		
		// get connexion descriptor
		//        $cnxName=$propDesc->getProperty('db.'.$connexion,'MySQL.'.$connexion);
		

		if ($connexion && $connexion != 'default')
			$cnxName = 'MySQL.' . $connexion;
		else
			$cnxName = 'MySQL';
			
		// load connexion parameter
		include (NX_CONF . 'ds/' . $cnxName . NX_HOST_SWITCH . '.inc');
		include (NX_LIB . "adodb/adodb.inc.php");
		$this->_db = NewADOConnection ( 'mysql' );
		$this->_db->SetFetchMode(ADODB_FETCH_ASSOC);
		
		$this->_db->debug = false;
		//$this->_db = new NxDB_mssql();		
		if (($host = @$properties ['MySQL.host']) != null && ($passwd = @$properties ['MySQL.password']) != null && ($user = @$properties ['MySQL.user']) != null && ($database = @$properties ['MySQL.database']) != null) {
			$this->init ( $host, $user, $passwd, $database );
		} else {
			$errorCB->addError ( "missing database admin parameters for " . $cnxName );
		}
	}
	
	/* DATABASE HANDLER API
		TODO: put these function in pecific DB handler?
	 */
	function init($host, $user, $pass, $dbname = "") {
		$this->db_linkid = $this->_db->connect ( $host, $user, $pass );
		
		if ($this->db_linkid == NULL || ($dbname != "" && ! $this->selectDB ( $dbname ))) {
			nxError ( "error connecting to DB on [" . $host . "] db=[" . $dbname . "]" );
			$this->error ( "error connecting to DB on [" . $host . "] db=[" . $dbname . "]" );
			return false;
		}
		nxLog ( "connected to db:$dbname@$host", 'mssql' );
		$this->db_linkid=$this->_db->_connectionID;
		return true;
	}
	
	function isConnected() {
		return ($this->db_linkid != false);
	}
	
	function close() {
		return $this->_db->Close ();
	}
	
	function createDB($dbname) {
		$querystr = "CREATE DATABASE " . $dbname;
		if (! $this->_db->Execute ( $querystr ))
			return $this->error ( "failed DB creation :[" + $querystr + "]" ); // failed
		else
			return true;
	}
	
	function deleteDB($dbname) {
		$querystr = "DROP DATABASE " . $dbname;
		if (! $this->_db->Execute ( $querystr ))
			return $this->error ( "failed DB deletion :[" + $querystr + "]" ); // failed
		else
			return true;
	}
	
	function selectDB($dbname) {
		return $this->_db->SelectDB ( $dbname );
	}
	
	function isRSOk($rs) {
		return is_array($rs->fields);
	}
	
	// executes a query and returns DB result as resource
	// if table doesnt exists, try to create it by looking in database views
	function query($querystr, $recover = true, $handler = null) {
					
		nxLog("SQL:QUERY[$querystr]","MYSQL");
		$linkid=$this->_db->_connectionID;
		//do not require a recordset to be returned, then you can call _query
		//return Either the resultid, true or false are returned by _query(). 
		$this->db_qresult = $this->_db->Execute ( $querystr);
		//$this->db_qresult = $this->_db->Execute ( $querystr);
		
		if (! $this->db_qresult) {
			// try to recover from error: fix and try again
			if ($recover && $this->onErrorRecover ( $querystr, null, $handler )) {
				nxLog ( "Recovered from DB error, try again..", 'MYSQL' );
				return $this->query ( $querystr, false );
			} else
				return $this->error ( "failed query:[" . $querystr . "]" ); // failed
		}
		if ($this->isRSOk($this->db_qresult) && $this->db_qresult->NumRows() == 0)
			return false;
		else {
			
			/*
			nxLog ( 'result:' . $this->db_qresult->fields [1], 'mysql' );
		    nxLog ( 'num_rows:' . $this->db_qresult->numRows(), 'mysql' );
			nxLog ( " sql:" . $querystr, "mysql" );
			nxLog ( " _queryID:" . $this->_db->_queryID, "mysql" );
		//$this->db_qresult=$this->_db->_queryID;
			*/
			//return $this->_db->_queryID;
			return $this->db_qresult;
		}
	
	}
	
	// executes a query and returns results asociative array
	function queryWithData($querystr, $recover = true, $handler = null) {
		
		nxLog("SQL:QUERY[$querystr]","MYSQL");
		return $this->_db->GetAssoc($querystr);
	}
	
	// returns last autoincrement ID
	function getLastInsertID(&$id) {
		if (is_resource ( $this->db_linkid ))
			return $this->_db->_insertid();
		
		return false;
	}
	
	function numRows($results = NULL) {
		if (! isset ( $results ))
			$results = $this->db_qresult; 
		
		if ($this->isRSOk($results))
			return $results->FieldCount();
		else
			return 0;
	}
		
	function resetRecords(&$results) {
		$num_rows = $this->db_qresult->NumRows();
		nxLog ( 'num_rows:' . $num_rows, 'MYSQL' );
		if ($num_rows > 0)
			$this->db_qresult->_seek ( $row= 0 );
	}
	
	function queryFields(&$results) {
		if (! isset ( $results ))
			$results = $this->db_qresult;
		
		if ($this->isRSOk($results))
		{
		    // $ADODB_FETCH_MODE = ADODB_FETCH_ASSOC;
			// return $results->_fetch();
			$res= $results->GetRowAssoc(false);
			$results->_fetch();
			return $res;
		}
		else
			return null;
	}
	
	// Meta data
	/*
	function listDBs()
	{
	}
*/
	
	/* ========== transaction API ============ */
	function startTransaction() {
		$this->_db->BeginTrans();
	}
	function commit() {
		$this->_db->CommitTrans();
	}
	function rollBack() {
		$this->_db->RollbackTrans();
	}
	
	// manage eror and try to recover from eror if possible
	function onErrorRecover($query, $view = null, $handler = null) {
		$err = $this->_db->ErrorNo() ;
		if ($handler == null)
			$handler = &$this;
		
		switch ($err) {
			case 1146 :
			case 1051 :
				// table doesnt exist
				$res = $handler->createTableRecover ( $query, $view );
				break;
			case 1054 :
				// table structure issue
				$res = $handler->recoverTableStructure ( $query, $view );
				break;
			default :
				// dunno how to recover from other errors...
				return false;
		}
		
		return $res;
	}
	
	// try to create missing table(s) by calling  views related to these tables.
	// table names are extracted directy from queries.
	function createTableRecover($query, $view = null) {
		// extract tables from the query
		$tables = array ();
		if (! preg_match ( '/(FROM|INTO|UPDATE) +([^,( ]+([ ]*,[ ]*[^, (]+)*)[ ]*(WHERE|GROUP|SET|[(])*/i', $query, $matches )) {
			return false;
		}
		
		$tables = explode ( ',', $matches [2] );
		$res = true;
		foreach ( $tables as $table ) {
			$desc = explode ( '.', $table );
			if (isset ( $desc [1] ))
				$table = $desc [1];
			@$errorCB = new NxErrorCB ( );
			$res &= $this->executeDBQuery ( trim ( $table, ' _' ), 'createSQLTable', $errorCB, true, null, false );
		}
		return $res;
	}
	
	/**
	 * there is an issue with the table structure (missing field). Regenerate the table structure + data.
	 *
	 * @param unknown_type $query
	 * @param unknown_type $view
	 * @return unknown
	 */
	function recoverTableStructure($query, $view = null) {
		// extract tables from the query
		$tables = array ();
		if (! preg_match ( '/(FROM|INTO|UPDATE) +([^,( ]+([ ]*,[ ]*[^, (]+)*)[ ]*(WHERE|GROUP|SET|[(])*/i', $query, $matches )) {
			return false;
		}
		
		// manage package
		$_name = explode ( '@', $view );
		if (count ( $_name ) > 1) {
			$package = "packages/{$_name[1]}/";
		} else
			$package = '';
		
		$tables = explode ( ',', $matches [2] );
		$res = true;
		foreach ( $tables as $table ) {
			@$errorCB = new NxErrorCB ( );
			$res &= $this->updateTableStructure ( $table, $package );
		}
		return $res;
	}
	

	
	function updateTableStructure($table, $package) {
		$props = &$this->props->parent;
		
		if ($props == null)
			$props = &$this->props;
		
		$desc = explode ( '.', $table );
		if (isset ( $desc [1] ))
			$table = $desc [1];
			
		// 1) get target table structure from db desc
		if (! ($f = $this->props->getConfFile ( 'db', ltrim ( "{$table}_createSQLTable.inc", '_' ), $package ))) {
			return false;
		}
		include ($f);
		$queries = $properties ["query"];
		if (is_array ( $queries )) {
			// not supported yet
			return false;
		} else {
			$query = $queries;
		}
		// we have the create qurey: extract each field definition
		$tfields = $this->getFieldsFromCreateQuery ( $queries );
		
		// 2) get existing table structure
		$res = $this->queryWithData ( "SHOW CREATE TABLE `{$table}`", false );
		if (! $res) {
			return false;
		}
		$sfields = $this->getFieldsFromCreateQuery ( $res [0] ['Create Table'] );
		
		// 3) update structure
		$alterQ = "ALTER TABLE $table ";
		foreach ( $tfields as $fn => $fd ) {
			if (strtoupper ( $fn ) == 'PRIMARY')
				continue;
			
			if (! isset ( $sfields [$fn] )) {
				$alterQ .= "ADD `$fn` $fd,";
			} else {
				// $alterQ .= "CHANGE `$fn` `$fn` $fd,";				
			}
		}
		
		$alterQ = rtrim ( $alterQ, ',' );
		$res = $this->query ( $alterQ, false );
		if (! $res) {
			return false;
		}
		
		return true;
	
	}
	
	function getFieldsFromCreateQuery($query) {
		// we have the create qury: extract each field definition
		/* CREATE TABLE IF NOT EXISTS _worldpay_account(_oid INT AUTO_INCREMENT NOT NULL,_account_type TEXT,_testMode TEXT,
		   _instId VARCHAR(7),_currency TEXT,_lang TEXT,_serverURL VARCHAR(255),_desc TEXT,PRIMARY KEY(_oid))]
		*/
		$str = substr ( $query, strpos ( $query, '(' ) + 1 );
		$str = substr ( $str, 0, strrpos ( $str, ')' ) );
		$fields = array ();
		if ($count = preg_match_all ( '/[,]?\s*[`]?([^ \t`]+)[`]?\s+([^,]+)/', $str, $matches )) {
			for($i = 0; $i < $count; $i ++) {
				$fields [$matches [1] [$i]] = $matches [2] [$i];
			}
		}
		
		return $fields;
	}
	
	// setting error CB
	function setErrorCB(& $err) {
		$this->errorCB = & $err;
	}
	
	// logging error
	function error($msg) {
		$this->errorCB->addError ( "database error: try later or contact the site web master", $msg . ":" . $this->_db->errorMsg() );
		return false;
	}
	
	function isOk() {
		return $this->isConnected ();
	}
	
	/* ============      RECORD and VIEW API     ========================
		Description : 
			+ loads queries from conf/db directory
			+ works with Nx records (descriptor, iterator ec.)
			
		NB. should be in a different architecture layer?
	*/
	
	function getRecords(&$recDesc) {
		$this->recDesc = & $recDesc;
		return new NxDB_Records ( $this, $recDesc, $this->db_qresult );
	}
	
	function &getRecordIterator() {
		return new NxDB_RecordIterator ( $this, $this->db_qresult );
	}
	
	// ============ DATABASE ============
	// returns the db object 
	function &getDB(&$errorCB) {
		return $this;
	}
	
	function executeDBQuery($view, $queryType, &$errorCB, $tryAll = false, $props = null, $recover = true, $record = null) {
		// mapping view name to db view: check if property view.db or db.view exists
		$dbview = $this->getProperty ( "$view.db", null );
		if ($dbview == null)
			$dbview = $this->getProperty ( "db." . $view, $view );
			
		// manage package
		$_name = explode ( '@', $dbview );
		if (count ( $_name ) > 1) {
			$package = $_name [1];
			$dbview = $_name [0];
		} else
			$package = $this->getProperty ( 'nx.package', '' );
		
		if ($props == null) {
			$props = &$this->props->parent;
			
			if ($props == null)
				$props = &$this->props;
		}
		
		// get the query
		$properties = array ();
		if ($f = $props->getConfFile ( 'db', "{$dbview}_{$queryType}.inc", $package )) {
			//nxLog ( "DB view:$f", "DS_DB" );
			include ($f);
			
			$queries = $properties ["query"];
			
			// execute the query
			$db = & $this->getDB ( $errorCB );
			if ($errorCB->isError ())
				return false;
			
			if (is_array ( $queries )) {
				foreach ( $queries as $query ) {
					$query = preg_replace ( array ('/ AND 0=0/', '/LIMIT 0,-1/' ), '', $query );
					$db->query ( $query, true );
					if ($errorCB->isError () && $tryAll == false)
						return false;
				}
				if ($errorCB->isError ())
					return false;
			} else {
				
				$db->query ( preg_replace ( array ('/ AND 0=0/', '/LIMIT 0,-1/' ), '', $queries ), true );
				if ($errorCB->isError ())
					return false;
			}
			
			return true;
		}
		
		return false;
	}
	//stone
	function &getDBRecords(&$recDesc, &$errorCB, $pageOffset = null, $maxNbRecords = null, $record = null) {
		$dbview = $recDesc->getViewName ();
		
		// manage package
		$_name = explode ( '@', $dbview );
		if (count ( $_name ) > 1) {
			$package = $_name [1];
			$dbview = $_name [0];
		} else
			$package = $this->getProperty ( 'nx.package', '' );
		
		$props = &$recDesc->parent;
		
		// get the query
		$queryType = "select";
		
		if ($pageOffset != null && $maxNbRecords != null)
			$recordOffset = $pageOffset * $maxNbRecords;
		else
			$recordOffset = 0;
		
		$properties = array ("recordOffset" => $recordOffset, "nbRecordsPerPage" => $maxNbRecords );
		
		if ($f = $recDesc->getConfFile ( 'db', "{$dbview}_selectRecord.inc", $package )) {
			nxLog ( "DB view:$f", "DS_DB" );
		
			include ($f);
			$query = preg_replace ( array ('/ AND 0=0/', '/LIMIT 0,-1/' ), '', $properties ["query"] );
			
			// get the db
			$db = & $this->getDB ( $errorCB );
			if ($errorCB->isError ())
				return false;
			
			$db->query ( $query );
			if ($errorCB->isError ()) {
				
				return null;
			}
			$records = & $db->getRecords ( $recDesc );
			$this->records = $records;
			// when nb of records is limited, get total number of records in DB (ie query w/o limit)
			if (isset ( $properties ["pageOffset"] )) {
				$count = $this->countRecords ( $recDesc, $errorCB, $record );
				if ($errorCB->isError ())
					nxError ( 'DB Query failed: cant count nb of records for view ' . $recDesc->getViewName (), "PLUG_DB" );
				else
					// set page counters in the record collection
					$records->setPageCounters ( $count, $properties ["pageOffset"], $properties ["nbRecords"], $properties ["pageAlias"] );
			}
		

			return $records;
		}
		
		return null;
	}
	
	/** manage queries defined by a DML view */
	function _queryView(&$recDesc, $dbOp, &$errorCB, &$props, $record = null) {
		$dbview = $recDesc->getViewName ();
		
		// manage package
		$_name = explode ( '@', $dbview );
		if (count ( $_name ) > 1) {
			$package = $_name [1];
			$dbview = $_name [0];
		} else
			$package = $this->getProperty ( 'nx.package', '' );
			
		// get db object
		$db = & $this->getDB ( $errorCB );
		if ($errorCB->isError ())
			return false;
		
		if ($f = $recDesc->getConfFile ( 'db', $dbview . "_" . $dbOp . "Record.inc", $package )) {
			// get the query
			$properties = array ();
			
			nxLog ( "DB view:$f", "DS_DB" );
			include ($f);
			
			// execute it
			$query = preg_replace ( array ('/ AND 0=0/', '/LIMIT 0,-1/' ), '', $properties ["query"] );
			$res = $db->query ( $query );
			if ($dbOp == 'insert') {
				$this->getLastInsertID ( $oid );
				if (isset ( $record ))
					$record->onNew ( $oid, $errorCB );
			}
			return $res;
		}
		
		nxError ( "Cant find DB view:$dbview", "DS_DB" );
		return false;
	}
	
	/** manage insert, update and insert_update queries */
	function _queryView_insert_update(&$recDesc, $dbOp, &$errorCB, &$props, $record = null) {
		
		$dbOp = strtolower ( $dbOp );
		if ($dbOp == 'update_if_exists') {
			if ($this->countRecords ( $recDesc, $errorCB, $record ) == 0) {
				// record doesn't exist
				return false;
			} else {
				// record exists: update
				return $this->_queryView ( $recDesc, 'update', $errorCB, $recDesc->parent, $record );
			}
		} elseif ($dbOp == 'insert_if_not_exists') {
			if ($this->countRecords ( $recDesc, $errorCB, $record ) == 0) {
				// record doesn't exist: insert
				return $this->_queryView ( $recDesc, 'insert', $errorCB, $recDesc->parent, $record );
			} else {
				// record exists
				return true;
			}
		} elseif (strtolower ( $dbOp ) == 'update_insert') {
			if ($this->countRecords ( $recDesc, $errorCB, $record ) == 0) {
				// record doesn't exist: insert
				return $this->_queryView ( $recDesc, 'insert', $errorCB, $recDesc->parent, $record );
			} else {
				// record exists: update
				return $this->_queryView ( $recDesc, 'update', $errorCB, $recDesc->parent, $record );
			}
		}
		
		// other queries
		return $this->_queryView ( $recDesc, $dbOp, $errorCB, $recDesc->parent, $record );
	}
	
	function &countRecords(&$recDesc, &$errorCB, $record = null) {
		$props = &$recDesc->parent;
		
		$resultSet = $this->_queryView ( $recDesc, 'count', $errorCB, $props, $record );
		
		// exec the query
		if ($resultSet && ! $errorCB->isError ()) {
			// get count of all records in special field "nbrecords"
			$fields = $this->queryFields ( $resultSet );
			if ($fields)
				return $fields ["nbrecords"];
		}
		
		return null;
	}
	
	// add/modify records with recDesc
	function addDBRecords(&$recDesc, &$errorCB, &$records, $dbOp = "insert") {
		$props = &$recDesc->parent;
		$dbview = $recDesc->getViewName ();
		
		// get db object
		if ($errorCB->isError ())
			return false;
			
		// store records in the record set
		for($recIt = $records->getRecordIterator (); $recIt->isValid (); $recIt->next ()) {
			$record = & $recIt->getCurrent ();
			$record->initAspect ();
			if ($record->onBeforeSave ( $dbOp == "insert", $errorCB )) {
				$this->_queryView ( $recDesc, $dbOp, $errorCB, $props, $record );
				$record->onSave ( $dbOp == "insert", $errorCB );
			}
		}
		return true;
	}
	
	// add/modify records with view name
	function addDBRecord(&$recDesc, &$errorCB, &$record, $dbOp = "insert") {
		if (isset ( $record )) {
			$record->initAspect ();
			if (! $record->onBeforeSave ( $dbOp == "insert", $errorCB ))
				return false;
		}
		
		if ($this->_queryView_insert_update ( $recDesc, $dbOp, $errorCB, $recDesc->parent, $record )) {
			if (isset ( $record ))
				$record->onSave ( $dbOp == "insert", $errorCB );
			
			return true;
		}
		
		return false;
	}
	
	// private functions
	function getWhereClause($qfname, $values, $relation = "LIKE", $sep = "OR", $prefixVal = "%", $suffixVal = "%", $selEmptyKey = true, $selEmptyField = false, $csv_sep = '|') {
		// ignore empty values or text-choice prefix (?)
		if ($values == null || strtolower ( $values ) == 'null' || ($values = trim ( $values, '\'' )) === '' || ($values == '?') || $relation == '')
			if ($selEmptyKey)
				return '0=0';
			else
				return '0=1';
		
		if (! is_array ( $values ))
			$values = explode ( $csv_sep, $values );
		
		$relation = preg_replace ( array ('/gte/', '/gt/', '/lte/', '/lt/', '/eq/' ), array ('>=', '>', '<=', '<', '=' ), $relation );
		
		$clause = "(";
		$_sep = "";
		foreach ( $values as $v ) {
			$clause .= $_sep . "$qfname $relation '$prefixVal" . trim ( $v, "' " ) . "$suffixVal'";
			$_sep = " $sep ";
		}
		
		if ($selEmptyField)
			$clause .= $_sep . "$qfname=''";
		
		$clause .= ")";
		
		return $clause;
	}
	
	// for compatibility... to be removed when SQL regenrated
	function &getProperty($name, $deft = null, $cascade = true) {
		return $this->props->getProperty ( $name, $deft, $cascade );
	}
	
	// get field from record if any, or default value
	function &getParamFieldValue($record, $fname, $format = 'DB', $dft = null) {
		if ($record)
			return $record->getFieldValue ( $fname, $format, $dft );
		
		if ($dft)
			return $this->props->getQValue ( $dft );
		
		return null;
	}
	
	// get parameter from request or session
	function &getParam($name, $dft = null, $sep = ',', $record = null) {
		$n = preg_replace ( '/[-. ]/', '_', trim ( $name, '[]' ) );
		
		if (isset ( $_REQUEST [$n] ))
			$res = $_REQUEST [$n];
		elseif (isset ( $_SESSION [$n] ))
			$res = $_SESSION [$n];
		elseif ($dft == null || $dft == 'null')
			$res = null;
		else
			$res = $this->props->getQValue ( $dft );
			
		// if provided as an array, returns values as a list
		if (is_array ( $res ))
			$res = implode ( $sep, $res );
		
		return $res;
	}
	
	// get parameter from request or session
	function &getFieldParam($name, $dft = null, $sep = ',', $record = null, $opts = null) {
		$n = preg_replace ( '/[-. ]/', '_', trim ( $name, '[]' ) );
		
		if (isset ( $record )) {
			$res = $record->getFieldValue ( $n, "DB", $dft, $opts );
		} elseif (isset ( $_GET [$n] ))
			$res = $_GET [$n];
		elseif (isset ( $_REQUEST [$n] ))
			$res = $_REQUEST [$n];
		elseif (isset ( $_SESSION [$n] ))
			$res = $_SESSION [$n];
		elseif ($dft == null || $dft == 'null')
			$res = null;
		else
			$res = $this->props->getQValue ( $dft );
			
		// if provided as an array, returns values as a list
		if (is_array ( $res ))
			$res = implode ( $sep, $res );
		
		return $res;
	}
	
	// get parameter from specified http source
	function &getParamValue($name, $dft = null, $type = 'request', $sep = ',') {
		$n = preg_replace ( '/[-. ]/', '_', trim ( $name, '[]' ) );
		
		switch ($type) {
			case 'post' :
				@$var = &$_POST;
				break;
			case 'get' :
				@$var = &$_GET;
				break;
			case 'cookie' :
				@$var = &$_COOKIE;
				break;
			case 'session' :
				@$var = &$_SESSION;
				break;
			// case 'request':
			default :
				@$var = &$_REQUEST;
				break;
		}
		if (isset ( $var [$n] ))
			$res = $var [$n];
		else
			$res = $dft;
			
		// if provided as an array, returns values as a list
		// NB. assumes it is already wrapped with simple quotes
		if (is_array ( $res ))
			$res = implode ( "'$sep'", $res );
		
		return $res;
	}
	
	function &getQValue($qv) {
		return $this->props->getQValue ( $qv );
	}

}

?>
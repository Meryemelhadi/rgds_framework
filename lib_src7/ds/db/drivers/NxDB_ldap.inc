<?php 

class NxDB_ldap
{
	function __construct() {
	$cnxName = 'ldap';
		
		include (NX_CONF . 'ds/' . $cnxName . NX_HOST_SWITCH . '.inc');
		
		if (($this->host = @$properties ['ldap.host']) != null &&
		 	($this->port = @$properties ['ldap.port']) != null && 
		 	($this->version = @$properties ['ldap.version']) != null && 
			($this->user = @$properties ['ldap.user']) != null && 
		 	($this->pass = @$properties ['ldap.pass']) != null && 
		 	($this->baseDn = @$properties ['ldap.baseDn']) != null && 
		 	($this->ldap_host = @$properties ['ldap.ldap_host']) != null && 
		 	($this->ldap_user = @$properties ['ldap.ldap_user']) != null && 
		 	($this->people = @$properties ['ldap.people']) != null && 
		 	($this->searchUser = @$properties ['ldap.searchUser']) != null && 
		 	($this->searchPassword = @$properties ['ldap.searchPassword']) != null && 
		 	($this->ldap_pass = @$properties ['ldap.ldap_pass']) != null) {
			$this->database=$this->baseDn;
		 	$this->ds = $this->connect ($this->host,$this->user,$this->pass);
			nxlog ( "ldap admin parameters Ok" );
		
		} else {
			nxlog ( "missing ldap admin parameters for " . $cnxName );
		}
	}
	function connect($host, $user, $pass) {	
		
		if (! ($this->ds = ldap_connect ( $this->host, $this->port ))) {
			nxlog ( "Could not connect to ldap server" );
		}
		ldap_set_option ( $this->ds, LDAP_OPT_PROTOCOL_VERSION, 3 );
		ldap_set_option ( $this->ds, LDAP_OPT_REFERRALS, 0 );
		if (! ($bind = ldap_bind ( $this->ds, $this->user, $this->pass ))) {
			nxlog ( "Unable to bind to server" );
		}
		$this->connected = true;
		return $this->connected;
	 }
	 
	function close($linkid)               { 
				$this->disconnect ();
	}
	function & disconnect() {
		@ ldap_free_result ( $this->results );
		$this->connected = ! @ ldap_unbind ( $this->ds );
		return ! $this->connected;
	}
	function free_result(&$qresult)       { 
		@ ldap_free_result ( $this->results );
		$this->connected = ! @ ldap_unbind ( $this->ds );
		return true;
	}
	
	function query($querystr, $linkid)    { 
		//$rs = @ldap_search( $this->ds, $this->database, $querystr );
		$rs = @ldap_search( $this->ds, $this->baseDn, 'CN=sikobiko' );
		$this->result = $rs;
		return $rs; 
	}
	
	
	function select_db($dbname, $linkid)  {	
		$this->database = $dbname;
		return true; }
	function num_rows (&$result)		  { 
		 if ($this->_result and is_resource($this->ds)) {
            return @ldap_count_entries($this->ds, $this->_result);
        }
        return null;
	}
	function & fetch_array(&$result,$result_type) { 
	    $this->fields = ldap_get_attributes( $this->ds, $this->result );
        $this->fields = $this->GetRowAssoc();
           
        return ( is_array( $this->fields ) );       
		
	}
	function & GetRowAssoc()
	{
		$this->_numOfRows = ldap_count_entries( $this->ds, $this->result );
		
        $results = array();
        foreach ( $this->fields as $k=>$v ) {
            if ( is_array( $v ) ) {
                if ( $v['count'] == 1 ) {
                    $results[$k] = $v[0];
                } else {
                    array_shift( $v );
                    $results[$k] = $v;
                } 
            }
        }
 
		return $results; 
	}
	function reset_results(&$result)	  {
		
	}
	function getLastInsertID($linkid,&$id) { 
		
		return true;
	}
	
	function isTableExist($dbLink, $database, $tableName)
	{
		return true;
	}
	
	function & errno()					  { return true; }		
	function & error()					  { return true; }
	function auth($uid, $password) {
		$result = $this->findAll ( 'CN', $uid );
		
		if ($result [0]) {
			if (ldap_bind ( $this->ds, $result [0] ['dn'], $password )) {
				return true;
			} else {
				return false;
			}
		} else {
			return false;
		}
	}
	function findAll($attribute = 'CN', $value = '*', $baseDn = 'cn=Users,dc=techdays2009,dc=com') {
		$r = ldap_search ( $this->ds, $baseDn, $attribute . '=' . $value );
		
		if ($r) {
			//if the result contains entries with surnames,
			//sort by surname:
			ldap_sort ( $this->ds, $r, "CN" );
			
			return ldap_get_entries ( $this->ds, $r );
		}
	}
// returns true or false
//this->ds $db_linkid
	function init($host, $user, $pass, $dbname = "") 
		{
	global $LDAP_CONNECT_OPTIONS;
		
		if ( !function_exists( 'ldap_connect' ) ) return null;
		
		$conn_info = array( $host,$this->port);
		
		if ( strstr( $host, ':' ) ) {
		    $conn_info = explode( ':', $host );
		} 
		
		$this->ds = @ldap_connect( $conn_info[0], $conn_info[1] );
		if (!$this->ds) {
			$e = 'Could not connect to ' . $conn_info[0];
			nxError("error connecting to DB on [".$host."] db=[".$dbname."] connection=[".$e."]");
			return false;
		}
		if( count( $LDAP_CONNECT_OPTIONS ) > 0 ) {
			$this->_inject_bind_options( $LDAP_CONNECT_OPTIONS );
		}
		
		if ($username) {
		    $bind = @ldap_bind( $this->ds, $user, $pass );
		} else {
			$username = 'anonymous';
		    $bind = @ldap_bind( $this->ds );		
		}
		
		if (!$bind) {
			$e = sprintf($this->_bind_errmsg,ldap_error($this->ds));;
			nxError("error bind to DB on [".$host."] db=[".$dbname."] bind=[".$e."]");
			return false;
		}
		$this->_errorMsg = '';
		$this->database = $dbname;
		return $this->ds;
	}
    function queryWithData($querystr,$recover=true,$handler=null) {
		
		$r = ldap_search ( $this->ds, $this->baseDn, 'uid=' . $querystr );
		if ($r) {
			$l = ldap_get_entries ( $this->ds, $r );
			$convert = $this->convert_from_ldap ( $l );
			return $convert [0];
		}
	}
	function queryFields(&$results)
	{
		$this->fields = $this->GetRowAssoc();
    	return ( is_array( $this->fields ) ); 
	}   
/**
	 *  Binds as an authenticated user, which usually allows for write
	 * access. The FULL dn must be passed. For a directory manager, this is
	 * "cn=Directory Manager" . For a user, it will be something
	 * like "uid=sikobiko,ou=users,dc=techdays2009,dc=com".
	 */
	function bindauth($bindDn, $pass) {
		if (! $this->result = @ldap_bind ( $this->ds, $bindDn, $pass )) {
			return false;
		} else {
			return true;
		}
	}
	
	/**
	 * 	\brief 		Unbind du serveur ldap.
	 * 	\param		ds
	 * 	\return		bool
	 */
	function unbind() {
		if (! $this->result = @ldap_unbind ( $this->connection )) {
			return false;
		} else {
			return true;
		}
	}
//les infos du user connecté
	function fetch($user, $password) {
		
		// connect to server
		if (! ($this->ds = ldap_connect ( $this->ldap_host, 636 ))) {
			nxlog ( "Could not connect to ldap server" );
		}
		echo "connect result is: " . $this->ds . "<BR>";
		ldap_set_option ( $this->ds, LDAP_OPT_PROTOCOL_VERSION, $this->version );
		ldap_set_option ( $this->ds, LDAP_OPT_REFERRALS, 0 );
		
		// bind to server
		if (($bind = ldap_bind ( $this->ds, $user, $password ))) {
			return true;
		}
		nxlog ( "bind result is: " . $bind . "<BR>" );
		
		$base_dn = "DC=techdays2009,DC=com";
		$filter = "(cn=$user)";
	$read = ldap_search($this->ds, $base_dn, $filter);
	
		
		$info = ldap_get_entries ( $this->ds, $read );
		
		echo $info ["count"] . " entries returned<p>";
		echo $info [0] ["displayname"] [0];
		echo $info [0] ["cn"] [0];
		
		ldap_close ( $this->ds );
	}
    

}
?>
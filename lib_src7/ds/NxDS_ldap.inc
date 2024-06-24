<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// Data source for LDAP


require_once (NX_LIB . 'ds/NxDS_Source.inc');
require_once (NX_LIB . "NxDB_Record.inc");

class NxDS_ldap extends NxDS_Source {
	
	var $ldapcharset = 'UTF-8';
	var $_bind_errmsg = "Binding: %s";
	
	
	function __construct() 
	{
		
		$cnxName = 'ldap';
		
		include (NX_CONF . 'ds/' . $cnxName . NX_HOST_SWITCH . '.inc');
		
		if (($this->host = @$properties ['ldap.host']) != null &&
		 	($this->port = @$properties ['ldap.port']) != null && 
		 	($this->version = @$properties ['ldap.version']) != null && 
			($this->user = @$properties ['ldap.user']) != null && 
		 	($this->pass = @$properties ['ldap.pass']) != null && 
		 	($this->baseDn = @$properties ['ldap.baseDn']) != null && 
		 	($this->ldap_host = @$properties ['ldap.ldap_host']) != null && 
		 	($this->ds = @$properties ['ldap.ds']) == null && 
		 	($this->ldap_user = @$properties ['ldap.ldap_user']) != null && 
		 	($this->people = @$properties ['ldap.people']) != null && 
		 	($this->searchUser = @$properties ['ldap.searchUser']) != null && 
		 	($this->searchPassword = @$properties ['ldap.searchPassword']) != null && 
		 	($this->ldap_pass = @$properties ['ldap.ldap_pass']) != null) {
			
		 		$this->connect ();
			nxlog ( "ldap admin parameters Ok" );
		
		} else {
			nxlog ( "missing ldap admin parameters for " . $cnxName );
		}
	
	}
	
	/*    function __construct($config = null) {
 
        parent :: __construct($config);
        return $this->connect();
    }
    
    
    function __destruct() {
        $this->close();
        parent :: __destruct();
    }*/
	
/*
	Valid Domain Values for LDAP Options:

	LDAP_OPT_DEREF (integer)
	LDAP_OPT_SIZELIMIT (integer)
	LDAP_OPT_TIMELIMIT (integer)
	LDAP_OPT_PROTOCOL_VERSION (integer)
	LDAP_OPT_ERROR_NUMBER (integer)
	LDAP_OPT_REFERRALS (boolean)
	LDAP_OPT_RESTART (boolean)
	LDAP_OPT_HOST_NAME (string)
	LDAP_OPT_ERROR_STRING (string)
	LDAP_OPT_MATCHED_DN (string)
	LDAP_OPT_SERVER_CONTROLS (array)
	LDAP_OPT_CLIENT_CONTROLS (array)

	Make sure to set this BEFORE calling Connect()

	Example:

	$LDAP_CONNECT_OPTIONS = Array(
		Array (
			"OPTION_NAME"=>LDAP_OPT_DEREF,
			"OPTION_VALUE"=>2
		),
		Array (
			"OPTION_NAME"=>LDAP_OPT_SIZELIMIT,
			"OPTION_VALUE"=>100
		),
		Array (
			"OPTION_NAME"=>LDAP_OPT_TIMELIMIT,
			"OPTION_VALUE"=>30
		),
		Array (
			"OPTION_NAME"=>LDAP_OPT_PROTOCOL_VERSION,
			"OPTION_VALUE"=>3
		),
		Array (
			"OPTION_NAME"=>LDAP_OPT_ERROR_NUMBER,
			"OPTION_VALUE"=>13
		),
		Array (
			"OPTION_NAME"=>LDAP_OPT_REFERRALS,
			"OPTION_VALUE"=>FALSE
		),
		Array (
			"OPTION_NAME"=>LDAP_OPT_RESTART,
			"OPTION_VALUE"=>FALSE
		)
	);
*/

	function _inject_bind_options( $options ) {
		foreach( $options as $option ) {
			ldap_set_option( $this->ds, $option["OPTION_NAME"], $option["OPTION_VALUE"] )
				or die( "Unable to set server option: " . $option["OPTION_NAME"] );
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
			nxError("error connecting to LDAP on [".$host."] db=[".$dbname."] connection=[".$e."]");
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
	
	function isConnected() {
		return $this->connected;
	}
	
	
	function close() {
		
		$this->disconnect ();
	}
	
	function selectDB($db) {
		$this->database = $db;
		return true;
	} // SelectDB
	
/* returns _queryID or false */
//	function query($querystr,$recover=true,$handler=null) 
//	function _query($sql,$inputarr)

	function query($querystr,$recover=true,$handler=null)
	{
		$rs = @ldap_search( $this->ds, $this->database, $querystr );
		//nxLog("Recovered from DB error, try again..",'LDAP');
		return $rs; 
	}
	
// executes a query and returns results asociative array
//function queryWithData($querystr,$recover=true,$handler=null) {
//function queryWithData($fields, $uid) {
	
	function queryWithData($querystr,$recover=true,$handler=null) {
		
		$r = ldap_search ( $this->ds, $this->baseDn, 'uid=' . $querystr );
		if ($r) {
			$l = ldap_get_entries ( $this->ds, $r );
			$convert = $this->convert_from_ldap ( $l );
			return $convert [0];
		}
	}


	 function numRows() {
        if ($this->_result and is_resource($this->ds)) {
            return @ ldap_count_entries($this->ds, $this->_result);
        }
        return null;
    } 
    
    function &GetRowAssoc()
	{
	$this->_numOfRows = ldap_count_entries( $this->ds, $this->_queryID );
		
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
		
           
    
    
    
    function queryFields(&$results)
	{
		$this->fields = $this->GetRowAssoc();
    	return ( is_array( $this->fields ) ); 
	}   
	
    
	function disconnect() {
		@ ldap_free_result ( $this->results );
		$this->connected = ! @ ldap_unbind ( $this->ds );
		return ! $this->connected;
	}
	
	// Connection --------------------------------------------------------------
	function connect() {
		
		ldap_set_option ( $this->ds, LDAP_OPT_PROTOCOL_VERSION, $this->version );
		ldap_set_option ( $this->ds, LDAP_OPT_REFERRALS, 0 );
		if (! ($this->ds = ldap_connect ( $this->host, $this->port ))) {
			nxlog ( "Could not connect to ldap server" );
		}
		if (! ($bind = ldap_bind ( $this->ds, $this->user, $this->pass ))) {
			nxlog ( "Unable to bind to server" );
		}
		$this->connected = true;
		return $this->connected;
	
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
	
	//-------
	/*
 usage:
//Get all users:
$this->NxDS_ldap->findAll('uid', '*');

//Get users from a specific group inside the 'People' group:
$this->NxDS_ldap->findAll('uid', '*', 'ou=Client Company,ou=People,cn=example,cn=com');

//Get a specific user:
$this->NxDS_ldap->findAll('uid', 'jsmith'); 
 */
	
	function findAll($attribute = 'CN', $value = '*', $baseDn = 'cn=Users,dc=techdays2009,dc=com') {
		$r = ldap_search ( $this->ds, $baseDn, $attribute . '=' . $value );
		
		if ($r) {
			//if the result contains entries with surnames,
			//sort by surname:
			ldap_sort ( $this->ds, $r, "CN" );
			
			return ldap_get_entries ( $this->ds, $r );
		}
	}
	//--------
	/*
 * usage:
 * $this->NxDS_ldap->auth($u, $p);
 */
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
	
	//------------------------------------------
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
	/*--------------------------------------------------------------------------------------
          Recordset
--------------------------------------------------------------------------------------*/
	
	/**
	 * 		\brief		Returns an array containing a details of elements
	 *		\param		$search			 	Valeur champ clé recherché, sinon '*' pour tous.
	 *		\param		$userDn			 	DN (Ex: ou=adherents,ou=people,dc=parinux,dc=org)
	 *		\param		$useridentifier 	Nom du champ clé (Ex: uid)
	 *		\param		$attributeArray 	Array of fields required (Ex: sn,userPassword)
	 * if ($attributeArray==NULL){ $attributeArray=array("samaccountname","mail","memberof","department","displayname","telephonenumber","primarygroupid"); }
	 *		\param		$activefilter		1=utilise le champ this->filter comme filtre
	 *		\return		array				Array of [id_record][ldap_field]=value
	 * 		\remarks	ldapsearch -LLLx -hlocalhost -Dcn=admin,dc=parinux,dc=org -w password -b "ou=adherents,ou=people,dc=parinux,dc=org" userPassword
	 * 
	 * 
	 * 	
	 * nx\conf\db\user_insertRecord.inc
	 * 
	function getuserrecords($user,$pass)
// Liste des champs a récupérer de LDAP
	$required_fields = array(
	$conf->global->LDAP_FIELD_FULLNAME,
	$conf->global->LDAP_FIELD_LOGIN,
...
	$required_fields=array_unique(array_values(array_filter($required_fields, "dolValidElement")));
	 *',$conf->global->LDAP_MEMBER_DN, $conf->global->LDAP_KEY_MEMBERS, $required_fields, 0);
	if (is_array($ldaprecords))
	{
		

		// Warning $ldapuser a une clé en minuscule
		foreach ($ldaprecords as $key => $ldapuser)
		{

$properties["query"]="INSERT INTO _user(_oid,_user_id,_password,_email,_groups,_reg_date,_checkedEmail,_reg_status,_empid,_first_name,_last_name,_gender,_civility,_marital_status,_birth_date,_birth_location,_address,_town,_rib,_diploma,_mobile_phone,_local_phone,_personal_phone,_start_service,_group,_contract,_csp,_fonction,_role,_localisation,_cne,_cnss,_cimr,_last_credit,_current_credit,_current_year,_reliquat,_notes,_photo) VALUES(".$record->getFieldValue("oid","DB","NULL").",".
$ldapuser[$conf->global->LDAP_FIELD_FIRSTNAME].",".
//--
			$member = new user($db);

			// Propriete membre
			$member->prenom=$ldapuser[$conf->global->LDAP_FIELD_FIRSTNAME];
			$member->nom=$ldapuser[$conf->global->LDAP_FIELD_NAME];
	 */
	function getRecords($search, $userDn, $useridentifier, $attributeArray, $activefilter = 0) {
		$fulllist = array ();
		
		// if the directory is AD, then bind first with the search user first
		

		$this->bindauth ( $user, $pass );
		nxLog ( "Ldap::bindauth serverType=activedirectory searchUser=" . $user );
		
		// Define filter
		if ($activefilter == 1) {
			if ($this->filter) {
				$filter = '(' . $filter . ')';
			} else {
				$filter = '(' . $useridentifier . '=*)';
			}
		} else {
			$filter = '(' . $useridentifier . '=' . $search . ')';
		}
		
		if (is_array ( $attributeArray )) {
			// Return list with required fields
			nxLog ( "Ldap::getRecords connection=" . $this->ds . " userDn=" . $userDn . " filter=" . $filter . " attributeArray=(" . join ( ',', $attributeArray ) . ")" );
			$this->result = @ldap_search ( $this->ds, $userDn, $filter, $attributeArray );
		} else {
			// Return list with fields selected by default
			nxLog ( "Ldap::getRecords connection=" . $this->ds . " userDn=" . $user . " filter=" . $filter );
			$this->result = @ldap_search ( $this->ds, $userDn, $filter );
		}
		if (! $this->result) {
			nxLog ( 'LDAP search failed: ' . ldap_errno ( $this->ds ) . " " . ldap_error ( $this->ds ) );
			return - 1;
		}
		
		$info = @ldap_get_entries ( $this->ds, $this->result );
		
		// Warning: Dans info, les noms d'attributs sont en minuscule meme si passé
		// a ldap_search en majuscule !!!
		//print_r($info);
		

		for($i = 0; $i < $info ["count"]; $i ++) {
			$recordid = $this->convToOutputCharset ( $info [$i] [$useridentifier] [0], $this->ldapcharset );
			if ($recordid) {
				//print "Found record with key $useridentifier=".$recordid."<br>\n";
				$fulllist [$recordid] [$useridentifier] = $recordid;
				
				// Add to the array for each attribute in my list
				for($j = 0; $j < count ( $attributeArray ); $j ++) {
					$keyattributelower = strtolower ( $attributeArray [$j] );
					//print " Param ".$attributeArray[$j]."=".$info[$i][$keyattributelower][0]."<br>\n";
					

					//permet de récupérer le SID avec Active Directory
					if ($keyattributelower == "objectsid") {
						$objectsid = $this->getObjectSid ( $recordid );
						$fulllist [$recordid] [$attributeArray [$j]] = $objectsid;
					} else {
						$fulllist [$recordid] [$attributeArray [$j]] = $this->convToOutputCharset ( $info [$i] [$keyattributelower] [0], $this->ldapcharset );
					}
				}
			}
		}
		
		asort ( $fulllist );
		return $fulllist;
	}
	
	//Attribute methods -----------------------------------------------------
	/**
	 *  Returns an array containing values for an attribute and for first record matching filterrecord
	 */
	
	function getAttribute($filterrecord, $attribute) {
		$attributes [0] = $attribute;
		
		// We need to search for this user in order to get their entry.
		$this->result = @ldap_search ( $this->ds, $this->people, $filterrecord, $attributes );
		
		$entry = ldap_first_entry ( $this->ds, $this->result );
		
		if (! $entry) {
			nxLog ( "Couldn't find user" );
			return false; // Couldn't find the user...
		}
		
		// Get values
		if (! $values = @ldap_get_values ( $this->ds, $entry, $attribute )) {
			nxLog ( ldap_errno ( $this->ds ) );
			return false; // No matching attributes
		}
		
		// Return an array containing the attributes.
		return $values;
	}
	
	function putRecord($data) {
		
		$dn = "uid=" . $data ['NxDS_ldap'] ['uid'] . "," . $this->baseDn;
		
		foreach ( $data ['NxDS_ldap'] as $field => $value ) :
			$data_ldap [$field] [0] = $value;
		endforeach;
		
		$data_ldap ['objectClass'] = array ('account', 'posixAccount', 'top', 'shadowAccount' );
		
		return ldap_add ( $this->ds, $dn, $data_ldap );
	}
	
	function del($uid) {
		$dn = "uid=$uid," . $this->baseDn;
		return ldap_delete ( $this->ds, $dn );
	}
	
	function findLargestUidNumber() {
		$r = ldap_search ( $this->ds, $this->baseDn, 'uidnumber=*' );
		if ($r) {
			ldap_sort ( $this->ds, $r, "uidnumber" );
			
			$result = ldap_get_entries ( $this->ds, $r );
			$count = $result ['count'];
			$biguid = $result [$count - 1] ['uidnumber'] [0];
			return $biguid;
		}
		return null;
	}
	
	/**
	 *  \brief      Convert a string into output/memory charset
	 *  \param      str            	String to convert
	 *  \param		pagecodefrom	Page code of src string
	 *  \return     string         	Converted string
	 */
	function convToOutputCharset($str, $pagecodefrom = 'UTF-8') {
		global $conf;
		if ($pagecodefrom == 'ISO-8859-1' && $conf->character_set_client == 'UTF-8')
			$str = utf8_encode ( $str );
		if ($pagecodefrom == 'UTF-8' && $conf->character_set_client == 'ISO-8859-1')
			$str = utf8_decode ( $str );
		return $str;
	}
	
	/**
	 * Récupère le SID de l'utilisateur
	 * ldapuser. le login de l'utilisateur
	 * Indispensable pour Active Directory
	 */
	function getObjectSid($ldapUser) {
		$criteria = '(' . $this->getUserIdentifier () . '=' . $ldapUser . ')';
		$justthese = array ("objectsid" );
		
		// if the directory is AD, then bind first with the search user first
		if ($this->serverType == "activedirectory") {
			$this->bindauth ( $this->searchUser, $this->searchPassword );
		}
		
		$i = 0;
		$searchDN = $this->people;
		
		while ( $i <= 2 ) {
			$ldapSearchResult = @ldap_search ( $this->ds, $searchDN, $criteria, $justthese );
			
			if (! $ldapSearchResult) {
				nxerror ( ldap_errno ( $this->ds ) . " " . ldap_error ( $this->ds ) );
				return - 1;
			}
			
			$entry = ldap_first_entry ( $this->ds, $ldapSearchResult );
			
			if (! $entry) {
				// Si pas de résultat on cherche dans le domaine
				$searchDN = $this->domain;
				$i ++;
			} else {
				$i ++;
				$i ++;
			}
		}
		
		if ($entry) {
			$ldapBinary = ldap_get_values_len ( $this->ds, $entry, "objectsid" );
			$SIDText = $this->binSIDtoText ( $ldapBinary [0] );
			return $SIDText;
		} else {
			nxerror ( ldap_errno ( $this->ds ) . " " . ldap_error ( $this->ds ) );
			return '?';
		}
	}
	
	/**
	 * Returns the textual SID
	 * Indispensable pour Active Directory
	 */
	function binSIDtoText($binsid) {
		$hex_sid = bin2hex ( $binsid );
		$rev = hexdec ( substr ( $hex_sid, 0, 2 ) ); // Get revision-part of SID
		$subcount = hexdec ( substr ( $hex_sid, 2, 2 ) ); // Get count of sub-auth entries
		$auth = hexdec ( substr ( $hex_sid, 4, 12 ) ); // SECURITY_NT_AUTHORITY
		$result = "$rev-$auth";
		for($x = 0; $x < $subcount; $x ++) {
			$subauth [$x] = hexdec ( $this->littleEndian ( substr ( $hex_sid, 16 + ($x * 8), 8 ) ) ); // get all SECURITY_NT_AUTHORITY
			$result .= "-" . $subauth [$x];
		}
		return $result;
	}
	
	function convert_from_ldap($data) {
		foreach ( $data as $key => $row ) :
			if ($key === 'count')
				continue;
			
			foreach ( $row as $key1 => $param ) :
				if (! is_numeric ( $key1 ))
					continue;
				if ($row [$param] ['count'] === 1)
					$final [$key] ['NxDS_ldap'] [$param] = $row [$param] [0];
				else {
					foreach ( $row [$param] as $key2 => $item ) :
						if ($key2 === 'count')
							continue;
						$final [$key] ['NxDS_ldap'] [$param] [] = $item;
					endforeach;
				}
			endforeach;
		endforeach;
		return $final;
	}
	
	function dolValidElement($element) {
		return (trim ( $element ) != '');
	}

}
?>
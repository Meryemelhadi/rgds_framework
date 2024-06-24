<?php

function NxLogin_key_closeSession() 
{ 
	session_destroy();
} 

$GLOBALS['NxLogin_key_current']=false;

class NxLogin_keySite extends NxProperties
{
	var $config;
	var $props;

	function __construct(&$props,$config=null,$pageOnly=false)
	{
		$this->site=&$props;
		$this->config=&$config;
		$this->pageOnly=$pageOnly;
	}

	function getDBName($subdomain=null)
	{
		$objErr = new NxErrorCB();
		$db = & $this->site->getDB($objErr);
		
		if ($subdomain==null)
		{
			$domainParts=explode('.',$_SERVER['SERVER_NAME']);
			$subdomain=$domainParts[0];
		}

		$res=$db->queryWithData("select _database_id from _directory_customer where _domain_name='".$subdomain."' and _account_activation = 0");	
		if ($res)
		{
			return $res[0]['_database_id'];
		}

		return '';
	}

	function loadUserContext($mkey,$service='user')
	{
		// close session after this script
		// register_shutdown_function('NxLogin_key_closeSession');
		
		// check key
		if (!$this->checkKeyAccess($mkey,$service))
			return false;
			
		// load user data
		$user_data = $this->site->getRecord($service.'.user_data');
		$user=new NxProperties($p=array(),$parent=null);
		$user_data->saveAsProperties($user,false,'Object');
		
		$_SESSION['NX_SITE_KEY']=$mkey;
				
		return $user;
	}

	// return true if no branch perm or intersection(user perms,tree perms) 
	function checkKeyAccess($mkey=null,$service='user')
	{
		$uoid=substr($mkey, 32);
		if ($this->pageOnly)
			$script=$_SERVER['SCRIPT_NAME'];
		else
			$script='';
		$mdKref=$this->encodeKeyAccess($uoid,$script,$service);
		return ($mdKref==$mkey);
	}
	
	function getCurrentKey()
	{
		global $loginService;

		if (isset($_REQUEST['skey']))
			return $_REQUEST['skey'];
			
		if($GLOBALS['NxLogin_key_current']!=false)
			return $GLOBALS['NxLogin_key_current'];
		$what=$this->pageOnly?$_SERVER['SCRIPT_NAME']:'';
		$uoid=$GLOBALS['_NX_user']->getProperty('oid');
		return $GLOBALS['NxLogin_key_current']=$this->encodeKeyAccess($uoid,$what,$loginService);
	}

	function getCurrentParamKey($pageOnly=true)
	{
		return '&skey='.$this->getCurrentKey();
	}
	
  	function getCurrentUrlKey()
	{
		$url=$_SERVER['HTTP_SITE_URL'].$_SERVER['SCRIPT_NAME'].'?'.$_SERVER['QUERY_STRING'];
		if (!isset($_REQUEST['skey']))
			$url.='&skey='.$this->getCurrentKey();
		
		return $url;
	}

	function getUrlKeyFor($uoid,$pwd,$what='',$service='user')
	{
		return '&amp;skey='.md5($uoid.$what.$pwd).$uoid;
	}

	function encodeKeyAccess($uoid,$what,$service,$subdomain=null)
	{
		$dbPrefix = $this->getDBName($subdomain);
		if ($dbPrefix!='')
		{
			$this->site->setProperty('account_db',$dbPrefix);
			$dbPrefix.='.';
		}

		// get DB service
		$error=new NxErrorCB();
		$db = & $GLOBALS['_NX_site']->getDB($error);

		// check if user is a group manager, ie. validator
		$res=$db->queryWithData(
		<<<EOH
			SELECT _user_id,_password
			FROM {$dbPrefix}_user 
			WHERE _oid='$uoid'
EOH
		);
		if ($res==false)
			return false;

		$_POST['password'] = $pwd = $res[0]['_password'];
		$_POST['user_id'] = $res[0]['_user_id'];
		return md5($uoid.($this->pageOnly?$what:'').$pwd).$uoid;
	}
}

?>
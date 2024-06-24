<?php

function NxLogin_key_closeSession() 
{ 
	session_destroy();
} 

$GLOBALS['NxLogin_key_current']=false;

class NxLogin_key extends NxProperties
{
	var $config;
	var $props;

	function __construct(&$props,$config=null,$pageOnly=true,$ip=false)
	{
		$this->site=&$props;
		$this->config=&$config;
		$this->pageOnly=$pageOnly;
		$this->ip=$ip;
	}

	function loadUserContext($mkey,$service='user')
	{
		// close session after this script
		// register_shutdown_function('NxLogin_key_closeSession');
		
		// check key
		if (!$this->checkKeyAccess($mkey,$service))
			return false;

		session_unset();
			
		// load user data
		$user_data = $this->site->getRecord($service.'.user_data');
		$user=new NxProperties($p=array(),$parent=null);
		$user_data->saveAsProperties($user,false,'Object');
		
		$_SESSION['NX_PAGE_KEY']=$mkey;
				
		return $user;
	}

    function getCurrentScript() {

        // remap buttons
        if (isset($_SERVER['UNENCODED_URL']))
        {
            return $_SERVER['UNENCODED_URL'];
        }
        elseif (isset($_SERVER['REDIRECT_URL']))
        {
            return $_SERVER['REDIRECT_URL'];
        }
            
        return $_SERVER['PHP_SELF'];
    }
    
    
	// return true if no branch perm or intersection(user perms,tree perms) 
	function checkKeyAccess($mkey=null,$service='user')
	{
		$uoid=substr($mkey, 32);
		if ($this->pageOnly)
			$script=$this->getCurrentScript();
		else
			$script='';
            
		$mdKref=$this->encodeKeyAccess($uoid,$script,$service);
		if($mdKref==$mkey)
            return true;

        $mdKref=$this->encodeKeyAccess($uoid,$_SERVER['PHP_SELF'],$service);
        if($mdKref==$mkey)
            return true;
           
        return false;
	}
	
	function getCurrentKey($what=null,$uoid=null)
	{
		global $loginService;

		if (isset($_REQUEST['mkey']))
			return $_REQUEST['mkey'];
			
		if($GLOBALS['NxLogin_key_current']!=false)
			return $GLOBALS['NxLogin_key_current'];
        if(!$what)
		    $what=$this->pageOnly?$_SERVER['PHP_SELF']:'';
		if(!$uoid)
			$uoid=$GLOBALS['_NX_user']->getProperty('oid');

		return $GLOBALS['NxLogin_key_current']=$this->encodeKeyAccess($uoid,$what,$loginService);
	}

	function getCurrentParamKey($pageOnly=true,$what=null,$uoid=null)
	{
		return '&mkey='.$this->getCurrentKey($what,$uoid);
	}
	
  	function getCurrentUrlKey()
	{
		$url=$_SERVER['HTTP_SITE_URL'].$_SERVER['SCRIPT_NAME'].'?'.$_SERVER['QUERY_STRING'];
		if (!isset($_REQUEST['mkey']))
			$url.='&mkey='.$this->getCurrentKey();
		
		return $url;
	}

	function getUrlKeyFor($uoid,$pwd,$what='',$service='user')
	{
		return '&amp;mkey='.md5($uoid.$what.$pwd).$uoid;
	}

	function encodeKeyAccess($uoid,$what,$service,$ip=false)
	{
		// get DB service
		$error=new NxErrorCB();
		$db = & $GLOBALS['_NX_site']->getDB($error);

		// check if user is a group manager, ie. validator
		$res=$db->queryWithData(
		<<<EOH
			SELECT _user_id,_password
			FROM _user 
			WHERE _oid='$uoid'
EOH
		);
		if ($res==false)
			return false;

		$_POST['password'] = $pwd = $res[0]['_password'];
		$_POST['user_id'] = $res[0]['_user_id'];
		$seq=$uoid.($this->pageOnly?$what:'').$pwd;
		if($this->ip)
			$seq.=$_SERVER['REMOTE_ADDR'];
		return md5($seq).$uoid;
	}
}

?>
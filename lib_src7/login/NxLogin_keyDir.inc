<?php

$GLOBALS['NxLogin_key_current']=false;

class NxLogin_keyDir extends NxProperties
{
	var $config;
	var $props;

	function __construct(&$props,$config=null,$dirOnly=true,$ip=false)
	{
		$this->site=&$props;
		$this->config=&$config;
		$this->dirOnly=$dirOnly;
		$this->ip=$ip;
	}

	function loadUserContext($dkey,$service='user')
	{
		// close session after this script
		// register_shutdown_function('NxLogin_keyDir_closeSession');
		
		// check key
		if (!$this->checkKeyAccess($dkey,$service))
			return false;

		session_unset();
			
		// load user data
		$user_data = $this->site->getRecord($service.'.user_data');
		$user=new NxProperties($p=array(),$parent=null);
		$user_data->saveAsProperties($user,false,'Object');
		
		$_SESSION['NX_DIR_KEY']=$dkey;
				
		return $user;
	}

    function getCurrentScript() {

        // remap buttons
        if (isset($_SERVER['UNENCODED_URL']))
        {
            return dirname($_SERVER['UNENCODED_URL']);
        }
        elseif (isset($_SERVER['REDIRECT_URL']))
        {
            return dirname($_SERVER['REDIRECT_URL']);
        }
            
        return dirname($_SERVER['PHP_SELF']);
    }
    
    
	// return true if no branch perm or intersection(user perms,tree perms) 
	function checkKeyAccess($dkey=null,$service='user')
	{
		$uoid=substr($dkey, 32);
		if ($this->dirOnly)
			$script=$this->getCurrentScript();
		else
			$script='';
            
		$mdKref=$this->encodeKeyAccess($uoid,$script,$service);
		if($mdKref==$dkey)
            return true;

        $mdKref=$this->encodeKeyAccess($uoid,$_SERVER['PHP_SELF'],$service);
        if($mdKref==$dkey)
            return true;
           
        return false;
	}
	
	function getCurrentKey($what=null,$uoid=null)
	{
		global $loginService;

		if (isset($_REQUEST['dkey']))
			return $_REQUEST['dkey'];
			
		if($GLOBALS['NxLogin_keyDir_current']!=false)
			return $GLOBALS['NxLogin_keyDir_current'];
        if(!$what)
		    $what=$this->dirOnly?dirname($_SERVER['PHP_SELF']):'';
		if(!$uoid)
			$uoid=$GLOBALS['_NX_user']->getProperty('oid');

		return $GLOBALS['NxLogin_keyDir_current']=$this->encodeKeyAccess($uoid,$what,$loginService);
	}

	function getCurrentParamKey($dirOnly=true,$what=null,$uoid=null)
	{
		return '&dkey='.$this->getCurrentKey($what,$uoid);
	}
	
  	function getCurrentUrlKey()
	{
		$url=$_SERVER['HTTP_SITE_URL'].$_SERVER['SCRIPT_NAME'].'?'.$_SERVER['QUERY_STRING'];
		if (!isset($_REQUEST['dkey']))
			$url.='&dkey='.$this->getCurrentKey();
		
		return $url;
	}

	function getUrlKeyFor($uoid,$pwd,$what='',$service='user')
	{
		return '&amp;dkey='.md5($uoid.$what.$pwd).$uoid;
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
		$seq=$uoid.($this->dirOnly?$what:'').$pwd;
		if($this->ip)
			$seq.=$_SERVER['REMOTE_ADDR'];
		return md5($seq).$uoid;
	}
}

?>
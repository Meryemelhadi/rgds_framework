<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// portal user management
require_once(NX_LIB."login/NxUser_Data.inc");

class NxPlug_User
{
	var $props;
	function __construct(&$props)
	{
		$this->props =& $props;
	}
	
	function login($perm="default", $ownerID=null, $userID=null,$password=null)
	{
		global $_NX_user;
		
		session_start();
		if ($userID===null || $password===null)
		{
			// perform the login
			$_NX_user = $this->requestUserOnce($perm,$ownerID);
		}
		else
		{
			// load user data from the database without interaction with user interface
			$_NX_user = $this->loadUser($userID,$password);
		}

		if ($_NX_user != null)
		{
			$this->props->setProperty("user.userID",$_NX_user->userID);
			$this->props->setProperty("user",$_NX_user);
			$this->props->setProperty("user.oid",$_NX_user->getProperty("user.oid"));

			// overload default settings for file uploads
			// $this->props->setProperty("pathUpload",NX_PUBLIC_USERS.$_NX_user->userID."/");
			// $this->props->setProperty("baseUrlUpload",NX_PUBLIC_USERS_URL.$_NX_user->userID."/");		
		}
		
		return $_NX_user;
	}

	// if no user session, request user and check it against user store, check permissions
	function & requestUserOnce($perm=null, $ownerID=null)
	{
		// get user from session
		$this->user = & $this->getUser();
		
		if ($this->user == null)
		{
			// load interface and store
			
			// Where user data is stored
			$userStoreClass = $this->props->getProperty("login.store","NxUser_StoreDB");
			$this->userStore =& $this->props->getPlugIn($userStoreClass,'','login');
			
			// what is the login interface required (currently supports forms and http basic)
			$loginInterfaceClass = $this->props->getProperty("login.interface","NxLogin_PHPAuth");
			$this->interface =& $this->props->getPlugIn($loginInterfaceClass,'','login');
			
			// not logged in => request it from interface
			// if returns, means user is not null
			$this->user =& $this->interface->login();
			
			// does it match an existing user
			if ($this->userStore->checkUser($this->user) != true)
			{
				nxLog('unknown user','login');
				// unknown user => request it again
				$this->interface->requestLogin();
				return null;
			}
	
			nxLog('user ok','login');
			
			// ok, login worked, store it for the session's life
			$this->_storeUser();
		}
			
		// check its permissions for the page
		if ($this->user->hasPerms($perm,$ownerID) == false)
		{
			// load interface plug in
			$loginInterfaceClass = $this->props->getProperty("login.interface","NxLogin_PHPAuth");
			$this->interface =& $this->props->getPlugIn($loginInterfaceClass,'','login');

			$this->interface->loginError("no permission",$this->user);
		}
		
		return $this->user;
	}

	// load user from database, using ID/password, check permissions
	function &loadUser($userID,$password, $perm=null, $ownerID=null)
	{
		// $userID=strtolower($userID);

		// create user object from user and password
		$this->user = new NxUser_Data($userID,$password,$this->props);
		
		// get class that store users in database
		$userStoreClass = $this->props->getProperty("login.store","NxUser_StoreDB");
		$this->userStore =& $this->props->getPlugIn($userStoreClass,'','login');

		// load user data in user object
		if ($this->userStore->checkUser($this->user,$userID,$password) != true)
		{
			// unknown user
			return null;
		}

		// store it for the session's life
		$this->_storeUser();
		
		// check its permissions for the page
		if ($perm != null && $this->user->hasPerms($perm,$ownerID) == false)
		{
			// load interface plug in
			$loginInterfaceClass = $this->props->getProperty("login.interface","NxLogin_PHPAuth");
			$this->interface =& $this->props->getPlugIn($loginInterfaceClass,'','login');

			$this->interface->loginError("no permission",$this->user);
		}
		
		return $this->user;
	}
	
	// close user session
	function logoff()
	{
		// already logged in
		if (isset($_SESSION["_nxuser_"]))
		{
			$_SESSION["_nxuser_"]=null;
			unset($_SESSION["_nxuser_"]);
			session_destroy();
		}
			
		return null;
	}
	
	// get user object from session
	function & getUser()
	{
		// already logged in
		if (isset($_SESSION["_nxuser_"]))
			$user =& $_SESSION["_nxuser_"];
		else
			$user = null;

		return $user;
	}

	// get current user ID, or null
	function getUserID()
	{
		$user = $this->getUser();
		if ($user != null)
			return $user->userID;
		else
			return null;
	}

	// check if user ID exists in user store
	function existUserID($userID)
	{
		$userID=strtolower($userID);
		
		$userStoreClass = $this->props->getProperty("login.store","NxUser_StoreDB");
		$this->userStore =& $this->props->getPlugIn($userStoreClass,'','login');
		
		// does it match an existing user
		return $this->userStore->existUserID($userID);
	}	

	// check if user ID exists in user store
	function refreshCurrentUser()
	{
		$userStoreClass = $this->props->getProperty("login.store","NxUser_StoreDB");
		$this->userStore =& $this->props->getPlugIn($userStoreClass,'','login');
		
		$user =& $this->getUser();
		
		// does it match an existing user
		return $this->userStore->refreshCurrentUser($user);
	}	

	// store user in session.
	function _storeUser()
	{
		$_SESSION["_nxuser_"] = &$this->user;
	}
}

?>
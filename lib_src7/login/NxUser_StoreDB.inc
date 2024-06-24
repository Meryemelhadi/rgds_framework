<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// manage a user database
class NxUser_StoreDB
{
	function __construct(&$props)
	{
		$this->props = &$props;
	}
	
	function checkUser(&$user,$userID=null, $password=null)
	{
		trace("check user","login");
		
		if ($userID===null)
			$userID = $user->userID;
		
		if ($password===null)
			$password = $user->password;
		
		$records = & $this->getUserRecords($userID, $password);
		if ($records === null || $records->count() <= 0)
		{
			trace("can't find db record for this user","login");
			return false;
		}

		trace("user found with right password","login");
			
		// get user data
		$recIt = $records->getRecordIterator(); 
		if ($recIt->isValid())
		{
			$record = & $recIt->getCurrent();
			$record->saveAsProperties($user,false,"HTML");
		}
				
		$user->setPerms("default"); 
		$user->validateUser();
		return true;
	}
	
	function & getUserRecords($userID, $password) 
	{
		$db = & $this->props->getPlugIn("DB");
		$errorCB = new NxErrorCB();
		return $db->getRecords("account.user_data",$errorCB);
	}

	function existUserID($userID) 
	{
		$this->props->setProperty("user.exist.userID",$userID);
		$db = & $this->props->getPlugIn("DB");
		$errorCB = new NxErrorCB();
		$records =& $db->getRecords("user.exist",$errorCB);

		if ($records === null || $records->count() <= 0)
			return false;
		else
			return true;

	}

	function refreshCurrentUser(&$user)
	{
		$db = & $this->props->getPlugIn("DB");
		$errorCB = new NxErrorCB();
		$records = & $db->getRecords("user.current.data",$errorCB);
		
		if ($records === null || $records->count() <= 0)
		{
			nxLog("can't find db record for current user => refresh failed","login");
			return false;
		}

		// get user data and update user object
		$recIt = $records->getRecordIterator(); 
		if ($recIt->isValid())
		{
			$record = & $recIt->getCurrent();
			$record->saveAsProperties($user,false,"HTML");
		}
				
		return true;
	}
	
}

?>

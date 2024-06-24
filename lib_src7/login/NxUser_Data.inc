<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// class that hold data/properties for current user
class NxUser_Data extends NxProperties
{
	function __construct($userID,$password,&$parent)
	{
		$props = array();
		parent::__construct($props,$parent);
		$this->userID 	= $userID;
		$this->password = $password;
		$this->isValid = false;
		
		$parent->setProperty("user.id",$userID);
	}
	
	// sets permissions and clear the password
	function setPerms($perms)
	{
		$this->perms  = explode(",",$perms);
	}
	
	function validateUser()
	{
		$this->password = null;
		$this->isValid = true;
	}
	
	// display a login error and redirect to the right page as appropriate
	function hasPerms($perm,$ownerID=null)
	{
		// no required permission
		if ($perm==null)
			return true;
		
		// check owner permission
		if ($perm == "owner" && isset($ownerID) && $ownerID == $this->userID)
			return true;

		// check other permissions
		foreach ($this->perms as $p)
			if ($p == $perm)
				return true;
		
		return false;
	}
}

?>

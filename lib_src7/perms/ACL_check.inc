<?php

class ACL_check {
	
	function ACL_check() {
	}
	
	function checkPerm() {
	}
	
	function getResoures() {		
	}
	
}

class ACL_manage {
	var $userID;
	var $db;
	var $error;
	
	function ACL_manage($userID,&$props) {
		$this->userID=$userID;
		
		// get db
		// error handler
		$this->error = new NxErrorCB();
		$this->db = & $props->getDB($this->error);
	}
	
	//  {private}
	
	function _getURI($app_uri) {
		return "{$this->userID}-{$app_uri}";
	}

	function _getGroupID($group,$uri,$add=true) {
		$res=$this->db->queryWithData("select _goid from _acl_group where _uri='$uri'");
		if ($res!=null)
			return $res[0]['_goid'];
			
		return null;
	}

	/* get user identity oid */
	function _getUserID($email,$goid,$uri) {
	
		$res=$this->db->queryWithData("select _ioid from _acl_identity where _email='{$email}' and _goid='$goid' and _uri='$uri'");
		if ($res!=null)
			return $res[0]['_ioid'];
			
		return null;
	}
	
	//  {public}

	// create a group
	// NB. foid is the oid of a group descriptor in the application space
	// it's a way to connect the ACL group with an application group.
	function createGroup($name,$app_uri,$desc,$group_oid=-1) {
		$uri=$this->_getURI($app_uri);
//		$query="insert into _acl_group(_name, _desc, _uri, _foid) values('{$name}','$desc','$uri', '$group_oid')";
		$query="insert into _acl_group(_name, _desc, _uri) values('{$name}','$desc','$uri')";
		if ($this->db->query($query))
		{
			$this->db->getLastInsertID($goid);
			return $goid;
		}
		
		return null;
	}
	
	function deleteGroup($name,$app_uri) {	
		$uri=$this->_getURI($app_uri);
		$goid=$this->_getGroupID($group,$uri,false);
		if ($goid==null)
			return null;

		$this->startTransaction();
		
		$query="insert into _acl_group(_name, _desc, _uri) values('{$name}','$desc','$uri')";
		$this->execQuery($query);
	}
	
	/* create new user identity */
	// NB. foid is the oid of a user descriptor in the application space
	// it's a way to connect the ACL identities with an application user.
	function createUser($email,$group='my_contacts',$app_uri='contacts', $user_oid=-1) {
		$uri=$this->_getURI($app_uri);
		$goid=$this->_getGroupID($group,$uri,true);
		if ($goid==null)
			if(($goid=$this->createGroup($group,$app_uri,"group: $group"))==null)
				return null;

		// $this->db->startTransaction();
		
		// add user record
		if (!$this->db->query("insert into _acl_identity(_email, _goid, _uri, _foid) values('$email','$goid','$uri','$user_oid')"))
		{
			// $this->db->rollback();
			return null;
		}
		$this->db->getLastInsertID($ioid);
		
		/*
		// insert user in group
		if(!$this->db->query("insert into _acl_goup_user(_ioid,_goid) values({$ioid},$goid)"))
		{
			$this->db->rollback();
			return null;
		}
		$this->db->commit();
		*/
		
		return $ioid;
	}
	
	/* delete user from perm db */
	function deleteUser($email,$group='my_contacts',$app_uri='contacts') {
		$uri=$this->_getURI($app_uri);
		$goid=$this->_getGroupID($group,$uri,false);
		if ($goid==null)
			return null;
			
		if(!$this->db->query("delete from _acl_identity where _email='{$email}' and _goid='$goid' and _uri='$uri'"))
		{
			return false;
		}
		return true;
	}
	
	function addPerm($email, $user_oid, $group, $group_oid, $app_uri, $res_uri, $res_oid, $rights) 
	{
		$uri=$this->_getURI($app_uri);
		
		// get group
		$goid=$this->_getGroupID($group,$uri,true);
		if ($goid==null)
			if(($goid=$this->createGroup($group,$app_uri,"group: $group"))==null)
				return null;
			
		// get user
		if ($email!=null)
		{
			$ioid = $this->_getUserID($email, $goid, $uri);
			if ($ioid==null)
			{
				$ioid = $this->createUser($email, $group, $app_uri, $user_oid);
			}
		}
		else
			$ioid=-1;	
		
		// add acl record
		if($this->db->query("insert into _acl(_ioid,_goid,_rights,_res_uri,_res_oid) values({$ioid},$goid,'$rights','$res_uri',$res_oid)"))
			return true;
		else
			return false;		
	}

	// set permission: if email, set perm on whole group
	// setPerm('franck@nxfrontier.com',12,'my contacts',2,'user_contacts','site://franck.nxoffice.net/',-1,'R|W|A|Contacts')
	// @param user_oid : oid to record holding data for the user. Not used by acl but stored by it as can be convenient for table joins
	// @param group_oid : oid to record holding data for the group. Not used by acl but convenient for table joins
	function setPerm($email, $user_oid, $group, $group_oid, $app_uri, $res_uri, $res_oid, $rights) {
		$uri=$this->_getURI($app_uri);
		
		// get group
		$goid=$this->_getGroupID($group,$uri,true);
		if ($goid==null)
			if(($goid=$this->createGroup($group,$app_uri,"group: $group"))==null)
				return null;
			
		// get user
		if ($email!=null)
		{
			$ioid = $this->_getUserID($email, $goid, $uri);
			if ($ioid==null)
				$ioid = $this->createUser($email, $group, $app_uri, $user_oid);
			if ($ioid==null)
				nxError("can't set perms on $email, $group, $app_uri, $res_uri, $res_oid, $rights",'ACL');
		}
		else
			$ioid=-1;	
		
		// check if acl exists, and if not add a new one or update it
		if($this->db->query("select _ioid FROM _acl WHERE _ioid=$ioid AND _goid=$goid AND _res_uri='$res_uri' AND _res_oid=$res_oid"))
		{
			$this->db->query("UPDATE _acl SET _rights='$rights' WHERE _ioid=$ioid AND _goid=$goid AND _res_uri='$res_uri' AND _res_oid=$res_oid");
		}
		else
		{			
			$this->db->query("insert into _acl(_ioid,_goid,_rights,_res_uri,_res_oid) values({$ioid},$goid,'$rights','$res_uri',$res_oid)");
		}
	}

	function removePerm($email, $group, $app_uri, $res_uri, $res_oid) 
	{
		$uri=$this->_getURI($app_uri);
		
		// get group
		$goid=$this->_getGroupID($group,$uri,false);
		if ($goid==null)
			return null;
			
		// get user
		if ($email!=null)
		{
			$ioid = $this->_getUserID($email, $goid, $uri);
			if ($ioid==null)
			{
				return null;
			}
		}
		else
			$ioid=-1;	
		
		// remove acl record
		if($this->db->query("delete from  _acl where _ioid=$ioid AND _goid=$goid AND _res_uri='$res_uri' AND _res_oid='$res_oid'"))
			return true;
		else
			return false;		
	}	
	
	function getPerm($email, $group, $app_uri, $res_uri, $res_oid) {
		$uri=$this->_getURI($app_uri);
		
		// get group
		$goid=$this->_getGroupID($group,$uri,false);
		if ($goid==null)
			return null;
			
		// get user
		if ($email!=null)
		{
			$ioid = $this->_getUserID($email, $goid, $uri);
			if ($ioid==null)
			{
				// unknown user
				return '';
			}
		}
		else
			$ioid=-1;	
		
		// check if acl exists, and if not add a new one or update it
		$res=$this->db->queryWithData("select _rights FROM _acl WHERE _ioid=$ioid AND _goid=$goid AND _res_uri='$res_uri' AND _res_oid='$res_oid");
		if ($res) {
			return $res[0]['_rights'];
		}

		// no right
		return '';
	}

	function checkPerm($email, $group, $app_uri, $res_uri, $res_oid,$right) {
		$rights=$this->getPerm($email, $group, $app_uri, $res_uri, $res_oid);
		if (strpos("|$rights|", "|$right|")!==false)
			return true;
		else
			return false;
	}
	
	function listResources($email, $res_uri, $fields, $table,$resoidField) {
		
		// check if acl exists, and if not add a new one or update it
		$res=$this->db->queryWithData("select $fields FROM _acl,_acl_identity as i,$table WHERE i._email='$email' AND _acl._res_uri LIKE '$res_uri%' AND _acl._res_oid=${table}.{$resoidField}");
		if ($res) {
			return $res;
		}

		// no right
		return null;
	}
	
	function listPerms($email,$uri_match) {
		// $res_uri="site://site-$site_oid";
		
		// check if acl exists, and if not add a new one or update it
		$res=$this->db->queryWithData("select _res_uri as uri, _rights as rights FROM _acl,_acl_identity as i WHERE i._email='$email' AND i._ioid=_acl._ioid AND _acl._res_uri LIKE '$uri_match%' GROUP BY uri,rights");
		if ($res) {
			return $res;
		}
		
		return array();
	}	
}

/* class managing rights for sites */
class SitePerms extends ACL_manage
{
	function SitePerms($userID,&$props) {
		parent::ACL_manage($userID,$props);
	}
	
	function setSitePerms($email,$site_oid, $owner, $rights) {
		$group='mycontacts'; 
		$app_uri='mycontacts';
		$res_uri="site://$owner/site-$site_oid";
		$this->setPerm($email, 0, $group, 0 , $app_uri,$res_uri, $site_oid,$rights);
	}
	
	function listSites($email) {
		$res_uri="site://";
		return $this->listResources($email, $res_uri, '_oid,_name', '_site','_oid');
	}
}

?>
<?php

require_once(NX_XLIB.'lib/framework.inc');
include_once(NX_XLIB.'lib/rh/conge/workflow.inc');
require_once(NX_LIB.'NxData_ArrayRecords.inc');

/* application */
class lib_rh_conge_records_myteam 
{
	var $ctxt;

	function __construct($ctxt,$instance) {
		$this->ctxt=$ctxt;
	}

	function getUsers($group,$groupByParent)
	{
		$users = $group->getUsers();
		$oid=$group->oid->string;
		if ($groupByParent[$oid])
			foreach($groupByParent[$oid] as $child)
				$users += $this->getUsers($group,$child);

		return $users;
	}

	function getSubGroups($group,$groupByParent)
	{
		// get group oid
		$oid=$group->oid->string;

		// store current group
		$groups[$oid]=$group;

		// check its parent : 
		if ($groupByParent[$oid])
			foreach($groupByParent[$oid] as $child)
				$groups += $this->getSubGroups($child,$groupByParent);

		return $groups;
	}

	function getRecords($recDesc,$errorCB,$data,$props) 
	{
		$view = $recDesc->view;

		// get user oid as param or else, user current user
		if ($props)
			$uoid = $props->getProperty('user_oid');

		if ($uoid==null)
			$uoid= Framework::getUser()->getProperty('oid',0);

		$user = rh_userPeer::getByOid($uoid);

		$myGroups = array();

		// get all groups
		$groups = rh_groupPeer::getAll();
		$groupByOid=array();
		$groupsByParent=array();
		$myGroups=array();
		foreach($groups as $group)
		{
			$oid = $group->oid->object;
			$groupByOid[$oid]=$group;

			$parentOid = $group->getParentOid();
			if ($parentOid!=null)
				$groupsByParent[$parentOid][]=$group;

			if ($group->isManager($uoid))
				$myGroups[$oid]=$group;
		}

		// get all sub groups
		$mySubGroups = $myGroups;
		foreach($myGroups as $group)
		{
			$mySubGroups += $this->getSubGroups($group,$groupsByParent);
		}

		// now we can get all IDs as a CSV string for getting users
		if (count($mySubGroups)==0)
			return null;

		$myGroupOids = implode('|',array_keys($mySubGroups));

		$users = rh_userPeer::getByGroups($myGroupOids,'records');
		return $users;
	}

	function countRecords($recDesc,$errorCB)                 {}
	function addRecords($recDesc,$errorCB,$records,$dbOp)    {}
	function addRecord($recDesc,$errorCB,$record,$dbOp)      {}
	function deleteRecord($recDesc, $errorCB,$props,$record) {}
}

?>
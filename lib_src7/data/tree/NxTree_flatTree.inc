<?php 

class NxTree_flatTree
{
	function __construct($name,$perms='') 
	{
		$this->tree=array();
		$this->perms=$perms;
	}
	
	function addBranch($id,$path,$name,$fullName,$perms='',$sep='-',$oid=null)
	{
		if (!$this->matchPerm($perms,$this->perms))
			return false;

		if ($fullName=='')
			$fullName=$name;
			
		$level=count(explode($sep,$path));
		$path_id=preg_replace(array('/\W+/','/^_+/'),array('_','_'),$path);
		
		$this->tree[$path_id]=array(
			'oid'=>$oid,
			'id'=>$id,
			'path'=>$path,
			'level'=>$level-1,
			'name'=>$name,
			'fullName'=>$fullName,
			'perms'=>$perms,
			'docs'=>array()
			);
			
		return true;
	}
	
	function addLeaf($path,$value,$perms='')
	{
		$path_id=preg_replace(array('/\W+/','/^_+/'),array('_','_'),$path);

		if (!$this->matchPerm($perms,$this->perms) || !isset($this->tree[$path_id]))
			return false;	
		
		$this->tree[$path_id]['docs'][]=$value;			
		return true;		
	}
	
	// return true if no branch perm or intersection(user perms,tree perms) 
	function matchPerm($branchperms,$userperm)
	{
		$branchperms=trim(preg_replace('/[;|, ]+/',',',$branchperms),',');
		if ($branchperms=='')
			return true;
			
		$acl=explode(',',$branchperms);
		$auser=explode(',',trim(preg_replace('/[;|, ]+/',',',$userperm),','));
		if(count(array_intersect($acl,$auser))>0)
			return true;
			
		return false;
	}	
}

?>
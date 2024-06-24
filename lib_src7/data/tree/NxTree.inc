<?php 

class NxTree
{
	/* remove branches requesting a different level of permission */
	function permTrim(&$tree,$perm)
	{
		if (isset($tree['children']))
			foreach ($tree['children'] as $k=>$v)
			{
				$child=&$tree['children'][$k];
				
				if (isset($child['access']) && $child['access']!='' && $perm!=$child['access'])
					unset($tree['children'][$k]);
				else 
					NxTree::permTrim($tree['children'][$k],$perm);
			}
			
		return $tree;
	}
	
	// load a tree (array)
	function load($path,$name='tree',$format='php',$perm='') 
	{
		if ($format=='php')
		{
			include($path);
			return NxTree::permTrim($$name,$perm);
		}
		
		return array();
	}
	
	// add a leaf to the tree
	function add(&$tree,$node_id,$path,$root_name,$path_sep,$leaf_val,$permission='')
	{
		// get path as array
		// $path=str_replace($root_name,'',$path);
		// $apath=explode($path_sep,str_replace($root_name,'root',$path));
		
		// build path
		$branch="\$tree".preg_replace(
			array(
				'/^[?]/',
				'/[-]([^-]+)/',
			),
			array(				
				'',
				"['children']['$1']",
			),
			$path
		);
		
		// look for position in tree
		$test=false;
		@eval("\$test=isset($branch);");
		if (!$test)
			// not found
			return false;
			
		eval("\$v=&$branch;");
			
		if ($node_id==$v['id'])
			// leaf same name as branch ID, then add value to branch
			$v['value']=$leaf_val;
		else
			// add value to branch
			$v['children'][]=array('value'=>$leaf_val,'type'=>'leaf');
		
		return true;
	}
}

?>
<?PHP

// all data format operations
class taglib_nxfrontier_nxl_flat_tree extends NxTagLib
{		
	// <tree:load format="php" file="#const:NX_CONF#content/nav/#property:tree.name#.inc" />
	// load a tree from a file.
	// tree of the form: array('name'=>'root',children'=>array(...))
	function create($node,&$content,$level,$pad)
	{	
		// get name
		$this->getAttribute($name,$node,'name',"tree",true,true);
		
		// get user permission
		$this->getAttribute($permission,$node,'permission','',true,true);

		// request library load
		// request library load
		$this->context->addInclude('data/tree/NxTree_flatTree.inc');
		
		return "\n{$pad}new NxTree_flatTree($name,$permission)";
	}
	
	// <tree:addBranch tree="#$tree#" path="#$path#" value="#$branch#" permission="#$perms#"/>
	// add a leaf to the tree with:
	//  - tree: tree variable or property (must be loaded by load function above)
	//	- path: where to put the leaf, default: tree root
	//  - value: leaf value. Could be any type
	//  - permission: permission value, must match the tree permission.
	function addBranch($node,&$content,$level,$pad)
	{	
		// get tree value
		$this->getAttribute($tree,$node,'tree',null,true,true);
		
		// get path
		$this->getAttribute($path,$node,'path','?',true,true);
		$this->getAttribute($name,$node,'name','',true,true);
		$this->getAttribute($fullName,$node,'fullName',$name,true,true);
		$this->getAttribute($id,$node,'id',$name,true,true);
		$this->getAttribute($oid,$node,'oid',null,true,true);
		
		// get user permission
		$this->getAttribute($permission,$node,'permission','',true,true);
		
		// request library load
		$this->context->addInclude('data/tree/NxTree_flatTree.inc');
		
		// get tmp var ID
		$tree_var=$this->createVarID('tree');

		return "\n{$pad}((\${$tree_var}=&$tree)!=null?\${$tree_var}->addBranch($id,$path,$name,$fullName,$permission,'-',$oid):0);";
	}

	// <tree:addLeaf tree="#$tree#" path="#$path#" value="#$branch#" permission="#$perms#"/>
	// add a leaf to the tree with:
	//  - tree: tree variable or property (must be loaded by load function above)
	//	- path: where to put the leaf, default: tree root
	//  - value: leaf value. Could be any type
	//  - permission: permission value, must match the tree permission.
	function addLeaf($node,&$content,$level,$pad)
	{	
		// get tree value
		$this->getAttribute($tree,$node,'tree',null,true,true);
		
		// get format
		$this->getAttribute($path,$node,'path','?',true,true);
		
		// get tree value
		if (!$this->getAttribute($value,$node,'value',null,true,true))
			if ($content!='')
			{
				if (preg_match('/\$/',$content))
					$value=$content;
				else
					$value="'$content'";
			}
			else
				return $this->fatal('no value');
		
		// get user permission
		$this->getAttribute($permission,$node,'permission','',true,true);
		
		// request library load
		$this->context->addInclude('data/tree/NxTree_flatTree.inc');
		
		// get tmp var ID
		$tree_var=$this->createVarID('tree');

		return "\n{$pad}((\${$tree_var}=&$tree)!=null?\${$tree_var}->addLeaf($path,$value,$permission):0);";
	}
}

?>
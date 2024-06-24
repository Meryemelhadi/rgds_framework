<?PHP

// all data format operations
class taglib_nxfrontier_nxl_tree extends NxTagLib
{		
	// <tree:load format="php" file="#const:NX_CONF#content/nav/#property:tree.name#.inc" />
	// load a tree from a file.
	// tree of the form: array('name'=>'root',children'=>array(...))
	function load($node,&$content,$level,$pad)
	{	
		// get name
		$this->getAttribute($name,$node,'name',"tree",true,true);
		
		// get format
		$this->getAttribute($fmt,$node,'format',"php",true,true);
		
		// get content
		$this->getAttribute($file,$node,'file',null,true,true);
		
		// get user permission
		$this->getAttribute($permission,$node,'permission','',true,true);

		// request library load
		$this->context->addInclude('data/tree/NxTree.inc');
		
		return "\n{$pad}NxTree::load($file,$name,$fmt,$permission)";
	}
	
	// <tree:add tree="#$ref_tree#" path="#$path#" pathroot="?" pathsep="-" value="#$oid#" expand="true"/>
	// add a leaf to the tree with:
	//  - tree: tree variable or property (must be loaded by load function above)
	//	- path: where to put the leaf, default: tree root
	//  - pathsep: separator used for the path default=-
	//  - pathroot:special symbol for the root default=?
	//  - value: leaf value. Could be any type
	//  - expand: if path outside tree, ignore or expand tree? default=expand
	function add($node,&$content,$level,$pad)
	{	
		// get tree value
		$this->getAttribute($tree,$node,'tree',null,true,true);
		
		// get root symbol
		$this->getAttribute($pathroot,$node,'pathroot','?',true,true);
		
		// get root symbol
		$this->getAttribute($id,$node,'id','',true,true);

		// get path separator
		$this->getAttribute($pathsep,$node,'pathsep','-',true,true);

		// get format
		$this->getAttribute($path,$node,'path',$pathroot,true,true);
		
		// get tree value
		$this->getAttribute($value,$node,'value',$content,true,true);
		
		// get user permission
		$this->getAttribute($permission,$node,'permission','',true,true);
		
		// request library load
		$this->context->addInclude('data/tree/NxTree.inc');
		
		return "\n{$pad}NxTree::add($tree,$id,$path,$pathroot,$pathsep,$value,$permission);";
	}
}

?>
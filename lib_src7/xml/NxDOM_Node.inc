<?PHP

define('ELEMENT_NODE',1);
define('ATTRIBUTE_NODE',2);
define('TEXT_NODE',3);
define('COMMENT_NODE',8);
define('DOCUMENT_NODE',9);

class NxDOM_Node
{
	var $tagName;
	var $id;
	var $parent=-1;
	var $nodeType=ELEMENT_NODE;
	var $localName;
	var $baseURI;
	var $childNodes=array();
	var $firstChild=null;
	var $lastChild=null;
	var $valid=true;
		
	function __construct($qname,&$doc,$baseURI,$type=ELEMENT_NODE)
	{
		$this->nodeType=$type;
		$this->tagName=$qname;
		$aqn=explode(':',$qname);
		if (count($aqn)>1)
		{
			$this->localName=$aqn[1];
			$this->prefix=$aqn[0];
		}
		else
		{
			$this->localName=$qname;
			$this->prefix=null;
		}
		
		$this->baseURI=$baseURI;		
		$doc->_addNode($this);
	}
	
	function &removeChild($oldChild)
	{
		$len=count($this->childNodes);
		$id=$oldChild->id;
		for($i=0;$i<$len;$i++)
		{
			if($this->childNodes[$i]->id==$id)
				break;
		}
		if ($i>=$len)
			return null;
			
		$child=&$this->childNodes[$i];
		$child->parent=0;
		if ($i==0)
		{
			if ($len-1>0)
				$this->firstChild=&$this->childNodes[1];
			else
				$this->firstChild=null;
		}
		
		if ($i==$len-1)
		{
			if ($len-1>0)
				$this->lastChild=&$this->childNodes[$len-2];
			else
				$this->lastChild=null;
		}
		unset($this->childNodes[$i]);
		
		// reorder array
		$this->childNodes=&array_values($this->childNodes);
		for($i=0;$i<$len-1;$i++)
		{
			NXDOM_Document::_updateNode($this->childNodes[$i]);
		}
		NXDOM_Document::_updateNode($this);
		
		return $child;
	}
	
	function appendChild(&$node)
	{
		$node->parent=$this->id;
		if (count($this->childNodes)==0)
		{
			$this->firstChild=&$node;
		}
		$this->lastChild=&$node;
		$this->childNodes[]=&$node;

		NXDOM_Document::_updateNode($node);
		NXDOM_Document::_updateNode($this);
	}
	
	// attach to parent but dont show it in list of parent children
	// useful for attaching attribute expression to their parent node
	function _attachToParent(&$parentNode)
	{
		nxltrace("attach node:{$this->id} to node {$parentNode->id}",'debug');
		$this->parent=$parentNode->id;
		NXDOM_Document::_updateNode($this);
	}
		
	function &getChildNodes()
	{
		return new NXDOM_NodeList($this->childNodes);
	}
	
	function hasChildNodes()
	{
		return count($this->childNodes)>0;
	}
	
	function _invalidate()
	{
		$this->valid=false;
	}
	
	function _refreshNode()
	{
	}
	
	function _restoreNode()
	{
	}

	function & _removeLastChild() {
		$child=&array_pop($this->childNodes);
		if (($len=count($this->childNodes))>0)
		{
			$this->firstChild=&$this->childNodes[0];
			$this->lastChild=&$this->childNodes[$len-1];
		}
		else
		{
			$this->firstChild=null;
			$this->lastChild=null;
		}
		NXDOM_Document::_updateNode($child);
		NXDOM_Document::_updateNode($this);
		return $child;
	}
	
	/* obsolete ? */
	function _addToCollection($collName,$name,$val)
	{
		if (!isset($this->coll[$collName]))
			$this->coll[$collName]=array();
			
		$this->coll[$collName][$name]=$val;
	}
	
	function _replaceChildren(&$nodes) {
		foreach ($node->childNodes as $child)
		{
			$child['_parent']=0;
		}
		$node['_childNodes']=array();
				
		foreach ($nodes as $child)
		{
			$this->appendChild($node,$child);
		}
	}
	
	function _setBaseTag($tag)
	{
		$this->baseTag=$tag;
	}
	
	function toString() {
		return "node: <b>{$this->tagName}</b> id={$this->id} parent={$this->parent} type={$this->nodeType} baseURI={$this->baseURI}";
	}

	function toXML() {
		return '';
	}
}

class NxDOM_Attr extends NxDOM_Node
{
	var $name;
	var $specified;
	var $value;
	var $ownerElement;
	
	function __construct($qname,&$doc,$value,&$ownerElement,$baseURI='')
	{
		if ($baseURI=='')
			$baseURI=$ownerElement->baseURI;
			
		parent::__construct($qname,$doc,$baseURI,ATTRIBUTE_NODE);
		$this->name=$qname;
		$this->value=$value;
		$this->ownerElement=&$ownerElement;
		$this->specified=true;
		$this->parent=$ownerElement->id;
	}
	
	function toString() {
		return "<b>Attr:{$this->tagName}</b> id={$this->id} name={$this->name} parent={$this->parent} type={$this->nodeType} baseURI={$this->baseURI} value=[{$this->value}]";
	}

	function toXML() {
		return "$this->name=\"{$this->value}\"";
	}
}

class NxDOM_Element extends NxDOM_Node
{
	var $atts;
	var $nsdef=null;
	
	function __construct($tag,$atts=null,&$doc,$baseURI)
	{
		parent::__construct($tag,$doc,$baseURI,ELEMENT_NODE);
						
		if ($atts===null)
			$this->atts=array();
		else
			$this->atts=&$atts;
			
		if (isset($atts['_nsdef']))
		{
			$this->nsdef=$atts['_nsdef'];
			unset($this->atts['_nsdef']);
		}
	}
	
	// copy global node state to node instance
	// store local attributes for later restore
	function _refreshNode()
	{
		$node=NXDOM_Document::__refreshNode($this);
		$this->atts_bak=$this->atts;
		$this->atts=$node->atts;		
	}

	// restore the last node modification, update global instance
	function _restoreNode()
	{
		$this->atts=$this->atts_bak;
		unset($this->atts_bak);
		NXDOM_Document::_updateNode($this);
	}
	
	function _getAttributes() {
		return $this->atts;
	}

	function _setAttributes(&$atts) {
		$this->atts=$atts;
		NXDOM_Document::_updateNode($this);
	}

	function getAttribute($name,$deft=null) {
		$name=strtolower($name);
		if (isset($this->atts[$name]))
			return $this->atts[$name];
		else
			return $deft;
	}
	
	function setAttribute($n,$v) {
		$n=strtolower($n);
		$this->atts[$n]=$v;
		NXDOM_Document::_updateNode($this);
	}
	
	function removeAttribute($n) {
		$n=strtolower($n);
		if (isset($this->atts[$n]))
			unset($this->atts[$n]);
		NXDOM_Document::_updateNode($this);
	}
	
	// set default attribute value (coll, name, default)
	function setDeftAtt($n,$d)
	{
		$n=strtolower($n);
		if (!isset($this->atts[$n]))
		{
			$this->atts[$n]=$d;
			NXDOM_Document::_updateNode($this);
		}
		
		return $this->atts[$n];
	}
	
	function setNSDef($nsdef) {
		$this->nsdef=$nsdef;
	}
	
	function &getElementsByTagName($name)
	{
		$children=&$this->childNodes;
		$ret=array();
		$len=count($children);
		for ($i=0;$i<$len;$i++)
		{
			$child=&$children[$i];
			//if ($name=='*' || $name==$child->tagName)
			if (($name=='*' && $child->localName!='#text') || $name==$child->localName)
			{
				$ret[]=&$child;
			}
		}
		return new NXDOM_NodeList($ret);
	}
	
	function &getElementsByTagNameNS($nsURI,$localName)
	{
		$children=&$this->childNodes;
		$ret=array();
		$len=count($children);
		for ($i=0;$i<$len;$i++)
		{
			$child=&$children[$i];
			if ($name=='*' || 
				($name==$child->localName && $nsURI==$child->baseURI))
			{
				$ret[]=&$child;
			}
		}
		return new NXDOM_NodeList($ret);
	}
	
	function toString($full=false) {
		$res="<b>Element: {$this->tagName}</b> id={$this->id} parent={$this->parent} type={$this->nodeType} baseURI={$this->baseURI}";
		if ($full && $this->nsdef)
			$res.=var_export($this->nsdef,true);
		return $res;		
	}
	
	function toXML($open=true) {
		if ($open)
		{
			$atts='';
			foreach ($this->atts as $an=>$av)
			{
				$atts .=" $an=\"$av\"";
			}		
	
			return "&lt;{$this->tagName}{$atts}&gt;";
		}
		
		return "&lt;/{$this->tagName}&gt;";
	}
}	

class NxDOM_String
{
}

class NxDOM_Text extends NxDOM_Node
{
	var $data;
	function __construct($doc,$text='',$baseURI=null)
	{
		parent::__construct('#text',$doc,$baseURI,TEXT_NODE);
		$this->data=$text;
	}
	
	function appendData($data) {
		$this->data.=$data;
	}
	
	// nb. not fully supported, remove everything for now..
	function deleteData($offset,$count) {
		$this->data='';
		NXDOM_Document::_updateNode($this);
	}

	function toString() {
		return "<b>{$this->tagName}</b> value=[{$this->data}] id={$this->id} parent={$this->parent} type={$this->nodeType} baseURI={$this->baseURI}";
	}

	function toXML() {
		return '' ;
	}
}

class NXDOM_NodeList {
	var $list;
	var $length;
	
	function __construct(&$list) {
		$this->list=&$list;
		$this->length=count($list);
	}
	
	function &item($i) {
		if (isset($this->list[$i]))
			return $this->list[$i];
		return null;
	}
	
	function &items() {
		return $this->list;
	}
}

// node collection shared by all DOM documents ("static" variable)
$GLOBALS['_NXDOM_nodes']=array();

class NXDOM_Document extends NxDOM_Node
{
	var $documentElement=null;
	var $documentURI=null;
	
	function __construct()
	{
		parent::__construct('#document',$this,null);
		$this->nodeType=DOCUMENT_NODE;
	}
	
	function & createElement($qname,$att=null) {
		return new NxDOM_Element($qname,$att,$this,$baseURI);
	}
	
	function &createElementNS($baseURI,$qname,$att=null) {
		return new NxDOM_Element($qname,$att,$this,$baseURI);		
	}
	
	function createTextNode($text,$baseURI)
	{
		return new NxDOM_Text($this,$text,$baseURI);
	}
	
	function &getParentElement(&$node) {
		$parentID=$node->parent;
		if ($parentID!=-1)
			return $GLOBALS['_NXDOM_nodes'][$parentID];
		else
			return null;
	}	
	
	static function _addNode(&$node)
	{
		$node->id=count($GLOBALS['_NXDOM_nodes']);
		$GLOBALS['_NXDOM_nodes'][]=$node;
	}

	// update the instance of this node in the node lookup
	static function _updateNode(&$node)
	{
		$GLOBALS['_NXDOM_nodes'][$node->id]=$node;
	}

	// update the instance of this node in the node lookup
	function _refreshNode($node=null)
	{
		if ($node)
			return @$GLOBALS['_NXDOM_nodes'][$node->id];
		return null;
	}

	static function __refreshNode($node=null)
	{
		if ($node)
			return @$GLOBALS['_NXDOM_nodes'][$node->id];
		return null;
	}

	// returns the base URI related to a suffix, in the context of a node
	function _getBaseURI($prefix,$node)
	{
		//nxltrace("DOM: searching uri for [$prefix], node=".$node->toString(true),'DOM');
		
		// walks backward until we find the prefix def
		$curNode=$node;
		
		while(isset($curNode->nodeType) /* && $curNode->nodeType!=DOCUMENT_NODE */) 
		{
			//nxltrace("checking node =".$curNode->toString(true));
			if ($curNode->nodeType==ELEMENT_NODE)
			{
				if ($curNode->prefix == $prefix && $curNode->baseURI!=null)
					return $curNode->baseURI;
					
				if($curNode->nsdef!=null)
					foreach ($curNode->nsdef as $ns)
					{
						//nxltrace("node defines ns[{$ns['prefix']}]={$ns['uri']}",'DOM');
						if ($ns['prefix']==$prefix)
							return $ns['uri'];
					}
			}
			$curNode=$this->getParentElement($curNode);
			if ($curNode===null)
				nxltrace("ERROR: prefix search:$prefix couldnt find parent node on node ".$node->toString(true),'error');
		}
		
		if (!isset($curNode))
			nxltrace("couldnt find uri for [$prefix] (no parent) on node ".$node->toString(true),'error');
		else
			nxltrace("couldnt find uri for [$prefix]: on node ".$node->toString(true).", last ancestor: ".$curNode->toString(),'error');
			
		return '';
	}
	
	function toString() {
		return "<b>Document:{$this->documentURI}</b> id={$this->id} parent={$this->parent} type={$this->nodeType} baseURI={$this->baseURI}";
	}

	function toXML() {
		return "[Document:{$this->documentURI}]";
	}

}

?>
<?PHP

include_once('NxDOM_Node'.NX_PHP_VER.'.inc');

class NxDOM_DomBuilder
{
	var $nod_stack;
	var $node;
	var $nodeLevel=0;
	var $isText=false;
	
	var $ns_uri_index=array();
	var $ns_index_uri=array();
	var $ns_stack=array();
	var $ns_map=array();
	
	function __construct()
	{
		$this->doc=new NXDOM_Document();
		
		$this->nod_stack=array();   // nested instruction stack
		$this->nod_stack[]=&$this->doc;
		$this->node=&$this->doc; // current instruction
		$this->nodeLevel=0;
		return true;
	}

	// =============== PARSING ===========================
	function &getCurrentNode()
	{
		return $this->node;
	}
		
	// obsolete ?
	function closeNodeTree() 
	{
		$this->doc=null;
		return true;
	}
	
	function & createNode($qname,&$att) {
			$qname=strtolower($qname);
		
			// push new NS definitions
			if (isset($att['_nsdef']))
			{
				$this->pushNS($att);
			}
							
			$this->getTypeInfo($qname,$prefix,$localName,$baseURI);
//PHP5			$node=& $this->doc->createElementNS($baseURI,$qname,$att);
			$node= $this->doc->createElementNS($baseURI,$qname,$att);
			
			if (isset($att['_nsdef']))
			{
				$node->setNSDef($att['_nsdef']);
			}
			
			if (isset($this->node))
			{
				$this->node->appendChild($node);
			}
			else
			{
				nxError("Syntax error in XML, can't add node $qname, unknown parent..","DOM");
			}
			
			// push nod_stack
			$this->nodeLevel++;
			$this->nod_stack[]=&$node;
			$this->node=&$node;		
									
			return $this->node;		
	}
	
	function closeNode() {
		// pop NS nod_stack
		if (isset($this->node->nsdef))
			$this->popNS();
	
		// pop nod_stack
		if (count($this->nod_stack)>0)
		{
			array_pop($this->nod_stack);		
			$this->node =& $this->nod_stack[count($this->nod_stack)-1];
			$this->nodeLevel--;
			
			// kill current node
			if (isset($this->node) && !$this->node->valid)
			{
				$this->node->_removeLastChild();	
			}
		}
		else
			nxError("Syntax error in XML, can't close node","DOM");

	}
	
	function setTextMode($isText=true)
	{
		$this->isText=$isText;
	}
	
	// append only text that is not white space, except if in "text mode" where everything is stored
	function appendText($cdata)
	{
		// if (trim($cdata)!='' || $this->isText)
		{
			if ($this->node->lastChild!=null && $this->node->lastChild->nodeType==TEXT_NODE)
				$this->node->lastChild->appendData($cdata);
			else
			{
				if (!isset($this->node))
				{
					nxError("pb with node");
				}
				else
				{
					$node=$this->createTextNode($cdata);
					$this->node->appendChild($node);
				}
			}
		}
	}
	
	function createTextNode($text)
	{
		$this->getTypeInfo('#text',$prefix,$localName,$baseURI);
		
		return $this->doc->createTextNode($text,$baseURI);
	}
	
	function invalidateNode()
	{
		$this->node->_invalidate();
	}
		
	function getNodeLevel()
	{
		return $this->nodeLevel;
	}
	
	function pushNS(&$atts)
	{
		$map=array();
		foreach ($atts['_nsdef'] as $ns)
		{
			$prefix=$ns['prefix'];
			$uri=$ns['uri'];
			
			// get ns index
			if (isset($this->ns_uri_index[$uri]))
			{
				$index=$this->ns_uri_index[$uri];
			}
			else
			{
				// add namespace uri to collections
				$index=count($this->ns_index_uri);
				$this->ns_uri_index[$uri]=$index;
				$this->ns_index_uri[]=array('uri'=>$uri,'prefix'=>$prefix);
			}
			
			// add ns to map
			$map[$prefix]=&$this->ns_index_uri[$index];
		}
		
		// complement with parent ns and push map
		$this->ns_map = $map + $this->ns_map;
		$this->ns_stack[]=$this->ns_map;
	}
	
	function popNS()
	{
		// pop ns map
		array_pop($this->ns_stack);
		if (($len=count($this->ns_stack))>0)
			$this->ns_map=$this->ns_stack[$len-1];
		else
			$this->ns_map=array();
	}
	
	function getTypeInfo($qname,&$prefix,&$localName,&$baseURI)
	{
		if (!preg_match('/^(?:([^:]+):)?(.+)$/',$qname,$matches))
		{
			$this->error(-1,'/* error : badly formed tag name */',$node['tag']);
			$localName=$qname;
			$prefix='';
		}
		else
		{
			// get tag
			$localName=$matches[2];
			
			// get namespace
			if (!$matches[1])
				$prefix='';
			else
				$prefix=$matches[1];
		}
		
		if (isset($this->ns_map[$prefix]))
		{		
			$baseURI=$this->ns_map[$prefix]['uri'];
		}
		else
		{
			$baseURI=null;
		}

		// if ($baseURI=='') nxltrace("can't find uri for node $qname");
	}
	
	function setNS(&$node)
	{
		if (!preg_match('/^(?:([^:]+):)?(.+)$/',$node->tagName,$matches))
		{
			$this->error(-1,'/* error : badly formed tag name */',$node['tag']);
			$tag=$node->tagName;
			$ns='';
		}
		else
		{
			// get tag
			$tag=strtolower($matches[2]);
			
			// get namespace
			if (!$matches[1])
				$ns='';
			else
				$ns=strtolower($matches[1]);
		}

		$node->_setBaseTag($tag);
		
		if (isset($this->ns_map[$ns]))
		{		
			$node->_setNS($this->ns_map[$ns]);
		}
	}
} 
?>
<?php 
// library for the creation xml objects (nxl templates)
include_once(NX_TAG_LIB.'w3/1999/xsl/Transform.inc');

class taglib_nxfrontier_xo_core extends taglib_w3_1999_xsl_Transform
{		
	// var $classSrcCurNode;
	
	function __construct(&$ctx,$uri='http://www.nxfrontier.com/xo/core') 
	{
		parent::__construct($ctx);
		$this->uri=$uri;
	}

	// define a new library
	function _taglib($node,$level)
	{
		// create a new library and register it with new namespace
		if (!$this->getAttribute($uri,$node,'uri'))
		{
			$uri=$this->context->dom->_getBaseURI('',$node);
			if ($uri=='')
				return 'no uri for taglib';
		}

		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
		
		nxltrace("XO taglib [$uri] - start, package=$package",'taglib');
		$this->context->addResource($uri,'xo_taglib_uri');	

		$this->getAttribute($replace,$node,'replace','false');
		$replace=$this->getBool($replace);
			
		if ($this->getAttribute($inherit,$node,'extends') || 
			$this->getAttribute($inherit,$node,'inherit') ||
			$this->getAttribute($inherit,$node,'implements'))
		{
			$loaded=$this->context->addLib($uri,new taglib_nxfrontier_xo_core($this->context,$uri),$replace);
			$parents=explode(',',$inherit);
			$model=array();
			foreach ($parents as $baseUri)
			{
				// clone/extends existing lib
				$baseUri=trim($baseUri);
				nxltrace("Inherit from taglib : $baseUri","trace");

				if ($baseUri!='')
				{
					$baselib=$this->context->getLib($baseUri);
					// $baselib->uri=$uri;
					// $loaded=$this->context->addLib($uri,$baselib,$replace);
					
					// add any tag added by tag library	(in XML format)		
					if(isset($this->context->models[$baseUri]))
					{
						foreach ($this->context->models[$baseUri] as $name=>$tag)
						{
							$model[$name] = $tag;
							nxltrace("add tag: $name to $uri from $baseUri","trace");
						}
					}
					// $this->context->models[$uri]=array()+$this->context->models[$baseUri];
				}								
			}
			$this->context->models[$uri]=$model;

		}
		else
		{
			$lib = new taglib_nxfrontier_xo_core($this->context,$uri);
			$loaded=$this->context->addLib($uri,$lib,$replace);
		}
			
		if ($loaded)
		{
			// preload site specific libraries (preload attribute)
			$this->loadConfig($node);
		}
		
		$this->context->addResource($loaded,'xo_taglib_loaded');	

		return true;
	}
	
	function taglib($node,$content,$level,$pad)
	{
		// pop URI
		$uri=$this->context->getLastResource('xo_taglib_uri',true);

		// pop variable: is lib loaded
		$loaded=$this->context->getLastResource('xo_taglib_loaded',true);
	
		if ($loaded && $this->isTagInLib__('on_load',$uri))
		{
			// map lib for polymorphism with "this" ns mapped to current taglib
			$store_lib=&$this->context->mapURI('http://www.nxfrontier.com/xo/this',$uri);

			// load tag lib constructor (nb. will get taglib node/attributes)
			$package=$this->context->getLastResource('package',false);

			nxltrace("TAGLIB CONSTRUCTOR - START (package=$package)",'taglib');
			$this->openCallTemplate__($node,'on_load',$uri);
			$this->execCallTemplate__('on_load',$uri,$node,'',0);
			nxltrace('TAGLIB CONSTRUCTOR - END','taglib');

			// pop lib
			$this->context->restoreLibUri('http://www.nxfrontier.com/xo/this',$store_lib);
		}
		
		nxltrace("XO taglib - end",'taglib');

		// pop package
		$this->context->getLastResource('package',true);
		
		return $content;
	}

	// do not execute template at parsing time
	function _tag()
	{
		return NX_TL_IGNORE_CONTENT;
	}

	// prepare tag and store it in model for later execution by xo:default
	function tag(&$node,$content,$level,$pad)
	{
		// store model node
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('missing name attribute');
			
		// get tag uri (default is tag def node uri)
		if(!$this->getAttribute($uri,$node,'uri') || $uri=='')
		{
			if ($node->baseURI!='http://www.nxfrontier.com/xo/core' ||
				($uri=$this->context->getLastResource('xo_taglib_uri',false))==null)
			{
				// xo:tag within tag lib => use taglib uri
				// xo:tag outside taglib => use xo
				// other prefix => use ns for this prefix
				$uri=$this->uri;			
			}
		}
			
		$name=strtolower($this->parseVal($name,false));
		
		// store current node for class_param function
		$node->classParamNode=$this->getSrcCurrentNode();
		
		// store doc URI (path) for resource deployment in html
		$node->setDeftAtt('doc_uri',$this->context->dom->documentURI);
		$node->setDeftAtt('_is_xsl_template','yes');
				
		// $this->models[$mod]=$node;
		$this->context->models[$uri][$name]=$node;
		nxltrace("DEFINE TAG <strong>{$node->prefix}:$name</strong> ns={$uri}",'taglib');
		return '';
	}
		
	// object : execute template
	function _object_(&$node)
	{
		// bak attributes
		$node->_refreshNode();
		
		if($this->getAttribute($uri,$node,'uri'))
			;
		elseif ($this->getAttribute($object,$node,'select')
			|| $this->getAttribute($object,$node,'object'))
		{
			if ($nodeList=&$this->xpath->getNodesByXpath($this->srcCurNode,$object)
				&& ($srcNodes=$nodeList->items()) != null
				&& isset($srcNodes[0])
				&& ($srcNodes[0]->nodeType==ELEMENT_NODE))
			{
				$object=$srcNodes[0];	
				if (isset($object->baseURI))
					$uri=$object->baseURI;
				else
					return $this->fatal('invalid "object"/"uri" attribute');
			}
		}
		else
			$uri=$node->baseURI;
			
		// get tag to call
		if (!$this->getAttribute($name,$node,'function')
			&& !$this->getAttribute($name,$node,'method')
			&& !$this->getAttribute($name,$node,'class')
			&& !$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing "method" attribute');
			
		$name=strtolower($this->parseVal($name,false));

		// map lib for polymorphism with "this" ns mapped to current taglib
		$node->store_lib=&$this->context->mapURI('http://www.nxfrontier.com/xo/this',$uri);

		return $this->openCallTemplate__($node,$name,$uri);
	}

	function object_($node,$content,$level,$pad)
	{		
		if(!$this->getAttribute($uri,$node,'uri'))
			$uri=$node->baseURI;

		// get template
		if (!$this->getAttribute($name,$node,'function')
			&& !$this->getAttribute($name,$node,'method')
			&& !$this->getAttribute($name,$node,'class')
			&& !$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing "class" attribute');
			
		$name=strtolower($this->parseVal($name,false));
		$res= $this->execCallTemplate__($name,$uri,$node,$content,$level);

		// pop lib
		$this->context->restoreLibUri('http://www.nxfrontier.com/xo/this',$node->store_lib);
		unset($node->store_lib);

		// restore attributes
		$node->_restoreNode();
		
		return $res;
	}

	function load_config(&$node)
	{		
		return $this->loadConfig($node);
	}

	function _param(&$node)
	{
		$this->getAttribute($name,$node,'name','unknown');

		nxltrace("START xo:param : $name",'walker');
	}

	
	/* overload xslt param, check attribute and real param.. */
	function param($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
		
		// get param value if exist
		$value=$this->context->getParam($node,$name);
		
		// if param value doesn't exist get attribute value
		if ($value===null)
		{
			nxltrace("no param value => load default...",'walker');
			
			// get value from attribute
			$this->getAttribute($value,$this->srcCurNode,$name);
		}
		
		// if param value doesn't exist get default value from select or default attribute
		if ($value==null)
		{
			// get value
			if ($this->getAttribute($sel,$node,'select'))
			{
				// node set
				//$value=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
				$value=$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
			}
			else
			{
				$this->getAttribute($value,$node,'value',$content);
			}
		}
		
		// get type
		if ($this->getAttribute($type,$node,'type') && $type=='string' && !preg_match("/[$']/",$value))
		{
			$value=trim($value,"'");
			$value="'$value'";
		}
		
		// set as a variable
		$this->context->setVariable($name,$value);
		nxltrace("  => xo:param $name=[$value]",'walker');
		
		return '';
	}

	// remap a namespace to a different lib
	function _map_uri(&$node)
	{
		if (!$this->getAttribute($uri,$node,'uri'))
			return $this->fatal('no "uri" attribute');
		
		if (!$this->getAttribute($target,$node,'target'))
			return $this->fatal('no "target" attribute');

		nxltrace("&lt;map_uri uri=\"$uri\" target=\"$target\" &gt;",'walker');
			
		// map lib for polymorphism with "this" ns mapped to current taglib
		$node->store_lib=$this->context->mapURI($uri,$target);
		
		return '';
	}
	function map_uri(&$node,$content,$level,$pad)
	{
		if (!$this->getAttribute($uri,$node,'uri'))
			return $this->fatal('no "uri" attribute');

		// map lib for polymorphism with "this" ns mapped to current taglib
		$this->context->restoreLibUri($uri,$node->store_lib);
		
		nxltrace("&lt;/map_uri&gt;",'walker');
		
		return $content;
	}
	
	// ============== stack facility ======================
	function push($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
		
		// get value
		if ($this->getAttribute($sel,$node,'select'))
		{
			// node set
			$value=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		}
		else
		{
			$this->getAttribute($value,$node,'value',$content);
		}
		
		// set as a variable
		$stack=&$this->context->getStack($name);
		$stack->push($value);
		
		nxltrace("&lt;xo:push/output name=\"$name\" value=\"$value\"/&gt;",'taglib');		
		return '';
	}

	function current($node,$content,$level,$pad)
	{
		// param name
		if (!($this->getAttribute($name,$node,'name') ||
			$this->getAttribute($name,$node,'p0')))
			return $this->fatal('missing name attribute');
		
		// set as a variable
		$stack=&$this->context->getStack($name);
		$ret=$stack->current();
		
		if ($ret==null)
		{
			if (!$this->getAttribute($ret,$node,'default'))
				$this->getAttribute($ret,$node,'p1');
				
		}
		
		nxltrace("&lt;xo:current name=\"$name\"/&gt; => [$ret;]",'taglib');
		return $ret;
	}

	// get stack content with a given separator between each value
	function get_stack_content($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');

		// how to flatten the stack
		$this->getAttribute($sep,$node,'separator','');

		$this->getAttribute($flush,$node,'flush','false');
		$flush=$this->getBool($flush);
		
		$stack=&$this->context->getStack($name);	
		$ret=$stack->getAll($sep,$flush);

		nxltrace("&lt;xo:get_stack_content stack name=\"$name\" separator=\"$sep\" flush=\"$flush\"/&gt; value=[$ret]",'taglib');
		return $ret;
	}
	
	function pop($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
		
		// set as a variable
		$stack=&$this->context->getStack($name);
		$ret=$stack->pop();

		nxltrace("&lt;xo:pop name=\"$name\"/&gt;> popped value=[$ret;]",'taglib');
		return '';
	}
	
	// resource facility (access by key)
	function get_last_resource($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');

		$this->getAttribute($removeKey,$node,'remove','no');
			
		return $this->context->getLastResource($name,$this->getBool($removeKey));
	}
	
	function add_resource($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');

		$this->getAttribute($key,$node,'key');

		// get value
		if ($this->getAttribute($sel,$node,'select'))
		{
			// node set
			$value=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		}
		else
		{
			$this->getAttribute($value,$node,'value',$content);
		}
			
		$this->context->addResource($value,$name,$key);
		return '';
	}
	
	function output($node,$content,$level,$pad)
	{
		return $this->push($node,$content,$level,$pad);
	}
	
	function get_output($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');

		// how to flatten the stack
		$this->getAttribute($sep,$node,'separator','');

		$this->getAttribute($flush,$node,'flush','true');
		$flush=$this->getBool($flush);
		
		$stack=&$this->context->getStack($name);	
		$ret=$stack->getAll($sep,$flush);

		nxltrace("&lt;xo:get_output stack name=\"$name\" separator=\"$sep\" flush=\"$flush\"/&gt; value=[$ret]",'taglib');
		return $ret;
	}

	function apply_templates($node,$content,$level,$pad)
	{
		// get xpath
		$sel=$node->getAttribute('select','.');
		$nodeList=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		
		if (!$nodeList || count($nodeList->items())==0)
			$nodeList=&$this->xpath->getNodesByXpath($node,'./*');
		
		$src='';
		foreach($nodeList->items() as $srcNode)
		{
			switch($srcNode->nodeType)
			{
				case ELEMENT_NODE:
					$src.=$this->context->processNode($srcNode,$level);
					break;
					
				case TEXT_NODE:
					switch($srcNode->data)
					{
						case '#text':
							// text()
							// $src.=$this->walkChildren_(&$node,$level);
						case '#value':
							// value()
						default:
							// genuine text node
							$src.=$this->walkChildren_($srcNode,$node,$level);
					}					
				case ATTRIBUTE_NODE:
					$src.=$this->walkChildren_($srcNode,$node,$level);
					break;
				default:
					break;
			}			
		}
		return $src;
	}

	
	//========= user defined tags =========
	
	// process tag by trying nxl lib first and then builtin tag handlers
	function __process_tag($node,$content,$level,$pad,$prefix,$tag,$mode)
	{
		if ($this->getAttribute($debug,$node,'debug') && ($debug=='yes'||$debug=='true'))
			DebugBreak();

		$uri=$this->uri;
		$name=strtolower($node->localName);

		// check lib
		if (!isset($this->context->models[$uri]))
		{
			$this->context->getLib($uri);
		}

		// bak attributes
		if ($mode=='_')
			$node->_refreshNode();
		
		// is there a template for the tag opening (header) defined in model
		if ($node->nodeType!=TEXT_NODE && 
			(isset($this->context->models[$uri]["_{$name}"]) ||
			isset($this->context->models[$uri][$name]) ||
			isset($this->context->models[$uri]["{$mode}tag_default"]) ||
			isset($this->context->models[$uri]["tag_default"])))
		{
			nxltrace("Calling NXL LIB handler for tag: {$mode}{$name}",'debug');
			
			// call nxl tag handler
			$method="{$mode}tag_default";
			
			$res = $this->$method($node,$content,$level,$pad,$prefix,$tag);
			if ($mode!='_')
				$node->_restoreNode();

			return $res;
		}
		else
		{
			// built in tag handler..
			nxltrace("Calling PHP LIB handler for tag: {$mode}{$name}",'debug');
			$res='';

			$method=$mode.$this->getHandlerName($tag);
			if (method_exists($this,$method))
			{
				$res = $this->$method($node,$content,$level,$pad);
			}
			
			if ($mode!='_')
				$node->_restoreNode();

			return $res;
		}
						
		return;
	}
	
	// map tag name to its handler
	// php symbols maped to symbol_, #name to name_, a-name to a_name
	function getHandlerName($tag) {
		return preg_replace(
			array('/_(comment|text)$/','/^[#](.*)$/','/[# .-]/',
				'/^(include|unset|require|namespace|echo|script|foreach|while|break|continue|if|else|class|object|function|return|exit|array|extends)$/'),
			array('$1_','$1_','_','$1_'),
			$tag);
	}
	
	function _tag_default($node,$level)
	{
		// $uri=$node->baseURI;
		$uri=$this->uri;
		$name=strtolower($node->localName);
		
		// map lib for polymorphism with "this" ns mapped to current taglib
		$node->store_lib=$this->context->mapURI('http://www.nxfrontier.com/xo/this',$this->uri); /* -REF */
		
		// bak attributes
		$node->_refreshNode();

		if($node->getAttribute('package'))
		{
			$package=$this->decodePackage($node,'inherit');
			$this->context->addResource($package,'package');
		}

		$ret=$this->openCallTemplate__($node,$name,$uri);

		
		// pop lib
		$this->context->restoreLibUri('http://www.nxfrontier.com/xo/this',$node->store_lib);
		unset($node->store_lib);
		
		return $ret;
	}
		
	function tag_default($node,$content,$level,$pad,$ns=null,$op=null)
	{	
		// $uri=$node->baseURI;
		$uri=$this->uri;
		// template name is tag name..
		$name=strtolower($node->localName);

		// map lib for polymorphism with "this" ns mapped to current taglib
		$node->store_lib=$this->context->mapURI('http://www.nxfrontier.com/xo/this',$this->uri); /* -REF */

		$res=$this->execCallTemplate__($name,$uri,$node,$content,$level);

		if($node->getAttribute('package'))
		{
			// get currenty package (and pop package resource)
			$this->context->getLastResource('package',true);
		}

		$node->_restoreNode();

		// pop lib
		$this->context->restoreLibUri('http://www.nxfrontier.com/xo/this',$node->store_lib);
		unset($node->store_lib);
		
		return $res;
	}
	
	// lib constructor
	function on_load(&$node)
	{
		nxltrace('user tag lib is loaded: '.$this->uri,'taglib');
		return '';
	}

	// object : holds tag attributes (avoid using xoc tags)
	function params($node,$content,$level,$pad)
	{
		return '';
	}

	// ========= space management ==========
	
	// function to be called from taglib
	function preserve_spaces($node) {
		// get value
		$this->getAttribute($keep,$node,'value','true');
		if ($this->getBool($keep)==false)
			return '';
			
		// get xpath
		$sel=$node->getAttribute('select','.');
		$nodeList=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		
		foreach($nodeList->items() as $srcNode)
		{
			switch($srcNode->nodeType)
			{
				case ELEMENT_NODE:
					$srcNode->setAttribute('_keepAllText','true');
					break;				
				default:
					break;
			}			
		}
		return '';
	}

	// ============ TEXTS AND COMMENTS ==============
	function _comment_($node)
	{	
		$this->keepAllText($node,true);
	}
	
	/* NB. comment are converted to _comment element */
	function comment_($node,$content,$level,$pad)
	{	
		if (@$content[0]!='[')
			return '';

		if ($this->getAttribute($v,$node,'value'))
			$content=$v.$content;

		return '';		
		// return "<!--{$content} -->";		
	}
	
	// text node: returns text
	// NB. get rid of blank text except if parent requests to keep it by 
	// setting attribute _keepAllText to "true".
	function _text_() {
	}
	
	function text_($node)
	{	
		if (trim($node->data)==='')
		{
			$parent=$this->context->getParentElement($node);
			if (isset($parent->nodeType) && $parent->nodeType==ELEMENT_NODE && $parent->getAttribute('_keepAllText')=='true')
				return $node->data;
			else
				return '';
		}
		
		return $node->data;
	}

	
	//======== function handlers ==========	
	function _func_param($node,$args,$level=0)
	{
		return $this->_func_xpath($node,$args,$level);
	}
	
	// get xpath value for tags dynamically created (ie. when executing another tag)
	function _func_class_param($node,$args,$level=0)
	{	
		// set xsl context node to the one it was when class was defined
		$prev=$this->setSrcCurrentNode($this->classSrcCurNode);
		$ret=$this->_func_xpath($node,$args,$level);
		$this->setSrcCurrentNode($prev);
		
		// $this->context->addResource("XO class_param({$args[0]}): [$node->prefix] = [$ret]",'taglib');
		
		return $ret;
	}
	
	// get xpath value for tags dynamically created (ie. when executing another tag)
	function _class_value()
	{
	}	
	function class_value($node,$content,$level,$pad)
	{
		$sel=$node->getAttribute('select','.');
		
		// set xsl context node to the one it was when class was defined
		$prev=$this->setSrcCurrentNode($this->classSrcCurNode);
		$ret = $this->_func_xpath($node,array($sel),$level);
		$this->setSrcCurrentNode($prev);
			
		return $ret;
	}
	
	// ============ instruction block ==================
	
	// NXL instruction can declare themselves as instruction blocks
	function declareInstructionBlock(&$node)
	{
		// declare parent node as block
		$this->srcCurNode->setAttribute('_isblock','true');
	}

	
}
	
?>
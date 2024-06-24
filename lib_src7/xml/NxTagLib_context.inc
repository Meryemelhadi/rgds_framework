<?PHP

include_once(NX_LIB.'collections/NxStack.inc');
include_once(NX_DIS.'taglib/NxTagLib.inc');
include_once('NxDOM_xpath.inc');

// singleton
$GLOBALS['XSL_context_stack']=array('open'=>new NxStack(),'current'=>new NxStack(),'srcNodes'=>new NxStack());
$GLOBALS['XSL_context_libs']=array();
$GLOBALS['XO_user_stacks']=array();

class NxTagLib_context
{
	var $verbose=false;

	// tag libs can use this array for storing their models (XSL template, XO tags, etc.)
	// to be used as $this->context->models[$uri][$tplID] in tag libs
	var $models=array();
	var $files=array();
		
	function __construct(&$props,&$dom,$tagLibPath,$verbose=false)
	{
		$this->props=&$props;
		$this->fs=$props->getPlugIn('FileSys');
		
		$this->tagLibPath=array();
		foreach (preg_split("/[,;]+/", $tagLibPath) as $path)
		{
			$this->tagLibPath[]=rtrim($path,'/').'/';
		}
		$this->rootPath=null;
		
		$this->verbose=$verbose;
		$this->res=array();
		$this->dom = &$dom;
		
		// compil variables/parameters (used by XSL, XO, etc.)
		// their scope is local to a tag (template)
		// $this->varStack=array('open'=>new NxStack(),'current'=>new NxStack());
		$this->varStack=&$GLOBALS['XSL_context_stack'];
		$this->userStacks=&$GLOBALS['XO_user_stacks'];
		
		$this->xpath=new NxDOM_xpath($this);					
		$this->lib=&$GLOBALS['XSL_context_libs'];
	}

	// ============= XPATH TREE WALK  ================
	
	// NB. new functions to replace "gen" versions
	function walkNode($srcNode,$level=0)
	{
		$src='';
		nxltrace('walk node:[start]'.$srcNode->toString(),'debug');
		switch($srcNode->nodeType)
		{
			case ELEMENT_NODE:
				//$src=$this->walkTree($srcNode,$level);
				//break;
				// element
			case DOCUMENT_NODE:
				// document
				
				$src=$this->walkChildrenNodes($srcNode,$level);
				break;
			case ATTRIBUTE_NODE:
				$src=$srcNode->value;
				break;
			case TEXT_NODE:
				switch($srcNode->data)
				{
					case '#value':
						// value()
						$src=$this->walkTree($srcNode,$level);
					case '#text':
						// text()
					default:
						// genuine text node
						$src=$srcNode->data;
				}
			default:
				break;
		}
		nxltrace('walk node:[end]'.$srcNode->toString(),'debug');
		return $src;
	}
	
	function walkTree(&$node,$level=0)
	{
		return $this->processNode($node,$level);
	}
	
	function walkChildrenNodes(&$node,$level=0)
	{
		return $this->processChildren($node,$level);
	}
	
	function genTree(&$node,$padLevel=0)
	{	
		// push document working dir
		$this->addResource(dirname($node->documentURI).'/','doc_wd');
				
		// process doc
		$ret = $this->processChildren($node,$padLevel);
			
		// pop doc wd
		$this->getLastResource('doc_wd',true);
	
		return $ret;
	}
	
	// ============= DOM like functions ============
	
	// obsolete
	function & getDOM() 
	{
		return $this->dom;
	}
	function & getDOMDoc() {
		return $this->dom;
	}
	
	function &getParentElement($node)
	{
		return $this->dom->getParentElement($node);
	}
	
	
	// ============ SRC NODE STACK ==============
	
	function pushSrcNode(&$srcCurNode)
	{
		$this->varStack['srcNodes']->push($srcCurNode);
	}
	
	function popSrcNode()
	{
		return $this->varStack['srcNodes']->pop();
	}

	function &getIterSrcNode()
	{
		if (isset($this->varStack['srcNodes']))
		{
			return $this->varStack['srcNodes']->iter();
		}

		nxltrace("XPATH getIterSrcNode = NULL");
		return null;
	}

	// get parent element in source tree (need to jump over template nodes)
	function &getParentElementSrc($node,&$iter,$moveCur=true)
	{
		$parent=&$this->dom->getParentElement($node);
		if ($parent!=null && $parent->nodeType==NODE_TYPE_ELEMENT 
			&& $parent->getAttribute('_is_xsl_template')=='yes')
		{
			$parent=&$iter->previous($moveCur);
		}
		
		return $parent;
	}
	
	// ============= XSL variable stack ==============
	
	function pushVarFrame()
	{
		// move current param/vars context from "open" to "current"
		$this->varStack['current']->push($this->varStack['open']->pop());
	}
	
	function popVarFrame()
	{
		$this->varStack['current']->pop();
	}
	
	function newVarFrame()
	{
		$this->varStack['open']->push(array('vars'=>array()));
	}
	
	function setVariable($name,$value)
	{
		$varCtxt=&$this->varStack['current']->current();
		if (isset($varCtxt['vars'][$name]))
			$res=$varCtxt['vars'][$name];
		else
			$res=null;

		$varCtxt['vars'][$name]=$value;
		return $res;
	}
	
	// get xsl variable
	// NB. the scope of variables is recursive like in Pascal language
	// => different from XSLT standard..
	function getVariable($name)
	{
		$varCtxt=&$this->varStack['current']->current();
		
		if (isset($varCtxt['vars'][$name]))
			return $varCtxt['vars'][$name];

		// error varaible not found..
		nxltrace("unknown variable name:$name",'error');
		return null;
	}
	
	// xsl param
	function setParam($callerNode,$name,$value)
	{
		$varCtxt=&$this->varStack['open']->current();
		$varCtxt['vars'][$name]=$value;
	}
	
	// get xsl variable
	// NB. the scope of variables is recursive like in Pascal language
	// => different from XSLT standard..
	function getParam($templateNode,$name)
	{
		$varCtxt=&$this->varStack['current']->current();
		
		if (isset($varCtxt['vars'][$name]))
			return $varCtxt['vars'][$name];
			
		return null;
	}
	
	// ============ USER STACKS =================
	function &getStack($name)
	{
		if (!isset($this->userStacks[$name]))
			$this->userStacks[$name]=new NxStack();
			
		return $this->userStacks[$name];
	}

	// =========== INSTRUCTION BLOCK ============
	
	// instruction can declare themselves as block 
	// => children work as statements, not expressions
	function declareBlock(&$node,$isBlock=true)
	{
		// instruction or expression ?
		$node->setAttribute('_isblock',$isBlock?'true':'false');
	}
	
	// check if parent is an instruction block
	function isInBlock($node,$parent=null)
	{
		// instruction or expression ?
		if ($parent==null)
			$parent=$this->getParentElement($node);
			
		if ($parent==null || $parent->nodeType!=ELEMENT_NODE)
			return false;
			
		return $this->isBlock($parent);
	}

	function isBlock($node)
	{
		if ($node->baseURI == 'http://www.w3.org/1999/XSL/Transform')
		{
			return $this->isInBlock($node);
		}

		if ($node->getAttribute('_isblock','false')=='true')
			return true;
		else
			return false;
	}
	
	// ============ TAGLIB MANAGEMENT ===========
	// load lib
	function getPathByURI($uri)
	{
		if (!isset($uri))
			return null;

		// get uri and package
		$_uri = explode('@',$uri);
		if(count($_uri)>1)
		{
			$uri=$_uri[0];
			$package=$_uri[1];
		}
		else
		{
			$package='';
		}

				
		// get lib path and class name
		if (!preg_match('#^((?:https?|ftp|file)[:]//(?:[^.]+)[.]([^/.]+)[.](?:[^/]+))/(.+)$#',$uri,$matches))
		{
			nxltrace("ERROR: wrong syntax for file uri:[$uri]",'error');
			return null;
		}
				
		$domain=trim($matches[1]);
		$base_domain=trim($matches[2]);
		$file_path=$matches[3];
		$path=trim($matches[2].'/'.$matches[3]);

		if(!$package)
			$package=$this->getLastResource('package',false);
			
		foreach ($this->tagLibPath as $libRoot)
		{
			if ($package)
			{
				$libRootPackage=str_replace('nxl/','packages/'.$package.'/nxl/',$libRoot);
				if ($libRootPackage != $libRoot)
				{
					$fbase="{$libRootPackage}{$path}";
					if (is_file("$fbase.nxl"))
					{
						return "$fbase.nxl";
					}
					if (is_file("$fbase.inc"))
					{
						return "$fbase.inc";
					}
				}
			}

			$fbase="{$libRoot}{$path}";
			if (is_file("$fbase.nxl"))
			{
				return "$fbase.nxl";
			}
			if (is_file("$fbase.inc"))
			{
				return "$fbase.inc";
			}
		}

		// now try to get it directly from the internet
		if (defined('NXL_LIBS_DOWNLOAD') && NXL_LIBS_DOWNLOAD!==false)
		{
			$repPath=NXL_LIBS_DOWNLOAD.$path;
			if (is_file($repPath.'.nxl'))
				return $repPath;
				
			$src=file_get_contents($uri.'.nxls');
			if ($src)
			{
				$fs=$this->props->getPlugIn('FileSys'); 
				if($fs->saveFile($repPath.'.nxl',$src))
					return $repPath;
			}
		}
		
		nxltrace("INCLUDE FILE: can't find file for [$uri]",'error');		
		return null;
	}
	
	// load lib
	function & getLib($uri)
	{
		if (!isset($uri))
			$uri='http://www.nxfrontier.com/nxl/default';
				
		if (!isset($this->lib[$uri]))
		{		
			nxltrace("TAGLIB: loading lib [$uri]...",'taglib');
			
			// get lib path and class name
			if (!preg_match('@^((?:https?|ftp|file)[:]//(?:[^.]+)[.]([^/.]+)[.](?:[^/]+))/([^#]+)(.+)?$@',$uri,$matches))
			{
				nxltrace("ERROR: wrong syntax for lib uri:[$uri]",'error');
				return null;
			}
				
			$domain=trim($matches[1]);
			$base_domain=trim($matches[2]);
			$file_path=$matches[3];
			$path=trim($matches[2].'/'.$matches[3]);
			$sel=$matches[4];
			if (/*$sel=='#skin' && */ isset($GLOBALS['NX_SKIN_PATH']))
				$tagLibPath=array_merge($GLOBALS['NX_SKIN_PATH'],$this->tagLibPath);
			else
				$tagLibPath=$this->tagLibPath;
								
			$libClass = "taglib_".preg_replace('@[# /.-]@','_',$path);
			
			foreach ($tagLibPath as $libRoot)
			{
				$fbase="{$libRoot}{$path}";
				if (class_exists($libClass) ||
					(is_file("$fbase.inc") &&
					($success=include_once("$fbase.inc"))))
				{
					$this->lib[$uri]=new $libClass($this);
					nxltrace("TAGLIB: finished loading native lib [$uri] : $fbase",'taglib');
					break;
				}
				elseif ($this->loadNXLLib($uri,$fbase)!==null)
					break ;
			}
			
			if (!isset($this->lib[$uri]))
			{
				$fbase=$this->getPathByURI($uri);
				if (!$fbase || $this->loadNXLLib($uri,$fbase)==null)
					nxltrace("TAGLIB: can't find lib:[$uri]",'error');
			}
		}
		
		return $this->lib[$uri];
	}
	
	// add a new librariy if not exist or if "replace" is requested
	function addLib($uri,&$lib,$replace=false)
	{
		if (!isset($this->lib[$uri]) || $replace)
		{
			$this->lib[$uri]=&$lib;
			nxltrace("TAGLIB: added new NXL lib [$uri]",'taglib');
			return true;
		}
		return false;
	}

	// load a user defined lib
	function loadNXLLib($uri,$fbase)
	{
		$lpath="$fbase.nxl";
		if (!is_file($lpath))
			return null;
		 
		// get included source (hopefully with requested taglib...)
		$this->processFragment($lpath,'/',0,null,false);

		// return taglib if now loaded...
		if (isset($this->lib[$uri]))
		{
			nxltrace("TAGLIB: loaded nxl lib [$uri] from file $lpath",'taglib');
			$this->lib[$uri]->addIncludePath(dirname($fbase));
			return $this->lib[$uri];
		}
		else
		{
			nxltrace("TAGLIB: can't find lib [$uri]",'taglib');
			return null;
		}
	}

	/* map a uri to a target uri. Allows an xml tree with a URI to be processed by a target lib */
	function mapURI($uri,$targetURI,$load=true)
	{
		// check if target lib exists
		if (!isset($this->lib[$targetURI]) && $load)
			$this->getLib($targetURI);
		
		// if uri to map already exists, returns it (to unmap the uri..)
		if (isset($this->lib[$uri]))
			$prevLib=$this->lib[$uri];
		else
			$prevLib=null;
			
		// map uri to the target uri or remove uri (in case of error, better fail than work w/ wrong lib..)
		if (isset($this->lib[$targetURI]))
			$this->lib[$uri]=$this->lib[$targetURI];
		else
			unset($this->lib[$uri]);
			
		nxltrace("mapped uri $uri to $targetURI class=". get_class($this->lib[$uri]),'walker');
			
		// return previous mapping
		return $prevLib;
	}

	// restore a lib to its original uri
	function & restoreLibUri($uri,$lib)
	{			
		$this->lib[$uri]=$lib;
		// nxltrace("restored uri {$lib['uri']} to $uri class=". get_class($lib),'walker');
	}
	
	function processFragment($fpath,$xpath,$padLevel,$rootPath=null,$bind=true)
	{
		// parse (SAX) the file and generate DOM doc
		$xml_parser = new NXL_parserNXL();
		
		if ($xml_parser->parseFile($fpath)) {
			$dom=$xml_parser->getDOMDoc();
		}
		else
			$dom=null;

		$xml_parser->close();
		$xml_parser=null;
				
		// process DOM by wlaking nodes and applying taglib statements
		if ($dom!=null)
		{
			// attach document node to its parent if any (useful when using xpath with ../.. expressions)
			if ($bind && isset($this->currNode))
				$this->currNode->appendChild($dom);
				
			// push the root of this file (root stack)
			$save_rootPath=$this->rootPath;
			$this->rootPath=$rootPath;	

			// push current dom (dom doc stack)
			$this->domStack[]=&$this->dom;
			
			// push document working dir
			$this->reformatPath(dirname($fpath).'/',$wd);
			$this->addResource($wd,'doc_wd');
			
			$this->dom=&$dom;
			
			// get part of the tree to walk
			$xpath = trim($xpath,'/');
			if ($xpath=='')
			{
				// process fragment
				$ret = $this->processChildren($dom,$padLevel);
			}
			else
			{
				// with xpath
				$nodeList=&$this->xpath->getNodesByXpath($dom,$xpath);
				$ret=$this->processNodes($nodeList->items(),$padLevel);
			}
			
			// pop doc wd
			$this->getLastResource('doc_wd',true);
			
			// pop dom doc
			$this->dom=&array_pop($this->domStack);
			
			// pop root path
			$this->rootPath=$save_rootPath;		
		}
		else
		{
			nxltrace('processing fragment: NXL SAX parser returns no DOM');
			$ret='';
		}

		return $ret;
	}
	
	// process collection of nodes
	function processNodes($nodes,$level) 
	{
		$src='';
		foreach($nodes as $srcNode)
		{
			switch($srcNode->nodeType)
			{
				case ELEMENT_NODE:
					$src .= $this->processChildren($srcNode,$level);
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
							$src.=$srcNode->data;
					}					
				case ATTRIBUTE_NODE:
					$src.=$srcNode->value;
					break;
				default:
					break;
			}			
		}
		return $src;
	}
	
	function processFragmentSource($src,$xpath,$padLevel,$rootPath=null,$bind=true)
	{
		// parse (SAX) the file and generate DOM doc
		$xml_parser = new NXL_parserNXL();
		
		if ($xml_parser->parse($src)) {
			$dom=$xml_parser->getDOMDoc();
		}
		else
			$dom=null;

		$xml_parser->close();
		$xml_parser=null;
		
		// attach document node to its parent if any (useful when using xpath with ../.. expressions)
		if ($bind && isset($this->currNode))
			$this->currNode->appendChild($dom);
			
		// push the root of this file (root stack)
		$save_rootPath=$this->rootPath;
		$this->rootPath=$rootPath;	
		
		// process DOM by wlaking nodes and applying taglib statements
		if ($dom!=null)
		{
			// push current dom (dom doc stack)
			$this->domStack[]=&$this->dom;
			
			// push document working dir
			$this->addResource('/','doc_wd');
			
			$this->dom=&$dom;
			
			// process fragment
			$xpath = trim($xpath,'/');
			if ($xpath=='')
			{
				// process fragment
				$ret = $this->processChildren($dom,$padLevel);
			}
			else
			{
				$nodeList=&$this->xpath->getNodesByXpath($dom,$xpath);
				$ret=$this->processNodes($nodeList->items(),$padLevel);
			}
			
			// pop doc wd
			$this->getLastResource('doc_wd',true);
			
			// pop dom doc
			$this->dom=&array_pop($this->domStack);
		}
		else
			$ret='';
			
		// pop root path
		$this->rootPath=$save_rootPath;
		return $ret;
	}
	
	// ============== TAG EXECUTION ==============
		
	// generate statement code: intruction + embedded content
	function processChildren(&$node,$level)
	{
		if ($node==null)
			return '';
			
		$src='';	

		// foreach ($node->getChildNodes() as $child)
		$children=$node->getChildNodes();
		// $len=count($children);
		//for ($i=0;$i<$len;$i++)
		foreach ($children->items() as $childNode)
		{
			// $src.=$this->processNode($children[$i],$level);
			$src.=$this->processNode($childNode,$level);
		}
		
		return $src;
	}
	
	// process node and its children, returns generated source
	function processNode(&$node,$level)
	{
		nxltrace('Process node:'.$node->toString(),'debug');
		
		if (trim($this->openNode($node,$level))!==NX_TL_IGNORE_CONTENT 
			&& $node->hasChildNodes())
			$content = $this->processChildren($node,$level+1);
		else
			$content='';
		
		return $this->processNodeSrc($node,$content,$level);
	}
	
	// function called before processing children nodes
	function openNode(&$node,&$level)
	{
		// get tag and ns
		if ($node->baseURI===null)
		{
			return null;
		}
					
		nxltrace($node->toXML(true),'walker',1);
			
		$this->tag=$tag=$node->localName;
		
		$pad=$this->getPad($level);
		$lib=&$this->getLib($node->baseURI);
		if ($lib===null)
		{
			return null;
		}
		
		$this->currNode=&$node;
	
		if (!isset($lib) || !method_exists($lib,'getAttribute'))
		{
			nxltrace("TAGLIB: Error cant find lib $uri : value=".$lib." class=".get_class($lib),'error');
			return null;
		}

		// breakpoint management
		if (($lib->getAttribute($break,$node,'breakpoint') || 
			$lib->getAttribute($break,$node,'breakpoint'))&& $lib->getBool($break))
			$lib->_breakpoint_($node);
			
		// call handler
		return $lib->__process_tag($node,'',$level,$pad,$node->prefix,$tag,'_');
			
		//$this->currNode=&$currNode;
	}
	
	// search for the instruction handler and call it
	function processNodeSrc(&$node,$content,$level)
	{
		// apply xsl:attribute changes
		$node->_refreshNode();
		$this->tag=$tag=$node->localName;
		
		// bug ?
		//$this->currNode=&$node;
		$prefix=$node->prefix;
		$pad=$this->getPad($level);
		$lib=&$this->getLib($node->baseURI);
		if ($lib===null)
		{
			$ret=$this->lib_unknown($node,$content,$level,$pad,$prefix,$tag);
			nxltrace($node->toXML(false),'error',-1);
			return $ret;
		}
		$this->currNode=&$node;
			
		// breakpoint management
		if (($lib->getAttribute($break,$node,'breakpoint') || 
			$lib->getAttribute($break,$node,'breakpoint'))&& $lib->getBool($break))
			$lib->_breakpoint_($node);
			
		$this->libName=get_class($lib);
		
		// call handler
		$ret=$lib->__process_tag($node,$content,$level,$pad,$prefix,$tag,'');
		
		nxltrace($node->toXML(false),'walker',-1);
		
		// restore initial node state (useful if node is walked again, in template)
		$node->_restoreNode();
		return $ret;
	}
	
	// get indentation pad
	function getPad($level)
	{
		return str_pad('', $level, '	', STR_PAD_LEFT);
	}

	// ------------ error handlers ----------------
	function code_unknown($node,$content,$level,$pad,$ns,$op)
	{
		$msg="ERROR: no handler for instruction: {$ns}:{$op} on node ".$node->toString();
		nxltrace($msg,'error');
		return "/* $msg */";
	}
	
	function lib_unknown($node,$content,$level,$pad,$ns,$op)
	{
		$msg="no library handler for instruction: {$ns}:{$op} on node ".$node->toString();
		nxltrace($msg,'warning');
		
		if ($node->nodeType!=ELEMENT_NODE)
			return '';
			
		if ($ns)
			$name="$ns:$op";
		else
			$name=$op;
		
		if ($uri=$node->getAttribute('uri',null))
			$uri=" xmlns=\"{$uri}\"";
		else
			$uri='';
		
		// copy attributes	
		$atts='';
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($an='uri')
				continue;
				
			// $av=$this->parseVal($av,false);			
			$atts .=" $an=\"$av\"";
		}
		
		// copy ns
		if ($node->nsdef)
			foreach ($node->nsdef as $ns)
			{
				if ($ns['prefix'])
					$atts .=" xmlns:{$ns['prefix']}=\"{$ns['uri']}\"";
				else
					$atts .=" xmlns=\"{$ns['uri']}\"";
			}

		if ($content!='')
			return "\n{$pad}<{$name}{$atts}{$uri}>$content\n{$pad}</$name>";
		else
			return "\n{$pad}<{$name}{$atts}{$uri}/>";
	}
	
	// -------- lib actions -----------
	
	// get lib and pass the expression to it
	function evalFunc($prefix,$expr,$uri=null)
	{		
		// get uri from prefix
		if ($uri==null)
			$uri=$this->dom->_getBaseURI($prefix,$this->currNode);
			
		nxltrace("getting library: prefix=[$prefix], uri=[$uri] for function $expr on node: ".$this->currNode->toString(),'taglib');
		
		// get lib
		$lib=&$this->getLib($uri);
		
		if (!isset($lib))
		{
			nxltrace("failure in loading library: prefix=[$prefix], uri=[$uri]",'TAGLIB');
			return $this->lib_unknown($this->currNode,'',0,'',$prefix,$expr);
		}
			
		// exec lib function
		//nxLog("expression to evaluate:[$expr]",'XSL');
		return $lib->_expression_($this->currNode,$expr,$uri,$this->dom,$prefix);
	}
	
	// error management
	function libWarning($msg)
	{
		echo "{$this->libName}::{$this->tag}, warning: $msg\n";
		return '';
	}
	
	function libFatalError($msg)
	{
		$lib=get_class($this->lib);
		
		$src ="===&gt;Error in {$this->libName}::{$this->tag}, $msg\n";	
		$src.=htmlentities(NxTagLib::passThrough($this->currNode,'...','',false));
		nxltrace($src,'error');
		return null;
	}	
	
	// ============== toolbox for libraries ==============
	
	/* function called by parseVal*/
	function lib_get($ns,$op,$name,$val)
	{
		$node=array('tag'=>'get','name'=>$name,'default'=>'$val');
		
		if (!$lib=&$this->getLib($ns))
		{
			return $this->lib_unknown($node,$content,$level,$pad,$ns,$op);
		}
			
		if (method_exists($lib,$op))
			return $lib->$op($node,'',0,'');
		else
			return $this->code_unknown($node,'',0,'',$ns,$op);		
	}
	
	/** save content to file */
	function saveSrc($path,$src) 
	{
		$this->fs->saveFile($path, $src,true);
	}
	function removeFile($path) 
	{
		$this->fs->delete($path);
	}
	
	/** returns the root of the tree where the current processed file is */
	function getCurrentRootPath()
	{
		if ($this->rootPath!= null)
			return $this->rootPath;
		else
			return $this->tagLibPath[0];
	}

	// returns the document directory
	function getWorkingDir()
	{
		// get base dir
		return dirname($this->dom->documentURI).'/';
	}
	function getCurrentDir()
	{
		// get base dir
		return $this->getLastResource('doc_wd');
	}
	
	function getFileURI()
	{
		return $this->dom->documentURI;
	}
	
	// reformat path, returns number of chars removed from '../' parts
	function reformatPath($path,&$path2)
	{		
		// reformat path
		$removedlen=0;
		$apath2=array();
		$apath=explode('/',str_replace('\\','/',$path));
		foreach($apath as $d)
		{
			if ($d=='.') continue;
			if ($d=='..')
			{
				// remove parent dir
				$removedlen+=strlen(array_pop($apath2))+1;
			}
			else
				$apath2[]=$d;
		}
		$path2=implode('/',$apath2);
		return $removedlen;
	}
	
	
	// =========== DIR/FILE DEPLOYMENT ==========
	function deployFile($from,$to,$wd=null)
	{
		if ($wd===null)
			$wd=$this->getWorkingDir();		
		
		$this->reformatPath($wd.$from,$path);
		$s ="deploying $path to $to";
					
		if($this->fs->copy($path,$to))
		{
			$res=true;
			nxltrace($s .' : <strong>OK</strong>');
		}
		else
		{
			$res=false;
			nxltrace("$s : <strong>FAILED (in {$this->dom->documentURI})</strong>",'error');
		}
			
		nxltrace($s,'compil');
		return $res;
	}
	
	function deployDir($dir,$re,$toDir,$levels=1,$wd=null)
	{
		if ($wd===null)
			$wd=$this->getWorkingDir();
			
		$lenwd=strlen($wd);
		
		$lenwd-=$this->reformatPath(rtrim($wd.$dir,'\\/').'/',$baseDir);
		
		$toDir=rtrim($toDir,'/\\').'/';
		
		$s="deploying dir $baseDir with [$re] to $toDir<ul>";
		foreach($this->fs->listFilesRegEx($baseDir,$re,$levels) as $f)
		{
			$to=$toDir.substr($f,$lenwd);
			$s .="<li>deploy file:$f to $to";			
			if($this->fs->copy($f,$to))
				$s .= ' : <strong>OK</strong>';
			else
				$s .= ' : <strong>FAILED</strong>';
		}
		nxltrace("$s</ul>",'compil');
	}
	
	function deleteFile($fpath,$wd=null)
	{		
		$s ="deleting $fpath";
					
		if($this->fs->delete($fpath))
		{
			$res=true;
			nxltrace($s .' : <strong>OK</strong>');
		}
		else
		{
			$res=false;
			nxltrace("$s : <strong>FAILED (in {$this->dom->documentURI})</strong>",'error');
		}
			
		nxltrace($s,'compil');
		return $res;
	}
	
	function cleanDir($toDir,$re,$levels=1)
	{		
		$toDir=rtrim($toDir,'/\\').'/';
		
		$s="deleting dir $toDir $re<ul>";

		if (!$re)
		{
			$s .="<li>delete dir:$toDir";
			if (!is_dir($toDir))
				$s .= ' : <strong>NOT FOUND</strong>';
			elseif($this->fs->rmDir($toDir))
				$s .= ' : <strong>OK</strong>';
			else
				$s .= ' : <strong>FAILED</strong>';			
		}
		else foreach($this->fs->listFilesRegEx($toDir,$re,$levels) as $f)
		{
			$s .="<li>delete file:$f";
			if($this->fs->delete($f))
				$s .= ' : <strong>OK</strong>';
			else
				$s .= ' : <strong>FAILED</strong>';
		}
		nxltrace("$s</ul>",'compil');
	}	
	
	function trace($msg)
	{
		// $this->res['compil'][]=$msg;
		// echo "$msg<br>\n";
		nxltrace($msg,'compil');
	}
	
	function addInclude($path,$base='NX_LIB') {
		$this->res['import'][$path]="require_once({$base}.'$path');";
	}

	function addHeaderCode($src) {
		static $codeID=0;
		$this->res['import']['code_'.$codeID]=$src;
		$codeID++;
	}

	//============= STACK MANAGEMENT ==============
	/* a resource is a variable or anything that neeeds to be passed between one tag and another.
	 * a resource belongs to a category (ex. includes, trace, etc.) and is stored in an array
	 * that can be treated as a stack (add/getLast) or /associative/ array (getResource, getResources),
	 */
	
	// add a resource to be used by top tags (example: imports, etc.)
	function addResource($val,$type='includes',$key=null)
	{
		if ($key)
			$this->res[$type][$key]=$val;
		else
			$this->res[$type][]=$val;

		if ($type=='package')
		{
			nxltrace("push resource $type, key=$key, value=<strong>$val</strong>, depth=".count($this->res[$type]),'package');
		}
	}

	// add a set of resources to be used by top tags (example: imports, etc.)
	function addResources($arr,$type='includes')
	{
		$this->res[$type]=$arr;
	}
	
	// return array of resources as array (if $flatten = null) or
	// as a single string if a $sepflatten is a character.
	function getResources($type='includes',$sepflatten=null,$removeKey=false)
	{
		if (isset($this->res[$type]))
			$res=$this->res[$type];
		else
			$res=array();
		
		if ($removeKey)
			unset($this->res[$type]);
			
		if (!is_string($sepflatten))
			return $res;
			
		return implode($sepflatten,$res);
	}
	
	// return a resources by key
	function getResource($type,$key,$removeKey=false)
	{
		if (isset($this->res[$type][$key]))
		{
			$res=$this->res[$type][$key];
			if ($removeKey)
				unset($this->res[$type][$key]);

			if ($type=='package')
			{
				if ($removeKey)
					nxltrace("pop resource $type, key=$key, value=<strong>$res</strong>$val, depth=<strong>".count($this->res[$type]).'</strong>','package');				
			}
							
			return $res;
		}

		nxltrace("cant find resource $type, key=$key",'debug');
		return null;
	}

	// return a last resource
	function getLastResource($type,$removeKey=false)
	{
		if (!isset($this->res[$type]) || ($len=count($this->res[$type])) ==0)
		{
			nxltrace("cant find resource $type",'debug');
			return null;
		}

		$keys=array_keys($this->res[$type]);	
		$lastk=$keys[$len-1];
		return $this->getResource($type,$lastk,$removeKey);
	}

	
	function removeResource($type='includes')
	{
		if (isset($this->res[$type]))
			unset($this->res[$type]);
	}
}

?>
<?PHP

// static class variable
$GLOBALS['XSL_srcCurNode']=null;
$GLOBALS['XO_classSrcCurNode']=null;

class taglib_w3_1999_xsl_Transform extends NxTagLib
{
	var $apply=false;
	var $models=array();
	var $srcCurNode;
	
	function __construct(&$ctx)
	{
		parent::__construct($ctx);
		$this->srcCurNode=&$GLOBALS['XSL_srcCurNode'];
		$this->classSrcCurNode=&$GLOBALS['XO_classSrcCurNode'];
	}
	
	function _template()
	{
		return NX_TL_IGNORE_CONTENT;
	}

	function isBlock($node)
	{
		return $this->isInBlock($node);
	}
	
	// <xsl:pi name='xml'>version="1.0" encoding="UTF-8"</xsl:pi>
	function pi($node,$content,$level,$pad)
	{
		// store model node
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('missing name attribute');
				
		nxltrace("XSL PI: [$name $content]",'compil');
		
		// return model content
		return '<?'."$name $content ".'?>'."\n";
	}	
	
	// <xsl:template name="model1">
	function template($node,$content,$level,$pad)
	{
		if ($this->apply==false)
		{
			// store model node
			if (($name=$node->getAttribute('name'))===null)
				return $this->fatal('missing name attribute');
				
			$this->models[$name]=&$node;
			nxltrace("XSL template definition: [$name]",'compil');
			return '';
		}
		
		// return model content
		return $content;
	}
		
	// <xsl:call-template name="model1"/>
	function call_template($node,$content,$level,$pad)
	{
		$this->apply=true;
		
		// get template
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('missing name attribute');
			
		if (!isset($this->models[$name]))
			return $this->fatal('unknown named template');
			
		$model=&$this->models[$name];
		
		// walk the template
		return $this->walkChildren_($this->context->getParentElement($node), //src node
									$model, //template
									$level-1);
	}
	
	function _with_param()
	{
	}
	
	function with_param($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
		
		// get value
		if ($this->getAttribute($sel,$node,'select'))
		{
			// init srcCurNode when not in a tag lib
			if (!isset($this->srcCurNode))
				$srcCurNode=$this->context->getParentElement($node);
			else
				$srcCurNode=$this->srcCurNode;
			
			// node set
			$value=&$this->xpath->getNodesByXpath($srcCurNode,$sel);
		}
		else
		{
			// content value
			$this->getAttribute($value,$node,'value',$content);
		}
		
		$this->context->setParam($node,$name,$value);
		nxltrace("&lt;with-param $name=[$value] /&gt;",'taglib');
		
		return '';
	}
	
	function _param(&$node)
	{
	}
	
	function param($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
		
		// get param value if exist
		$value=$this->context->getParam($node,$name);
		
		// if param value doesn't exist get default value
		if ($value===null)
		{
			// get value
			if ($this->getAttribute($sel,$node,'select'))
			{
				// node set
				$value=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
			}
			else
			{
				// content value
				$value=$content;
			}
		}
		
		// set as a variable
		$this->context->setVariable($name,$value);
		nxltrace("  => param $name=[$value]",'walker');
		
		return '';
	}
	
	function _element(&$node)
	{
		// keep name so taht attribute can be added later..
		if ($this->getAttribute($name,$node,'name'))
			$node->_name=$name;
		else
			$node->_name=null;
		
		$node->removeAttribute('name');
	}
	
	function element($node,$content,$level,$pad)
	{
		// param name
		if (!isset($node->_name))
			return $this->fatal('missing name attribute');
		$name=$node->_name;
			
		if ($this->getAttribute($ns,$node,'namespace'))
			$ns=" xmlns=\"$ns\"";
		else
			$ns='';

		// copy attributes
		$atts='';
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($an=='uri')
				continue;
				
			$av=$this->parseVal($av,false);
				
			$atts .=" $an=\"$av\"";
		}
			
			
		return "<$name$atts$ns>$content</$name>";
	}

	function _comment($node,$content,$level,$pad)
	{
		$this->keepAllText($node,true);
	}
	
	function comment($node,$content,$level,$pad)
	{
		return "<!--{$content}-->";
	}

	function attribute($node,$content,$level,$pad)
	{
		// param name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');

		// walks backward until we find a parent not in xsl or xo uri
		$curNode=$node;
		$foundNode=null;
		
		while(isset($curNode->nodeType) /* && $curNode->nodeType!=DOCUMENT_NODE */) 
		{
			//nxltrace("checking node =".$curNode->toString());
			if ($curNode->nodeType==ELEMENT_NODE)
			{
				if (//$curNode->baseURI != null && 
					(($curNode->baseURI!='http://www.w3.org/1999/XSL/Transform' &&
					$curNode->baseURI!='http://www.nxfrontier.com/xo/core') || $curNode->localName=='element' )
					)
					{
						$foundNode=$curNode;
						break;
					}
			}
			$curNode=$this->getParentElement($curNode);
			if ($curNode===null)
				nxltrace("ERROR: couldnt find parent node for node ".$node->toString(),'error');
		}
		
		if (!isset($foundNode))
			nxltrace("couldnt find element to apply attribute $name on",'error');
		else
		{
			nxltrace("Applying attribute $name=\"$content\" on node ".$node->toString(),'walker');
			$foundNode->setAttribute($name,$content);
		}
	
		return '';
	}
	
	// <xsl:foreach select="/parts/part">
	function _foreach_()
	{
		return NX_TL_IGNORE_CONTENT;
	}
	
	function foreach_($node,$content,$level,$pad)
	{
		return $this->for_each($node,$content,$level,$pad);
	}
	
	function _for_each()
	{
		return NX_TL_IGNORE_CONTENT;
	}
	function for_each($node,$content,$level,$pad)
	{
		// get xpath
		$sel=$node->getAttribute('select','.');
		$nodeList=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		
		$src='';
		foreach($nodeList->items() as $srcNode)
		{
			switch($srcNode->nodeType)
			{
				case ELEMENT_NODE:
					// parse attributes
					foreach ($srcNode->_getAttributes($srcNode) as $an=>$av)
					{
						$av=$this->parseVal($av,false);
						$srcNode->setAttribute($an,$av);
					}
				
					$src.=$this->walkChildren_($srcNode,$node,$level);
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

	function _apply_templates()
	{
		return NX_TL_IGNORE_CONTENT;
	}

	function apply_templates($node,$content,$level,$pad)
	{
		// get xpath
		$sel=$node->getAttribute('select','.');
		$nodeList=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		
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
		
	function _variable()
	{
	}
	
	function variable($node,$content,$level,$pad)
	{
		// get variable name
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
		
		// get value
		if ($this->getAttribute($sel,$node,'select')||$this->getAttribute($sel,$node,'value'))
		{
			// node set
			$value=&$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		}
		else
		{
			$value=$content;
		}
		
		nxltrace(" => xsl:variable $name=[$value]",'walker');
		
		$this->context->setVariable($name,$value);
		
		return '';
	}
		
	function _value()
	{
	}
	
	function value_of($node,$content,$level,$pad)
	{
		// get xpath
		$sel=$node->getAttribute('select','.');
//		nxltrace("XSL:VALUE-OF xpath=[$sel] on node:".$this->srcCurNode->toString());
		$res=$this->_func_xpath($this->srcCurNode,array($sel),$level);
//		nxltrace("XSL:VALUE-OF ret=[$res]");
		return $res;
	}
	
	function value($node,$content,$level,$pad)
	{
		// get xpath
		$sel=$node->getAttribute('select','.');
		return $this->_func_xpath($this->srcCurNode,array($sel),$level);
	}

	function _if_($node,$level)
	{
		if (!$this->getAttribute($cond,$node,'test'))
			return NX_TL_IGNORE_CONTENT;
		
		if (!$this->checkCond($node,$cond))
			return NX_TL_IGNORE_CONTENT;
	}
	function if_($node,$content,$level,$pad)
	{
		if (($cond=$node->getAttribute('test'))==null)
		{
			return $this->fatal('missing "test" attribute');
		}

		nxltrace(" => xsl:if result[$content]",'walker');
			
		return $content;
	}
		
	function _text($node)
	{	
		$this->keepAllText($node,true);
	}
	
	function text($node,$content,$level,$pad)
	{
		return $content;
	}
	/*
		<xsl:choose>
		<xsl:when test="">
		</xsl:when>
		<xsl:otherwise>
		</xsl:otherwise>
	</xsl:choose>
	*/
	function _choose($node)
	{
		$this->context->addResource(false,'choose_case_ok');
	}
	function choose($node,$content,$level,$pad)
	{
		$this->context->getLastResource('choose_case_ok',true);
		nxltrace(" => xsl:choose result[$content]",'walker');
		return $content;
	}
	
	function _when($node)
	{
		if ($this->context->getLastResource('choose_case_ok',false)===true)
			// choose case already found
			return NX_TL_IGNORE_CONTENT;
			
		if (!$this->getAttribute($cond,$node,'test'))
			return NX_TL_IGNORE_CONTENT;
			
		if (!$this->checkCond($node,$cond))
			return NX_TL_IGNORE_CONTENT;
			
		// ok: set "choose_case_ok" condition to true
		$this->context->getLastResource('choose_case_ok',true);
		$this->context->addResource(true,'choose_case_ok');
	}
	function when($node,$content,$level,$pad)
	{
		// for debug only...
		if (($cond=$node->getAttribute('test'))==null)
			return $this->fatal('missing "test" attribute');
			
		// if we are here it means the condition is true...			
		return $content;
	}

	function _otherwise($node)
	{
		if ($this->context->getLastResource('choose_case_ok',false)===true)
			// choose case already found
			return NX_TL_IGNORE_CONTENT;
						
		// ok: set "choose_case_ok" condition to true
		$this->context->getLastResource('choose_case_ok',true);
		$this->context->addResource(true,'choose_case_ok');
	}
	
	function otherwise($node,$content,$level,$pad)
	{
		// if we are here it means the condition is true...			
		return $content;
	}
	
	//======== function handlers ==========
	function _func_xpath($node,$args,$level=0)
	{
		// get nodelist
		foreach($args as $xpath)
		{			
			if (($res=$this->xpath->walkNodesByXpath($this->srcCurNode,$xpath))!=null)
			{
				return $res;
			}
		}
		return '';
	}

	function & getSrcCurrentNode()
	{
		return $this->srcCurNode;
	}
	
	function & setSrcCurrentNode($srcNode)
	{
		$cur=$this->srcCurNode;
		$this->srcCurNode=$srcNode;
		return $cur;
	}
	
	// check xpath boolean expression
	function checkCond($node,$cond)
	{
		return $this->xpath->checkXpathCond($this->srcCurNode,$cond);
	}
	
	// walk template from "node" with given source node for xpath
	function walkChildren_($srcNode,$node,$level)
	{
		// push current node
		$prev=$this->srcCurNode;
		$this->srcCurNode=$srcNode;
		
		// exec foreach content
		$src=$this->context->walkChildrenNodes($node,$level);
		
		// pop current node
		$this->srcCurNode=$prev;
		return $src;
	}
	
	function walkTemplate_($srcNode,$node,$level)
	{
		// push current node
		$prev=$this->srcCurNode;
		$this->srcCurNode=$srcNode;
		
		// walk children
		$src=$this->context->walkChildrenNodes($node,$level);
		
		// pop current node
		$this->srcCurNode=$prev;
		return $src;
	}
	
	function isTagInLib__($tag,$uri,$load=true)
	{
		if (!isset($this->context->models[$uri]) && $load)
			$this->context->getLib($uri);
		
		// is there a template for the tag opening (header) defined in model
		return isset($this->context->models[$uri][$tag]);
	}

	function openCallTemplate__(&$node,$name,$uri)
	{
		if (($this->getAttribute($break,$node,'breakpoint') || 
			$this->getAttribute($break,$node,'breakpoint'))&& $this->getBool($break))
			$this->_breakpoint_($node);
				
		// parse attributes
		foreach ($node->_getAttributes() as $an=>$av)
		{
			$av=$this->parseVal($av,false);
			$node->setAttribute($an,$av);
		}
		
		$atts='';
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			$atts .=" $an=\"$av\"";
		}		
		
		nxltrace('Open template on node:'.$node->toString(),'debug');
		$ns=" uri=\"{$uri}\" class=\"{$this->uri}\" ";
		if ($name != $node->localName)
			nxltrace("<strong>&lt;{$name} ({$node->tagName}){$atts}$ns&gt;</strong>",'tag',1);
		else
			nxltrace("<strong>&lt;{$node->tagName}{$atts} $ns&gt;</strong>",'tag',1);
					
		// template name is tag name..
		// $name=strtolower($node->localName);
		$node->setAttribute('tag',$name);
		
		// check lib
		if (!isset($this->context->models[$uri]))
		{
			$this->context->getLib($uri);
		}
		
		// is there a template for the tag opening (header) defined in model
		if (isset($this->context->models[$uri]["_$name"]))
		{
			// $model=&$this->models[$mod];
			$model=&$this->context->models[$uri]["_$name"];
			
			// open new context for parameters (move to current when calling template)
			$this->context->newVarFrame();

			// exec tag opening
			$model2 = clone $model;
			$this->__runTag($node,$model2,'',0);
		}
		elseif (isset($this->context->models[$uri]["_tag_default"]))
		{
			// $model=&$this->models[$mod];
			$model=&$this->context->models[$uri]["_tag_default"];
			
			// open new context for parameters (move to current when calling template)
			$this->context->newVarFrame();

			// exec tag opening
			$model2 = clone $model;
			$this->__runTag($node,$model2,'',0);
		}
		
		// get template for tag body
		if (isset($this->context->models[$uri][$name]))
		{
			// $model=&$this->models[$mod];
			$model=&$this->context->models[$uri][$name];
			$model2 = clone $model;
		}
		elseif(isset($this->context->models[$uri]['tag_default']))
		{
			$model=&$this->context->models[$uri]['tag_default'];
			$model2 = clone $model;
		}
		else
		{
			nxltrace("error: unknown tag $name and no tag_default in lib..",'error');
			return;
		}
			
		// store model in node
		$node->_model=$model2;
		
		// open new context for parameters (move to current when calling template)
		$this->context->newVarFrame();
		
		// ignore tag content ?
		if (( //$this->getAttribute($iscontent,$model,'content') ||
			$this->getAttribute($iscontent,$model,'content','yes'))	&& $this->getBool($iscontent)==false)
		{
			return NX_TL_IGNORE_CONTENT;
		}
//		return NX_TL_IGNORE_CONTENT;
	}
	
	function execCallTemplate__($name,$uri,$node,$content='',$level=0)
	{		
		if (($this->getAttribute($break,$node,'breakpoint') || 
			$this->getAttribute($break,$node,'breakpoint'))&& $this->getBool($break))
			$this->breakpoint_($node);
		
		if (!isset($node->_model))
			return $this->fatal("unknown TAG:$name");
			
		// pop model
		$model=$node->_model;
		unset($node->_model);			
		
		// parse attributes
		/*
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			$av=$this->parseVal($av,false);
			$node->setAttribute($an,$av);
		}
		*/

		// set xpath context for class params (obsolete?)
		$prev=$this->classSrcCurNode;
		$this->classSrcCurNode=$model->classParamNode;
				
		$src=$this->__runTag($node,$model,$content,$level);
		
		/*
		// push doc URI for resources in the object
		$this->context->addResource($doc_uri=$model->getAttribute('doc_uri'),'doc_uri');
		
		// bug here?
		//$this->context->trace("WD (tag exec:$name)=".dirname($doc_uri));
		
		// get context node
		$srcNode=$node;
		
		// obsolete ?
		if ($this->getAttribute($sel,$node,'select'))
		{
			// add context from path to context node
			$srcNode=$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		}
		else
			$srcNode=$node;
		
		// push xpath variable array
		//$this->context->addResource(array(),'xsl_var_list');
		
		// move current param/vars context from "open" to "current"
		$this->context->pushVarFrame();
		
		// add sys variables
		$this->context->setVariable('_content_',$content);
		$this->context->setVariable('_level_',$level);
		
		// walk the template
		$src = $this->walkChildren_($srcNode, //src node
									$model,  //template
									$level-1);
		
		// clear current context
		$this->context->popVarFrame();
									
		// pop xsl variable list
		//$this->context->getLastResource('xsl_var_list',true);
		
		// pop doc URI
		$doc_uri=$this->context->getLastResource('doc_uri',true);	
		*/
			
		$this->classSrcCurNode=$prev;
		
		if (($this->getAttribute($trace,$node,'trace') || 
			$this->getAttribute($trace,$model,'trace'))&& $this->getBool($trace))
			$s='<ul><li>tag result: '.nl2br(htmlspecialchars($src,ENT_NOQUOTES)).'</ul><li>';
		else
			$s='';

									
		nxltrace("$s<strong>&lt;/{$name}&gt;</strong>",'tag',-1);
		// nxltrace("WD (end tag exec:$name)=".dirname($this->context->getLastResource('doc_uri',false)));
		return $src;
		// return $content;
	}
	
	function __runTag(&$node,&$model,$content,$level)
	{
		// push doc URI for resources in the object
		$this->context->addResource($doc_uri=$model->getAttribute('doc_uri'),'doc_uri');
		
//		nxltrace("__runTag {$node->localName}",'tag',-1);
		
		// bug here?
		//$this->context->trace("WD (tag exec:$name)=".dirname($doc_uri));
		
		
		// get context node
		$srcNode=$node;
		
		$this->context->pushSrcNode($srcNode);
		
		// obsolete ?
		if ($this->getAttribute($sel,$node,'select'))
		{
			// add context from path to context node
			$srcNode=$this->xpath->getNodesByXpath($this->srcCurNode,$sel);
		}
		else
			$srcNode=$node;
		
		// push xpath variable array
		//$this->context->addResource(array(),'xsl_var_list');
		$atts_model=$model->_getAttributes();
				
		// move current param/vars context from "open" to "current"
		$this->context->pushVarFrame();
		
		// add sys variables
		$this->context->setVariable('_working_dir_',$this->context->getWorkingDir());
		$this->context->setVariable('_content_',$content);
		$this->context->setVariable('_level_',$level);
		$this->context->setVariable('_view_',$this->context->getLastResource('_view_'));
		$this->context->setVariable('_view_id_',$this->context->getLastResource('_view_id_'));
		$this->context->setVariable('_path_',$this->context->getLastResource('_path_'));
		$model->setAttribute('_is_xsl_template','yes');
		
		// walk the template  // check...
		$src = $this->walkChildren_($srcNode, //src node
									$model,  //template
									$level-1);
		
		// clear current context
		$this->context->popVarFrame();

		$this->context->popSrcNode();
					
		$model->_setAttributes($atts_model);	
		
		// pop xsl variable list
		//$this->context->getLastResource('xsl_var_list',true);
		
		// pop doc URI
		$doc_uri=$this->context->getLastResource('doc_uri',true);
		
		return $src;
	}
}

?>
<?php

/* 
	xpath utils.
	
	NB. depends on DOM.
	
*/
class NxDOM_xpath
{
	var $doc;
	
	function __construct(&$context)
	{
		$this->context=&$context;
	}
	
	function walkTree(&$node,$level=0)
	{
		return $this->context->processNode($node,$level);
	}
	
	function walkChildrenNodes(&$node,$level=0)
	{
		return $this->context->processChildren($node,$level);
	}
	
	//======== xpath utils ==========
	function getNodesByXpath(&$baseNode,$xpath,$type='nodes')
	{
		if (!isset($baseNode))
		{
			$res=array();
			return new NXDOM_NodeList($res);
		}
		
		$srcNodeIter=$this->context->getIterSrcNode();			
		$xpath=trim($xpath);
		
		if (trim($xpath,"'")==$xpath)
			// path
			$items=explode('/',$xpath);
		else
			// string
			$items=array($xpath);
		
		// load current node(s)
		if (method_exists($baseNode,'items')) //get_class($baseNode)=='NXDOM_NodeList')
			$resultSet=$baseNode->items();
		else
			$resultSet=array($baseNode);			

		foreach($items as $item)
		{
			if ($item==='')
				continue;
				
			// extract condition if any
			if(preg_match('/^([^\[]+)\[(.+)\]$/U',trim($item),$matches)>0)
			{
				$item=$matches[1];
				$cond=$matches[2];
			}
			else
			{
				$item=trim($item);
				$cond=true;
			}
				
			$res=array();
			$position=0;
			$doc=&$this->context->getDOM();
			
			foreach ($resultSet as $node)
			{
				if (!isset($node))
					continue;
					
				if (method_exists($node,'toString'))
					nxltrace("XPATH WALK: apply $item on node ".$node->toString(),'debug');
				
				if ($node->nodeType==ELEMENT_NODE)
					$position++;
					
				// simple string?
				$s=trim($item,"'");
				if ("'{$s}'"==$item)
				{
					// string, returns it as text
					$res[]=new NxDOM_Text($doc,$s);
					continue;
				}
				
				// remove condition if any
				/*
				if(($pos=strpos($item,'['))!==false)
				{
					// conditional tag[cond]
					$cond=rtrim(substr($item,$pos+1),']');
					if($this->checkXpathCond($node,$cond))
						$res[]=&$node;
				}
				*/
				
				// is attribute (@attr)
				if ($item[0]=='@') 
				{					
					if ($node->nodeType==ELEMENT_NODE)
					{
						// is attribute
						$name=substr($item,1);
						if ($name=='*')
						{						
							foreach($node->_getAttributes() as $name=>$v)
							{
								$res[]=new NxDOM_Attr($name,$doc,$v,$node);
							}
						}
						elseif(($v=$node->getAttribute($name))!==null)
						{
							$res[]=new NxDOM_Attr($name,$doc,$v,$node);
						}
					}
					else
						nxltrace("cant apply $item on non element:".$node->toString());
				}
				elseif($item[0]=='$')
				{
					// variable
					$value=$this->context->getVariable(substr($item,1));
					
					// add up variable result to current res set
					if (isset($value))
						if (is_object($value) && method_exists($value,'items'))// (get_class($value)=='NXDOM_NodeList')
							$res=array_merge($res,$value->items());
						else
							$res[]=new NxDOM_Text($doc,$value);

				}
				elseif ($item=='.' || $item=='node()')
				{
					// current node
					if($this->checkXpathCond($node,$cond))
						$res[]=$node;
				}
				elseif ($item=='..' || $item=='parent()')
				{
					// parent node
					$parentNode=$this->context->getParentElementSrc($node,$srcNodeIter,true);
					//nxltrace('PARENT of node:'.$node->toString().' is '.$parentNode->toString());
					if($this->checkXpathCond($parentNode,$cond))
						$res[]=$parentNode;
				}
				elseif ($item=='*')
				{
					// get all element children
					$sel=$node->getElementsByTagName('*');
					foreach ($sel->items() as $sel)
					{
						if($this->checkXpathCond($sel,$cond))
							$res[]=$sel;
					}
					// $res=array_merge($res,$sel->items());
				}
				elseif ($item=='text()')
				{
					// get all text children
					$sel=$node->getElementsByTagName('#text');
					foreach ($sel->items() as $sel)
					{
						if($this->checkXpathCond($sel,$cond))
							$res[]=$sel;
					}
//					$res=array_merge($res,$sel->items());
				}
				elseif ($item=='position()')
				{
					// 1) if position() with no condition, then count position
					// within parent node set.
					// 2) if condition, then use position variable.
					// 3) if within foreach, depends onm context node
					// TBD. implement context node...
					if (($node->id==$baseNode->id) && $cond===true)
					{
						// find position of node within its parent scope
						$parentNode = $this->context->getParentElementSrc($node,$srcNodeIter,false);
						$children=$parentNode->getChildNodes();
						$pos=1;
						foreach ($children->items() as $child)
						{
							if ($child->id==$node->id)
								break;
								
							if ($child->nodeType==$node->nodeType)
								$pos++;
						}
					}
					else
						$pos=$position;
						
					nxltrace("XPATH walker for position()=[$pos]",'debug');
						
					$res[]=new NxDOM_Text($doc,$pos);
				}
				elseif ($item=='last()')
				{
					// 1) if last() with no condition, then count position
					// within parent node set.
					// 2) if condition, then use position variable.
					// 3) if within foreach, depends onm context node
					// TBD. implement context node...
					$last=0;
					if (($node->id==$baseNode->id) && $cond===true)
					{
						// find position of node within its parent scope
						$parentNode = $this->context->getParentElementSrc($node,$srcNodeIter,false);
						$children=$parentNode->getChildNodes();
						foreach ($children->items() as $child)
						{							
							if ($child->nodeType==$node->nodeType)
								$last++;
						}
					}
					else
					{
						$contextNode=$resultSet;
						foreach ($contextNode as $node2)
							if ($node2->nodeType==ELEMENT_NODE)
								$last++;
					}
					
					nxltrace("XPATH walker for last()=[$last]",'debug');
					$res[]=new NxDOM_Text($doc,$last);
				}
				elseif(($pos=strpos($item,'('))!==false)
				{
					// xpath functions
					if ($item=='value()')
					{
						$res[]=new NxDOM_Text($doc,'#value');
					}
					elseif ($item=='text()')
					{
						$res[]=new NxDOM_Text($doc,'#text');
					}
					elseif ($item=='name()')
					{
						if (isset($node->localName))
							$res[]=new NxDOM_Text($doc,$node->localName);
					}
					// other functions not yet supported..
					
					// PHP BUG with preg_match...
					
					elseif (//strpos($item,'substring')!==false && 
							preg_match('/(substring|substring-before|substring-after|translate|replace|matches|contains)\(([^,)]+),([^,)]+)(?:,([^,)]+))?\)/',$item,$p))
					{
						$arg1=$this->walkNodesByXpath($node,$p[2]);
						if ($arg1) 
						{
							if ($p[3]==(int)$p[3] || $p[3][0]=="'")
								$arg2=$p[3];
							else
								$arg2=$this->walkNodesByXpath($node,$p[3]);
								
							if (!empty($p[4]))
							{
								if ($p[4]==(int)$p[4] || $p[4][0]=="'")
									$arg3=$p[4];
								else
									$arg3=$this->walkNodesByXpath($node,$p[4]);							
							}
							else
								$arg3=null;
																
							switch($p[1])
							{
								case 'substring':
									if ($arg3!==null)
										$val=substr($arg1,$arg2-1,$arg3);
									else
										$val=substr($arg1,$arg2-1);
									break;
								case 'contains':
									$arg2=trim($arg2,"'");
									$val=(strpos($arg1, $arg2)!==FALSE)?true:false;
									break;
								case 'substring-before':
									$arg2=trim($arg2,"'");
									$pos = strpos($arg1, $arg2);
									if ($pos!==false)
										$val=substr($arg1,0,$pos);
									else
										$val='';
									nxltrace("substring-before($arg1,$arg2)=pos=$pos, val=$val",'listing');
									break;
								case 'translate':
									$val=strtr($arg1,$arg2,$arg3);
									nxltrace("translate($arg1,$arg2,$arg3)=$val");
									break;
								case 'replace':$val=str_replace($arg2,$arg3,$arg1);break;
								default:$val=null;
							}
							if ($val!==null)
								$res[]=new NxDOM_Text($doc,$val);
						}
					}
				}
				else
				{
					if ($node->nodeType==ELEMENT_NODE)
					{
						$sel=$node->getElementsByTagName($item);
						foreach ($sel->items() as $sel)
						{
							if($this->checkXpathCond($sel,$cond))
								$res[]=$sel;
						}
					}
					elseif ($node->nodeType==DOCUMENT_NODE)
					{
						$sel=$node->getChildNodes();
						foreach ($sel->items() as $sel)
						{
							if($sel->tagName==$item && $this->checkXpathCond($sel,$cond))
								$res[]=$sel;
						}
					}
					
//					$res=array_merge($res,$sel->items());	
				}	
			}
			nxltrace("XPATH walker for [$item]: found nodes:",'debug');
			if ($res) 
				foreach ($res as $node)
					if (method_exists($node,'toString'))
						nxltrace('XPATH:walker node:'.$node->toString(),'debug');
					else
						nxltrace('XPATH:walker node: unknown node type','debug');
			nxltrace("XPATH walker for [$item] (end)",'debug');
			
			$resultSet=$res;
		}
		return new NXDOM_NodeList($resultSet);
	}	
	
	function walkNodesByXpath($srcNode,$xpath)
	{
		$nodeList=$this->getNodesByXpath($srcNode,$xpath);

		if ($nodeList->length==0)
			return null;

		// walk nodes
		$src='';
		foreach($nodeList->items() as $node)
		{
			$src .= $this->context->walkNode($node,0);
		}
		
		return $src;
	}
			
	// check xpath boolean expression
	function checkXpathCond($node,$cond)
	{
		if ($cond===true)
			return true;
			
		//if (preg_match('/^([^=><!]+)([=><!]+)([^=><!]+)$/',$cond,$matches))

		if (preg_match('/^([^><!]+)\s*(?:([=><!]+)\s*([^\[\]=><!]+))$/',$cond,$matches))
		{
			// first operand
			$xp1=$matches[1];
			$v1=$this->walkNodesByXpath($node,$xp1);

			// second operand
			$xp2=$matches[3];
			$v2=$this->walkNodesByXpath($node,$xp2);

			// operator
			$op=$matches[2];
			
			switch ($op)
			{
				case '=':
					if ($v1==$v2)
						return true;
					break;
				case '!=':
					if ($v1!=$v2)
						return true;
					break;
				case '>':
					if ($v1>$v2)
						return true;
					break;
				case '<':
					if ($v1<$v2)
						return true;
					break;
				case '>=':
					if ($v1>=$v2)
						return true;
					break;
				case '<=':
					if ($v1<=$v2)
						return true;
					break;
				case '':
					if ($v1)
						return true;
					break;
			}
		}
		else
		{
			// no comparison, so only try to match xpath to nodes
			$nodeList=&$this->getNodesByXpath($node,$cond);
			if ($nodeList->length>0)
				return true;
			/*	
			$v1=$this->walkNodesByXpath($node,$cond);
			if($v1!=="'false'" && $v1!='')
				return true;
			*/
		}
		
		return false;
	}
	
}

?>
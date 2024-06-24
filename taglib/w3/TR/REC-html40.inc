<?PHP

include_once(NX_TAG_LIB.'w3/1999/xhtml.inc');

// library for the creation of html or xml views
// URI: http://www.w3.org/TR/REC-html40
class taglib_w3_TR_REC_html40 extends taglib_w3_1999_xhtml
{		
	var $def = array(
		'default'=>'indent=no',
		'html'=>'close=force;indent=no',
		'head'=>'close=force;indent=no',
		'title'=>'close=force;content=text;indent=no',
		'meta'=>'newline=yes;close=no;indent=no',

		'body'=>'close=force;indent=no',	
		'h1'=>'close=force;indent=no',	
		'h2'=>'close=force;indent=no',	
		'h3'=>'close=force;indent=no',	
		'h4'=>'close=force;indent=no',	
		'h5'=>'close=force;indent=no',	
		'br'=>'newline=yes;close=no;indent=no',
		'hr'=>'newline=yes;close=no;indent=no',
		'script'=>'close=force;content=text;indent=no',
		'iframe'=>'close=force;indent=no',
		'a'=>'close=force;content=text;indent=no',
		'span'=>'close=force;content=text;indent=no',
		'div'=>'close=force;content=text;indent=no',
		'b'=>'close=force;content=text;newline=no;indent=no;',
		'strong'=>'close=force;content=text;newline=no;indent=no;',
		'p'=>'close=force;content=text;indent=no',
		'textarea'=>'close=force;content=text;indent=no',
		'table'=>'indent=no',
		'tr'=>'close=force;indent=no',
		'td'=>'close=force;content=text;indent=no',
		'th'=>'close=force;content=text;indent=no',
		'img'=>'newline=no;close=no;indent=no;',
	);

	static $generated_scripts=array();

	function __construct(&$context)
	{
		parent::__construct($context);
	}
	
	function _html($node,&$level)
	{
		$level=0;
		$this->loadConfig($node);
		//$this->keepAllText($node,true);
	}

	function html($node,$content,$level,$pad)
	{
		return $this->passThrough($node,$content,$pad,true);
	}

	function _processElement(&$node)
	{		
		$node->removeAttribute('_tag');
		$node->removeAttribute('_pmethod');
		$node->removeAttribute('pmethod');
		$this->preprocessAttributes($node);	
	}
		
	function script_(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($language,$node,'language') && strtolower($language)=='php')
		{
			return '<?php '.$content . '?>';
		}
		
		if (($this->getAttribute($role,$node,'_role') || $this->getAttribute($role,$node,'role')) && $role=='design')
			return '';

		if (!$this->getAttribute($type,$node,'type'))
		{
			$node->setAttribute('type','text/javascript');
		}
		
		if (!$this->getAttribute($language,$node,'language'))
		{
			$node->setAttribute('language','Javascript');
		}
			
		$this->getAttribute($mode,$node,'static');
		if($mode=='static')
			$mode='@'; 
		else
			$mode='';

		$this->_processElement($node);
		if ($this->getAttribute($src,$node,'src'))
		{
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info,$mode);
			if ($url!=null)
				$src=$url;
				
			$node->setAttribute('src',$src);
		}
		
		if (trim($content) && !preg_match('/<--/',$content))
		{
			$content="\n<!--\n$content\n-->\n";
		}

		if ($this->getAttribute($add_to,$node,'add_to'))
		{
			if (!$this->getAttribute($id,$node,'id'))
				$id = 'main.js';

			if(!isset(self:$generated_scripts[$id]))
				self:$generated_scripts[$id] = $content;
			else
				self:$generated_scripts[$id] .= "\n<!--\n  SCRIPT $id added to $add_to \n-->\n".
					$content;

			return "\n<!--\n  SCRIPT $id added to $add_to \n-->\n";
		}
		elseif ($this->getAttribute($save_to,$node,'save_to'))
		{
			if(!isset(self:$generated_scripts[$save_to]))
				$this->saveScript($id,self:$generated_scripts[$save_to]);

			return "\n<!--\n  SCRIPT generated from $save_to \n-->\n";
		}
		else
		{
			// regular behaviour
			$this->context->indentHTML=true;
			return $this->passThrough($node,$content,$pad,true);
		}
	}	

	function saveScript($id,$content)
	{
		$package=$this->context->getLastResource('package',true);
		$fpath=GENERIS_PACKAGES.$package.'/views/default/res/js/'.trim($id,'/');

		// save class
		$this->context->saveSrc($fpath,$content);
	}
	
	function tag_default($node,$content,$level,$pad,$ns,$op)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$name=$node->tagName;
		$atts='';
		if ($name=='#text')
			return $node->data;

		// html tag acting as a container
		if ($this->getAttribute($tag,$node,'_pmethod',null)==null)
		{
			// html tag not flatten => can have some content
			return $this->passThrough($node,$content,$pad,true);
		}
		
		/* manages tags that should be closed (but are not always done in html 4.0) */
		$node->removeAttribute('_pmethod');
		$node->removeAttribute('_tag');
		$node->removeAttribute('pmethod');

		// flatten html (ie not normally closed..)
		if ($this->getAttribute($tag,$node,'tag',null)==null)
		{
			$this->preprocessAttributes($node);
			// opening tag
			$atts='';
			foreach ($node->_getAttributes($node) as $an=>$av)
			{
				if ($an=='nx_res_url')
					continue;
					
				$av=$this->parseVal($av,false);	
				$atts .=" $an=\"$av\"";
			}
			$name=strtolower($node->tagName);
			return "\n{$pad}<{$name}{$atts}>";
		}
		
		// else: closing tag (flatten)
		return '</'.strtolower($tag).'>';
	}
}

?>
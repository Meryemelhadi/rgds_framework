<?PHP

include_once(NX_TAG_LIB.'w3/1999/xhtml.inc');

// library for the creation of html or xml views
// URI: http://www.w3.org/TR/REC-html40
class taglib_nxfrontier_html_html extends taglib_w3_TR_REC_html40
{		
	function _nxl($node,$level)
	{
		$this->body='';
		$this->head='';
		
		// first time we go through this tree, store its doc ID (useful for tag objects/deployment)
		$node->setDeftAtt('doc_uri',$this->context->dom->documentURI);
		
		// push doc URI for resources
		$this->context->addResource($doc_uri=$node->getAttribute('doc_uri'),'doc_uri');
		$this->context->trace("WD (html open)=".dirname($doc_uri));
		
		// setup deployment
		// if(($isDeploy=$this->context->getLastResource('deploy',false))!==null)
		if ($isDeploy=(isset($_GET['deploy']) && $this->getBool($_GET['deploy'])))
		{
			// $this->workingDir=$this->getWorkingDir();
			
			// set destination dirs for deployment
			$this->defaultSkinDir=NX_CONF.'view/'.NX_SKIN_SERVER.'/';
			
			if(($skin=$this->context->getLastResource('skin',false))==null)
				$skin=NX_SKIN_SERVER;			
			$this->skinDir=NX_CONF."view/$skin/";
		}
		
		// do not walk children (because of xpath)
		return NX_TL_IGNORE_CONTENT;
	}
	
	function nxl($node,$content,$level)
	{			
		// define part of doc to return (nb. obsolete?)
		//$xpath=explode('/',$this->context->getLastResource('xpath',true));
		//$isFragt=($xpath[count($xpath)-1]=='body');
		
		// get nodes for xpath
		$xpath = trim($this->context->getLastResource('xpath',true),'/');
		
		// is document fragment (in this case take care or related resources..)
		$isFragt=($xpath !='');
		
		// body, parse the whole doc (for all required resources)
		if ($isBody=(strtolower($xpath)=='html/body'))
			$xpath='';
			
		$nodeList=&$this->xpath->getNodesByXpath($node,$xpath);
				
		// walk children and build content		
		$content='';
		foreach($nodeList->items() as $node)
		{
			// change indent settings
			$identHTML = $this->context->indentHTML;
			if (strtolower($node->localName)=='script')
				// cancel html indent for scripts
				$this->context->indentHTML=false;

			// walk node
			$content.=$this->context->walkNode($node,0);

			// restore indent
			$this->context->indentHTML=$identHTML;		
		}

		$ajs=$this->context->getResources('resource_js',null,true);
		$acss=$this->context->getResources('resource_css',null,true);
		$aimg=$this->context->getResources('resource_images',null,true);

		// deploy resources and add TPL {require:}
		$isDeploy=(isset($_GET['deploy']) && $this->getBool($_GET['deploy']));
		
		$res='';
		foreach ($ajs as $info) {
			$res.=$this->__deploy_res($info,$isDeploy,$isFragt);
		}
		
		foreach ($acss as $info) {
			$res.=$this->__deploy_res($info,$isDeploy,$isFragt);
		}
		
		foreach ($aimg as $info) {
			$res.=$this->__deploy_res($info,$isDeploy,$isFragt);
		}

		// pop doc URI
		$doc_uri=$this->context->getLastResource('doc_uri',true);
		$this->context->trace("WD (html close)=".dirname($doc_uri));
		
		// pop the body resource
		$body=$this->context->getLastResource('html.body',true);
		
		// whern body requested, only resturns it with required resources
		if ($isBody)
			return $res.$body;	
		else
			return $content;
	}
	
	function __deploy_res($info,$isDeploy,$isFragt)
	{
		if ($isDeploy)
		{
			// only BODY => add TPL {require:}
			if (isset($info['fromDir']))
			{
				$this->context->deployDir($info['fromDir'],$info['pattern'],$info['to'],1,$info['wd']);
			}
			else
			{
				$this->context->deployFile($info['from'],$info['to'],$info['wd']);
			}
		}
						
		if ($isFragt)
			// only BODY => add TPL {require:}
			return $info['require'];
		else
			return '';
	}
	
	function __process_res(&$node,$name,&$resID,&$info)
	{
		if (preg_match('/\$[_a-z]+(->|\[)/i',$name))
			// dynamic URL.. 
			return null;
			
		if (preg_match('/^[\']?(http|ftp)s?/i',$name))
			// fully specified resource 
			return null;

		// get actual skin
//		if ($this->getAttribute($isSkin,$node,'skin','false') && $isSkin='true')
		if (true)
		{
			// skin mode
			
			// get current skin as defined in tpl:view or tpl:views	parent	
			$isSkin='skin';
			$skin=$this->context->getLastResource('skin',false);
			if ($skin==null)
				$skin='default';			
		}
		else
		{
			// no skin
			$isSkin='';
			$skin='default';
		}
								
		// is it an absolute path ?
		if (preg_match('%^/((?:[^/]+/)*)(((images|css|js)/)((?:[^.](?!_(?:IE|NN|NS|OP|GK)))+[^.])(?:_((?:IE|NN|NS|OP|GK)[0-9_.]+))?[.]([^./]+))$%iU',
		$name,$matches))
		{	
			// absolute URL
			
			// "absolute" path => relative to NXL root
			$wd=$this->context->getCurrentRootPath();
			
			// extract skin and device
			$pathRel=$matches[1];
						
			$fullName=$matches[2];
			$type=$matches[4];
			$dir=$matches[3];
			$baseName=$matches[5];
			$isDevice=$matches[6]!=''?'device':'';
			$ext=$matches[7];
			// $isSkin=$pathRel==''?'skin':'';
			// $isSkin='';
			$f="{$dir}{$baseName}.{$ext}";	
		}
		elseif (preg_match('%((?:[.][.]/)*)(((images|css|js)/)((?:[^.](?!_(?:IE|NN|NS|OP|GK)))+[^.])(?:_((?:IE|NN|NS|OP|GK)[0-9_.]+))?[.]([^./]+))$%iU',
			$name,$matches))
		{	// relative URL
		
			// relative path => relative to doc or object dir
			// store it first time (in case replayed through xpath...)
			if (($uri=$node->getAttribute('doc_uri'))==null)
			{
				$uri=$this->context->getLastResource('doc_uri',false);
				$node->setAttribute('doc_uri',$uri);
			}		
			$wd=dirname($uri).'/';		
		
			// extract skin and device
			$pathRel=$matches[1];
			$fullName=$matches[2];
			$type=$matches[4];
			$dir=$matches[3];
			$baseName=$matches[5];
			$isDevice=$matches[6]!=''?'device':'';
			$ext=$matches[7];
			// $isSkin=$pathRel==''?'skin':'';
			// $isSkin='skin';
			$f="{$dir}{$baseName}.{$ext}";
		}
		else
			// where is this file coming from ?
			return null;
			
		// get actual skin
//		if ($isSkin=='skin')
		if (true)
		{
			// skin mode
			$viewDir=dirname("{$wd}{$pathRel}");
			
			// where to get the file: from skin dir if exists
			if(is_file("{$viewDir}/$skin/$fullName"))
			{
				// from skin
				$wd="$viewDir/$skin/";
				$srcDir=$dir;
			}
			else if (is_file("{$viewDir}/default/$fullName"))
			{
				// from default skin
				$wd="$viewDir/default/";
				$srcDir=$dir;
			}
			else
				// or from source file
				$srcDir="{$pathRel}{$dir}";
		}
		else
		{
			// no skin
			
			$srcDir="{$pathRel}{$dir}";	
			$isSkin='';
			$skin='default';
		}
		
		// TPL directive : "require" resource
		$res="{@require:$f";
		if ($isSkin!='' || $isDevice!='')
			$res.=":{$isSkin} {$isDevice}";
		$res.="}";
		
		// where to store it for TPL (runtime view system)
		$dpath=NX_CONF."view/$skin/res/";
					
		if ($isDevice!='')
		{
			// all device files in directory
			$resID = "$dpath/{$baseName}(_[^.]+)?[.]$ext";
			$info=array('fromDir'=>$srcDir,
						'wd'=>$wd,
						'to'=>$dpath,
//						'pattern'=>"%{$baseName}(_[^.]+)?[.]$ext%",
						'pattern'=>"%{$baseName}(_[^.]+)?[.][^.]+%",
						'require'=>$res);
						
			nxltrace("ADD $type: {$wd}{$srcDir}{$info['pattern']} to {$dpath})",'compil');
		}
		else
		{
			// only one file
			$resID = "$dpath/{$baseName}(_[^.]+)?[.]$ext";
			$info=array('from'=>"{$srcDir}{$baseName}.{$ext}",
						'wd'=>$wd,
						'to'=>"{$dpath}{$f}",
						'require'=>$res);
			
			nxltrace("ADD $type: {$wd}{$info['from']} to {$info['to']})",'compil');
		}
		
		// add resource for deployment
		$this->context->addResource($info,"resource_$type",$resID);

		return "../res/$fullName";
	}
	
	function _html($node,&$level)
	{
		$level=0;
		$this->loadConfig($node);
		//$this->keepAllText($node,true);
	}

	function body($node,$content,$level,$pad)
	{
		$this->context->addResource($content,'html.body');
		return $this->passThrough($node,$content,$pad,true);		
	}
	
	function head($node,$content,$level,$pad)
	{
		$this->head=$content;
		return $this->passThrough($node,$content,$pad,true);		
	}

	function link(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);	
		if ($this->getAttribute($src,$node,'href'))
		{
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node->setAttribute('href',$src);
		}
		// $src=preg_replace('%^([.][.][/])+%','',$src);
		// $src="../res/$src";
		return $this->passThrough($node,$content,$pad,true);		
	}
	
	function script_(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);
		if ($this->getAttribute($src,$node,'src'))
		{
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node->setAttribute('src',$src);
		}
		return parent::script_($node,$content,$level,$pad);		
	}

	function debug()
	{
		$this->context->trace("debug");
	}
	
	function img(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);

		// reload initial url if 2nd pass (through xpath walker)
		if (($src=$node->getAttribute('nx_res_url'))==null)
		{
			if (($src=$node->getAttribute('src'))!=null)
				$node->setAttribute('nx_res_url',$src);
			else
				$src=null;
		}
			
		$node2=$node;

		if ($src!=null)
		{
			$src=$this->parseVal($src);
			
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node2->setAttribute('src',$src);
		}
		return $this->passThrough($node2,$content,$pad,true);		
	}
	
	function embed(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);

		// reload initial url if 2nd pass (through xpath walker)
		if (($src=$node->getAttribute('nx_res_url'))==null)
		{
			if (($src=$node->getAttribute('src'))!=null)
				$node->setAttribute('nx_res_url',$src);
			else
				$src=null;
		}
			
		$node2=$node;

		if ($src!=null)
		{
			$src=$this->parseVal($src);
			
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node2->setAttribute('src',$src);
		}
		return $this->passThrough($node2,$content,$pad,true);		
	}

	function param(&$node,$content,$level,$pad)
	{
		nxltrace("html:param",'compil');

		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);
		
		if ($this->getAttribute($name,$node,'name') && (strtolower($name)=='movie' || strtolower($name)=='src'))
		{
			nxltrace("html:param: is resource..",'compil');
			
			// reload initial url if 2nd pass (through xpath walker)
			if (($src=$node->getAttribute('nx_res_url'))==null)
			{
				if (($src=$node->getAttribute('value'))!=null)
					$node->setAttribute('nx_res_url',$src);
				else
					$src=null;
			}
				
			nxltrace("html:param: src=$src",'compil');
			$node2=$node;
	
			if ($src!=null)
			{
				$src=$this->parseVal($src);
				
				// get resource path as url and deployment info
				$url=$this->__process_res($node,$src,$resID,$info);
				if ($url!=null)
					$src=$url;
					
				$node2->setAttribute('value',$src);
			}
			return $this->passThrough($node2,$content,$pad,true);
		}
		
		return $this->passThrough($node,$content,$pad,true);
	}
	
	function td(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);

		// reload initial url if 2nd pass (through xpath walker)
		if (($src=$node->getAttribute('nx_res_url'))==null)
		{
			if (($src=$node->getAttribute('background'))!=null)
				$node->setAttribute('nx_res_url',$src);
			else
				$src=null;
		}
			
		$node2=$node;

		if ($src!=null)
		{
			$src=$this->parseVal($src);
			
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node2->setAttribute('background',$src);
		}
		return $this->passThrough($node2,$content,$pad,true);		
	}
	
	function table(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);

		// reload initial url if 2nd pass (through xpath walker)
		if (($src=$node->getAttribute('nx_res_url'))==null)
		{
			if (($src=$node->getAttribute('background'))!=null)
				$node->setAttribute('nx_res_url',$src);
			else
				$src=null;
		}
			
		$node2=$node;

		if ($src!=null)
		{
			$src=$this->parseVal($src);
			
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node2->setAttribute('background',$src);
		}
		return $this->passThrough($node2,$content,$pad,true);		
	}
	
	function input(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);

		// reload initial url if 2nd pass (through xpath walker)
		if (($src=$node->getAttribute('nx_res_url'))==null)
		{
			if (($src=$node->getAttribute('src'))!=null)
				$node->setAttribute('nx_res_url',$src);
			else
				$src=null;
		}
			
		$node2=$node;

		if ($src!=null)
		{
			$src=$this->parseVal($src);
			
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info);
			if ($url!=null)
				$src=$url;
				
			$node2->setAttribute('src',$src);
		}
		return $this->passThrough($node2,$content,$pad,true);				
	}

	function comment_($node,$content,$level,$pad)
	{	
		if (@$content[0]==']')
			return '';

		if ($this->getAttribute($v,$node,'value'))
			$content=$v.$content;

		return "<!--{$content} -->";		
	}

	function _processElement(&$node)
	{		
		$node->removeAttribute('_pmethod');
		$node->removeAttribute('pmethod');
		$this->preprocessAttributes($node);	
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
					
		$node->removeAttribute('_pmethod');
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
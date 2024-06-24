<?PHP

// library for the creation of xhtml or xml views
class taglib_w3_1999_xhtml extends NxTagLib
{		
	var $def = array(
		'default'=>'',
		'html'=>'close=force',
		'head'=>'close=force',
		'title'=>'close=force;content=text',
		'meta'=>'newline=yes;close=yes',

		'body'=>'close=force',	
		'h1'=>'close=force;newline=no;indent=no;content=text',	
		'h2'=>'close=force;newline=no;indent=no;content=text',	
		'h3'=>'close=force;newline=no;indent=no;content=text',	
		'h4'=>'close=force;newline=no;indent=no;content=text',	
		'h5'=>'close=force;newline=no;indent=no;content=text',	
		'br'=>'newline=yes;close=yes;indent=no;content=no;',
		'hr'=>'newline=yes;close=yes;indent=no;content=no;',
		'script'=>'close=force;content=text',
		'iframe'=>'close=force',
		'a'=>'close=force;content=text',
		'span'=>'close=force;content=text',
		'div'=>'close=force;content=text',
		'b'=>'close=force;content=text;newline=no;indent=no;',
		'strong'=>'content=text;newline=no;indent=no;',
		'textarea'=>'close=force;content=text;indent=no',
		'input'=>'newline=yes;close=yes;indent=no;content=no',
		'p'=>'close=force;content=text',
		'table'=>'',
		'tr'=>'close=force',
		'td'=>'close=force;content=text',
		'th'=>'close=force;content=text',
		'img'=>'newline=no;close=yes;indent=no;',
	);

	function __construct(&$context) {
		parent::__construct($context);
		$this->context->indentHTML=true;
	}

	function _processElement(&$node)
	{		
		$this->preprocessAttributes($node);	
	}
	
	/* ======== PROCESSING OF RESOURCES ============ */
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
		else {
			// get doc type if any
			if ($this->getAttribute($doc_type,$node,'doc_type'))
				return '<!DOCTYPE '.$doc_type.">\n".$content;
			else
				return $content;
		}
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
	
	function __process_res(&$node,$name,&$resID,&$info,$mode='@')
	{
		if (isset($GLOBALS['NXL_PROCESS_RESOURCES']) && $GLOBALS['NXL_PROCESS_RESOURCES']===false)
			return $name ;
		
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
			$deployIt=true;
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
			$deployIt=true;
		}
		elseif (preg_match('%((?:[.][.]/)*)(((images|css|js)/)((?:.(?!_(?:IE|NN|NS|OP|GK)))+.)(?:_((?:IE|NN|NS|OP|GK)[0-9_.]+))?[.]([^./]+))$%i',
			$name,$matches))
		{	// relative URL, with dynamic property => do not deploy
		
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
			$deployIt=false;
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
		
		if ($deployIt)
		{
			// TPL directive : "require" resource
			$res="{{$mode}require:$f";
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
		}
		
		return "../res/$fullName";
	}

 	function tag_default($node,$content,$level,$pad,$ns,$op)
	{
		// returns it
		return $this->passThrough($node,$content,$pad,true);
	}

	function hr($node,$content,$level,$pad,$ns,$op)
	{
		// returns it
		return $this->passThrough($node,$content,$pad,true);
	}

	function comment_($node,$content,$level,$pad)
	{	
		if (@$content[0]!='[')
			return '';

		if ($this->getAttribute($v,$node,'value'))
			$content=$v.$content;

		return "<!--{$content} -->";		
	}	
	
	function _html($node)
	{
		$this->loadConfig($node);
	}
		
	function html($node,$content,$level,$pad)
	{
		$atts='';		
		$this->preprocessAttributes($node);
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($parseAtts)
				$av=$this->parseVal($av,false);

			if (!preg_match('/_?(pmethod|doc_uri|nx_res_url|xmlns)/',$an))
				$atts .=" $an=\"$av\"";
		}
        
        if (isset($node->nsdef) && count($node->nsdef)>0)
        {
            $ns='';
            foreach ($node->nsdef as $nsdef)
            {
                if (isset($nsdef['prefix']) && strlen($nsdef['prefix'])>0)
                    $ns .= "xmlns:{$nsdef['prefix']}=\"{$nsdef['uri']}\" ";
                else
                    $ns .= "xmlns=\"{$nsdef['uri']}\" ";
            }
        }
        else
            $ns = 'xmlns="http://www.w3.org/1999/xhtml"';
			
		return "<html $ns $atts>\n$content</html>";
	}
			
	function head($node,$content,$level,$pad)
	{
		if (trim($content)=='')
		{
			$content='<title></title>';
		}
		return $this->passThrough($node,$content,$pad,true);
	}
	
	/* config generateur js */
	static $generated_scripts=array();
	static $generated_scripts_active=true;

	function config_nx(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($gen,$node,'js'))
		{
			if($gen == 'yes' || $gen == "true" || $gen == "active" )
				self::$generated_scripts_active = true;
			else
				self::$generated_scripts_active = false;
		}	
	}


	function _script_() {
		$this->context->indentHTML=false;
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
		
		if ($this->getAttribute($add_to,$node,'append_to'))
		{
//DebugBreak();
			if (!$this->getAttribute($id,$node,'id'))
				$id = 'main.js';

			$content2 = <<<EOH



/*
  ________________________________________________________
	
	COMPONENT : $id
	
  -------------------------------------------------------- */
 
 $content
/*  /COMPONENT $id   */

EOH;

			if(!isset(self::$generated_scripts[$add_to]))
			{
				self::$generated_scripts[$add_to] = $content;
				$this->saveScript($add_to,$content2,false);
			}
			else
			{
				self::$generated_scripts[$add_to] .= "\n<!--\n  SCRIPT $id added to $add_to \n -->\n".
					$content;

				$this->saveScript($add_to,$content2,true);
			}

			if(self::$generated_scripts_active)
			{
				return "\n<!--\n  SCRIPT $id added to $add_to \n-->\n";
			}
			else
			{
				// regular behaviour
				$this->context->indentHTML=true;
				return $this->passThrough($node,$content,$pad,true);
			}
		}
		else
		{
			if (trim($content) && !preg_match('/<!\[CDATA\[/',$content))
			{
				// $content="\n//<![CDATA[\n$content\n//]]>\n";
			}

			// regular behaviour
			$this->context->indentHTML=true;
			return $this->passThrough($node,$content,$pad,true);
		}

	}
	
	function saveScript($id,$content,$append=true)
	{
		$package=$this->context->getLastResource('package',true);
		if(!$package)
			$package = $this->context->props->getProperty('nx.package');

		if($package)
			$fpath=GENERIS_PACKAGES.$package.'/view/default/res/'.trim($id,'/');
		else
			$fpath=trim($id,'/');

		// save class
		$this->context->saveSrc($fpath,$content,$append);
	}


	function body($node,$content,$level,$pad)
	{
		$this->context->addResource($content,'html.body');
		return $this->passThrough($node,$content,$pad,true);		
	}

	function link(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
		
		// deploy mode: dynamic or static. If static, css link added in envelope page at compile time,
		// if dynamic, it is dynamically added to list of resources (better if conditional loading of content).
		$this->getAttribute($mode,$node,'static');
		if($mode=='static')
			$mode='@'; 
		else
			$mode='';
			
		$this->_processElement($node);	
		if ($this->getAttribute($src,$node,'href'))
		{
			// get resource path as url and deployment info
			$url=$this->__process_res($node,$src,$resID,$info,$mode);
			if ($url!=null)
				$src=$url;
				
			$node->setAttribute('href',$src);
		}
		// $src=preg_replace('%^([.][.][/])+%','',$src);
		// $src="../res/$src";
		return $this->passThrough($node,$content,$pad,true);		
	}
	
	function a(&$node,$content,$level,$pad)
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

	function iframe(&$node,$content,$level,$pad)
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

	function object_(&$node,$content,$level,$pad)
	{
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return '';
			
		$this->_processElement($node);

		// reload initial url if 2nd pass (through xpath walker)
		if (($src=$node->getAttribute('nx_res_url'))==null)
		{
			if (($src=$node->getAttribute('data'))!=null)
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
				
			$node2->setAttribute('data',$src);
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
	
	// ======== extensions ===========
	function _trace()
	{
	}
	
	function trace($node,$content,$level,$pad)
	{
		$src=nl2br(htmlspecialchars($this->parseVal($content)));
		$this->context->trace($src);
		return '';
	}
	
	function space($node,$content,$level,$pad)
	{
		$this->getAttribute($n,$node,'nb','1',true,false,false);
		return str_repeat('&nbsp;',(int)$n);
	}
	
	// ====== utilities =====
	// overload parseVal and wraps attributes with <?php when expressions exist
	function parseVal($v,$quotes=false, $wrapPhp = false, $dft = '', $prop = '$this')
	{
		if (preg_match('/#[^#]+#/',$v))
		{
			$v = parent::parseVal($v,true,false);
			if (preg_match('/\$[_a-z]+(->|\[)/i',$v))
				return '<?php echo '.$v.';?>';
			else
				return trim($v,"'");
		}
		
		return $v;
	}

	// preprocess attributes (if prefixed by _, overload normal attribute)
	function preprocessAttributes(&$node)
	{
		$attributes=$node->_getAttributes($node);
		foreach ($attributes as $an=>$av)
		{
			if ($an[0]=='_')
			{
				$an_=substr($an,1);
				$node->setAttribute($an_,$av);
				$node->removeAttribute($an);
			}			
		}
	}
	
	// pass the content through and apply a formatter described in class variable
	function passThrough($node,$content,$pad='',$parseAtts=false)
	{		
		if ($this->getAttribute($role,$node,'_role') && $role=='design')
			return $content;
			
		$name=$node->localName;
		$dom=&$this->getDOM();
		$atts='';
		if ($name=='#text')
			return $node->data;
		
		$name=strtolower($name);	
		
		$this->preprocessAttributes($node);
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($parseAtts)
				$av=$this->parseVal($av,false);
	
			// escape quotes ??
			// $av=preg_replace('/"/','\\"',$av);

			if (!preg_match('/_?(pmethod|doc_uri|nx_res_url)/',$an))
				$atts .=" $an=\"$av\"";
		}
		
		// get options
		if (isset($this->def[$name]))
			$opts=array_flip(explode(';',strtolower($this->def[$name])));
		else
			$opts=array_flip(explode(';',strtolower($this->def['default'])));
				
		if(!$this->context->indentHTML)
		{
			$pad1=$pad2=$sep1=$sep2='';
		}
		else 
		{
			if (isset($opts['indent=no']))
			{
				$pad1='';
				$pad2='';
			}
			else
			{
				$pad1=$pad;
				if (isset($opts['content=text']))
					$pad2='';
				else
					$pad2=$pad;
			}
				
			if (isset($opts['newline=no']))
				$sep1='';
			else
				$sep1="\n";
	
			if (!isset($opts['indent=no']) && !isset($opts['newline=no']) && !isset($opts['content=text']))
				$sep2="\n";
			else
				$sep2='';
		}

		if (isset($opts['close=force']))
		{
			$close1="></$name>";
			$close2="</$name>";
		}
		elseif (isset($opts['close=no']))
		{
			$close1='>';
			$close2="</$name>";
		}
		else
		{
			$close1='/>';
			$close2="</$name>";
		}
					
		if ($content!='')
			if (isset($opts['content=no']))
				return "{$sep1}{$pad1}<{$name}{$atts}{$close1}$content";
			else
				return "{$sep1}{$pad1}<{$name}{$atts}>$content{$sep2}{$pad2}{$close2}";
		else
			return "{$sep1}{$pad1}<{$name}{$atts}{$close1}";	
	}

	function _func_skin()
	{
		$skin=$this->context->getLastResource('skin',false);
		if ($skin==null)
			return 'default';

		return $skin;
	}
}

?>
<?PHP

// all data format operations
class taglib_nxfrontier_nxl_i8n extends NxTagLib
{
	function _strings(&$node) {
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		

		global $NX_LANG;
		global $_NX_site;
		$this->getAttribute($lang,$node,'lang',$NX_LANG,true,false);
		$this->getAttribute($name,$node,'name','default',true,false);
		$this->getAttribute($type,$node,'type','strings',true,false);
		$name=strtr($name,'.','/');
		
		$this->context->addResource("i8n_{$name}_{$lang}@$package",'i8n_name');
		
		nxltrace("Create I8N strings : {$name}_{$lang}@$package","trace");
		
		// inherit strings
		$strings=array();
		$this->getAttribute($inherit,$node,'inherit',false,true,false);
		if ($inherit!=false)
		{
			$list=explode(',',$inherit);
			foreach ($list as $parent)
			{
				$parent=strtr($parent,'.','/');
				
				nxltrace("Inherit I8N strings : {$parent}_{$lang}","trace");
				// check current package, or root package or specified name with package
				$par_strings=$this->context->getResources("i8n_{$parent}_{$lang}@$package");
				if(count($par_strings)==0)
					$par_strings=$this->context->getResources("i8n_{$parent}_{$lang}");
				
				$strings+=$par_strings;	
			}
		}
		$this->context->addResources($strings,"i8n_{$name}_{$lang}@$package");
	}
	
	function strings($node,&$content,$level,$pad)
	{		
		global $NX_LANG;
		global $_NX_site;
		
		$i8n_name=$this->context->getLastResource('i8n_name',true);
		$this->getAttribute($lang,$node,'lang',$NX_LANG,true,false);
		$this->getAttribute($name,$node,'name','default',true,false);
		$this->getAttribute($type,$node,'type','strings',true,false);
		$name=strtr($name,'.','/');

		// get root directory for application (and pop package)
		$package=$this->context->getLastResource('package',true);
		if ($package=='root')
			$rootPath=NX_CONF.'content/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/content/';
		else
			$rootPath=NX_CONF."packages/{$package}/content/";

		// cleanup cache
		$domain=$_NX_site->getProperty('site.domain',$GLOBALS['NX_CONTENT_COMP']);		
		$this->_invalidate_cache("{$domain}$type/{$name}_{$lang}.inc");
		$this->_invalidate_cache("{$domain}$type/{$name}_{$lang}@$package.inc");
		
		// get strings
		$strings=$this->context->getResources($i8n_name);
		// sort($strings);
		$count=count($strings);
		if ($count==0)
			nxltrace("no string found... empty I8N pack","trace");	
		
		// create content source
		$content = '$list=array(
';
		foreach ($strings as $k=>$v)
		{
			if(preg_match('/\\\n/',$v))
			{
				if(!preg_match("/'/",$v))
					$content .= "'$k' => \"$v\",\n";
				else
				{
					$v = str_replace("\\'","'",$v);
					$content .= "'$k' => <<<EOH
$v
EOH
,\n";
				}
			}
			else
				$content .= "'$k' => '$v',\n";
		}
		$content.='); ';
		
		nxltrace("Added I8N $count strings : {$name}_{$lang}@$package=$content","listing");

		$content = '<?php '.$content.'?>';
		
		// save strings
		$this->context->saveSrc($rootPath."$type/{$name}.inc",$content);
		$this->context->saveSrc($rootPath."$type/{$name}_{$lang}.inc",$content);
	}

	function string($node,&$content,$level,$pad)
	{		
		// get id and value
		$this->getAttribute($id,$node,'id','',true,false);
		$this->getAttribute($value,$node,'value',$content,true,false);

		$id=str_replace(array('\\\'','\''),array("'",'\\\''),$id);
		$value=str_replace(array('\\\'','\''),array("'",'\\\''),$value);

		// get root directory for application (and pop package)
		$strings_name=$this->context->getLastResource('i8n_name',false);
		$this->context->addResource($value,$strings_name,$id);

		//nxltrace("{$strings_name}::string[$id]=$value","trace");
	}

	// for field : use name, label and prompt and generate strings...
	function field($node,&$content,$level,$pad)
	{		
		// get name and label
		$this->getAttribute($name,$node,'name','',true,false);
		$id="field:$name";
		$this->getAttribute($value,$node,'label',$content,true,false);

		$id=str_replace(array('\\\'','\''),array("'",'\\\''),$id);
		$value=str_replace(array('\\\'','\''),array("'",'\\\''),$value);

		// get root directory for application (and pop package)
		$strings_name=$this->context->getLastResource('i8n_name',false);
		$this->context->addResource($value,$strings_name,$id);

		$this->getAttribute($prompt,$node,'prompt','',true,false);
		if($prompt)
		{
			$idPrompt="field.prompt:$name";
			$id=str_replace(array('\\\'','\''),array("'",'\\\''),$idPrompt);
			$value=str_replace(array('\\\'','\''),array("'",'\\\''),$prompt);
			$this->context->addResource($value,$strings_name,$id);
		}

		$this->getAttribute($help,$node,'help','',true,false);
		if($help)
		{
			$idHelp="field.help:$name";
			$id=str_replace(array('\\\'','\''),array("'",'\\\''),$idHelp);
			$value=str_replace(array('\\\'','\''),array("'",'\\\''),$help);
			$this->context->addResource($value,$strings_name,$id);
		}
	
	}

	// for field : use name, label and prompt and generate strings...
	function fields($node,&$content,$level,$pad)
	{		
		return $content;
	}
	
	function _invalidate_cache($path) {
				
		$path=preg_replace('#[\\\\/]+#','/',$path);
		if (is_file($path))
		{
			$fs=$this->props->getPlugin('FileSys');
			$fs->delete($path);
		}		

	}

}

?>
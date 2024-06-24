<?PHP

// all data format operations
class taglib_nxfrontier_nxl_lists extends NxTagLib
{
	function _list_(&$node) {
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		

		global $NX_LANG;
		global $_NX_site;
		$this->getAttribute($lang,$node,'lang',$NX_LANG,true,false);
		$this->getAttribute($name,$node,'name','default',true,false);
		$this->getAttribute($type,$node,'type','lists',true,false);
		$name=strtr($name,'.','/');
		
		$this->context->addResource("list_{$name}_{$lang}@$package",'list_name');
		
		nxltrace("Create list strings : {$name}_{$lang}@$package","trace");
		
		// inherit strings
		$strings=array();
		$this->getAttribute($inherit,$node,'inherit',false,true,false);
		if ($inherit!=false)
		{
			$list=explode(',',$inherit);
			foreach ($list as $parent)
			{
				$parent=strtr($parent,'.','/');

				nxltrace("Inherit list strings : {$parent}_{$lang}","trace");
				// check current package, or root package or specified name with package
				$par_strings=$this->context->getResources("list_{$parent}_{$lang}@$package");
				if(count($par_strings)==0)
					$par_strings=$this->context->getResources("list_{$parent}_{$lang}");
				
				$strings+=$par_strings;	
			}
		}
		$this->context->addResources($strings,"list_{$name}_{$lang}@$package");
/*
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		

		global $NX_LANG;
		global $_NX_site;
		$this->getAttribute($lang,$node,'lang',$NX_LANG,true,false);
		$this->getAttribute($name,$node,'name','default',true,false);
		$this->getAttribute($type,$node,'type','lists',true,false);
		
		$this->context->addResource("lists_{$name}_{$lang}@$package",'lists_name');
		
		// inherit strings
		$strings=array();
		$this->getAttribute($inherit,$node,'inherit',false,true,false);
		if ($inherit!=false)
		{
			$list=explode(',',$inherit);
			foreach ($list as $parent)
			{
				// check current package, or root package or specified name with package
				$par_strings=$this->context->getResources("lists_{$parent}_{$lang}@$package");
				if(count($par_strings)==0)
					$par_strings=$this->context->getResources("lists_{$parent}_{$lang}");
				
				$strings+=$par_strings;	
			}
		}
		$this->context->addResources($strings,"lists_{$name}_{$lang}@$package");
*/
	}
	
	function list_($node,&$content,$level,$pad)
	{		
		global $NX_LANG;
		global $_NX_site;
		
		$list_name=$this->context->getLastResource('list_name',true);
		$this->getAttribute($lang,$node,'lang',$NX_LANG,true,false);
		$this->getAttribute($name,$node,'name','default',true,false);
		$this->getAttribute($type,$node,'type','lists',true,false);
		
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
		
		// get lists
		$strings=$this->context->getResources($list_name);
		// sort($strings);
		$count=count($strings);
		if ($count==0)
			nxltrace("no string found... empty list pack","trace");	
		
		// create content source
		$content = '$list=array(
';
		foreach ($strings as $k=>$v)
		{
			$content .= "'$k' => '$v',\n";
		}
		$content.='); ';
		
		nxltrace("Added list $count options : {$name}_{$lang}@$package=$content to ".$rootPath."$type/{$name}_{$lang}.inc","listing");

		$content = '<?php '.$content.'?>';
		
		// save strings
		$this->context->saveSrc($rootPath."$type/{$name}.inc",$content);
		$this->context->saveSrc($rootPath."$type/{$name}_{$lang}.inc",$content);
		
		
		/*
		global $NX_LANG;
		global $_NX_site;
		
		$lists_name=$this->context->getLastResource('lists_name',true);
		$this->getAttribute($lang,$node,'lang',$NX_LANG,true,false);
		$this->getAttribute($name,$node,'name','default',true,false);
		$this->getAttribute($type,$node,'type','lists',true,false);

		// get root directory for application (and pop package)
		$package=$this->context->getLastResource('package',true);
		if ($package=='root')
			$rootPath=NX_CONF.'content/';
		else
			$rootPath=NX_CONF."packages/{$package}/content/";

		// cleanup cache
		$domain=$_NX_site->getProperty('site.domain',$GLOBALS['NX_CONTENT_COMP']);		
		$this->_invalidate_cache("{$domain}$type/{$name}_{$lang}.inc");
		$this->_invalidate_cache("{$domain}$type/{$name}_{$lang}@$package.inc");
		
		// get strings
		$strings=sort($this->context->getResources($lists_name,false));
		
		// create content source
		$content = '<?php $list=array(';
		foreach ($strings as $k=>$v)
		{
			$content .= "'$k' => '$v',\n";
		}
		$content.='); ?>';
		
		nxltrace("Added lists strings : {$name}_{$lang}@$package to ".$rootPath."$type/{$name}_{$lang}.inc","trace");
		
		// save strings
		$this->context->saveSrc($rootPath."$type/{$name}.inc",$content);
		$this->context->saveSrc($rootPath."$type/{$name}_{$lang}.inc",$content);
		*/
	}

	function option($node,&$content,$level,$pad)
	{		
		// get id and value
		$this->getAttribute($id,$node,'value','',true,false);
		$this->getAttribute($value,$node,'label',$content,true,false);
		$value=str_replace(array('\\\'','\''),array("'",'\\\''),$value);

		// get root directory for application (and pop package)
		$strings_name=$this->context->getLastResource('list_name',false);
		$this->context->addResource($value,$strings_name,$id);
/*
		// get id and value
		$this->getAttribute($id,$node,'value','',true,false);
		$this->getAttribute($value,$node,'label',$content,true,false);
		$value=preg_replace(array('/\\?\'/'),array('\\\''),$value);

		// get root directory for application (and pop package)
		$strings_name=$this->context->getLastResource('lists_name',false);
		$this->context->addResource($value,$strings_name,$id);
*/
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
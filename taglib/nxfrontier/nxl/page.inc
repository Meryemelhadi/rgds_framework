<?PHP

include_once(NX_LIB.'framework.inc');

// page generation
class taglib_nxfrontier_nxl_page extends NxTagLib
{		
	function _pages(&$node,&$level)
	{
		$node->setDeftAtt('path','/');

		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');
	}
	
	function pages()
	{
		$this->context->getLastResource('package',true);
	}

	function _page(&$node,&$level)
	{
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
		
		$name=$node->setDeftAtt('name','index');
		$node->setDeftAtt('protected','no');
		if (!$this->getAttribute($path,$node,'path'))
		{
			$this->getAttribute($path,$this->context->getParentElement($node),'path','/');
			$node->setDeftAtt('path',$path);
		}
		if ($path=='')
		{
			nxltrace("path not set for page:$name, set to '/'");
			$path='/';
		}
		$this->addResource($path,'path');
				
		$node->setDeftAtt('code','display');
		$node->setDeftAtt('strings','');
		$node->setDeftAtt('lang','en');
		$node->setDeftAtt('cache','no');
		$node->setDeftAtt('skin','default');
		$this->getAttribute($skin,$node,'skin','default',true,true);		
		$this->context->addResource($skin,'skin');

		$node->setDeftAtt('title',$name);
		
		// is cache ?
		$this->getAttribute($cache,$node,'cache','no');
		$this->addResource($cache,'page_cache');
		
		// reset the automatic node level used for indentation
		$level=0;
	}
		
	function page(&$node,&$content,$level,$pad)
	{	
		if(!trim($content))
		{
			$this->getAttribute($logic,$node,'logic',null);
			$this->getAttribute($application,$node,'application',null);
			if($logic)
				$content = $this->exec($node,$content,$level,$pad,$node);
		}

		$package=$this->context->getLastResource('package',true);
		
		// get path
		$path=$this->getLastResource('path',true);
		
		// get name
		$this->getAttribute($name,$node,'name','index');		
		
		// file path
		$fpath = NX_DOC_ROOT.trim($path,'/')."/{$name}.php";
		
		$src= '<?php
/* file automatically generated from NXL source ['.$this->context->getFileURI().']. Do not edit it. */
		
';
// $NX_DOMAIN=strtolower($_SERVER[\'SERVER_NAME\']);
//';
//		$conf_host = $this->getSitePath($path).'conf/host/{$NX_DOMAIN}.inc';
//		$src.="require_once(\"$conf_host\");\n";
			
		$this->getAttribute($perms,$node,'permissions','default');

		// 'protected' => 'no',
		$this->getAttribute($isprot,$node,'protected','no');
		switch(strtolower($isprot))
		{
			case '':
			case 'false':
			case 'public':
			case 'no':
				break;
			case 'true':
			case 'yes':
				$src .= "\$loginPerm='$perms';
\$loginService='user';
";
				break;
			default:
				$src .= "\$loginPerm='$perms';
\$loginService='{$isprot}';
";
		}
		
		if ($this->getBool($isprot))
			$src .= '$loginPerm="default";
';
	    	
		// pop skin
		$this->context->getLastResource('skin',true);

		// 'cache' => 'no',
		// reset cache param
		/*
		$cache=$this->getLastResource('page_cache',true);

		if ($this->getBool($cache))
			$v='true';
		else
			$v='false';
			
		$src .= "define('NX_HTTP_Cache',$v);
";
*/

		// 'skin' => 'default',
		if ($skin=$this->parseVal($node->getAttribute('skin','default'),true))
			;
/*
			$src .= "define('NX_SKIN_SERVER',$skin);
";
*/

		// get mime and encoding
		$this->getAttribute($type,$node,'type','html');
		switch(strtolower($type))
		{
			case 'js':
			case 'javascript':
				$mime='application/x-javascript';
				break;
			case 'css':
				$mime='text/css';
				break;
			case 'xml':
				$mime='text/xml';
				break;
			case 'html':
			default:
				$mime='text/html';
				break;
		}
		
		// get mime and encoding
		$this->getAttribute($mime,$node,'mime',$mime);
		
		$src.="global \$NX_MIME;\n\$NX_MIME='$mime';\n";
		
		// get site conf path
		//$path=$this->getLastResource('path',false);
		$sitePath = $this->getSitePath($path).'NxSite.inc';
		$src.="require_once('$sitePath');";
				
		$src.= "\n$content?>";
		$src = $this->cleanupSrc($src);
		$this->context->saveSrc($fpath,$src);

		nxltrace("PAGE: $fpath = [<pre>$src</pre>]",'listing');	
		if (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES) {
			$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/{$name}.php";
			$this->context->saveSrc($fpath,$src);
			nxltrace("PAGE: $fpath = [<pre>(copy)</pre>]",'listing');	
		}

		return '';
	}
	
	function output($node,$content,$level,$pad)
	{
		switch ($node->getAttribute('method','html'))
		{
			case 'xml2text':
				return '?>'.nl2br(htmlspecialchars($content)).'<?php';
				break;
			default:
				return '?>'.$content.'<?php';
				break;
		}		
	}	

	function _application(&$node,&$level)
	{
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
		
		$name=$node->setDeftAtt('name','index');
		$node->setDeftAtt('protected','no');
		if (!$this->getAttribute($path,$node,'path'))
		{
			$this->getAttribute($path,$this->context->getParentElement($node),'path','/');
			$node->setDeftAtt('path',$path);
		}
		if ($path=='')
		{
			nxltrace("path not set for page:$name, set to '/'");
			$path='/';
		}
		$this->addResource($path,'path');
				
		$node->setDeftAtt('code','display');
		$node->setDeftAtt('strings','');
		$node->setDeftAtt('lang','en');
		$node->setDeftAtt('cache','no');
		$node->setDeftAtt('skin','default');
		$this->getAttribute($skin,$node,'skin','default',true,true);		
		$this->context->addResource($skin,'skin');

		$node->setDeftAtt('title',$name);
		
		// is cache ?
		$this->getAttribute($cache,$node,'cache','no');
		$this->addResource($cache,'page_cache');
		
		// reset the automatic node level used for indentation
		$level=0;
	}

	// new application format based on PHP descriptor
	function application($node,$content,&$level,$pad,$parent=null)
	{
		// get model
        $this->getAttribute($processus,$node,'workflow','');
        $this->getAttribute($appType,$node,'application_type',$processus ? 'processus' : 'crud');
		$appTypeCls = "application_{$appType}";
		include_once("applications_types/{$appTypeCls}.inc");
		$model = new $appTypeCls($this,$appType);

		$model->generate($node,$parent);

/*
		// decode package (and remove package suffix on "logic" param)
		$package=$this->decodePackage($node,'inherit','logic',true);	
		
		// get cache mode
		$iscache=$this->getBool($this->getLastResource('page_cache',true));
		
		if(!$parent)
			$parent=$this->context->getParentElement($node);

		// get params
		$params=$this->context->getResources('params',"\n");
		$src.= "\n$params";
		
		// get path
		$path=$this->getLastResource('path',true);
		
		// get name
		$this->getAttribute($name,$node,'name','index');				

		$this->getAttribute($page,$parent,'page',$name);
		$this->getAttribute($application,$node,'application',$name);
		$this->getAttribute($table,$node,'table',$application);
		$this->getAttribute($base_data,$node,'base_data',$table);
        $this->getAttribute($strings,$node,'strings',$application);
        $this->getAttribute($skin,$node,'skin','default');
        $this->getAttribute($string_base,$node,'string_base','record');
        
        $this->getAttribute($default_op,$node,'default_operation','list');
        $this->getAttribute($role,$node,'role','default');
        $this->getAttribute($isCreator,$node,'isCreator',false);
		$isCreator=($isCreator!='false')&&($isCreator!='no');

        $this->getAttribute($isValidator,$node,'isValidator',false);
		$isValidator=($isValidator!='false')&&($isValidator!='no');

		// prepare string mapping for model->clone copy
		$this->getAttribute($string_mapping,$node,'string_mapping','');

		$map = array('/evalmag/'=>$package,'/modelcomp/'=>$application);
		$stringsMap = explode('|',$string_mapping);
		foreach($stringsMap as $stringMap)
		{
			$a = explode('=',$stringMap);
			if(count($a)>1)
			{
				$map['|'.$a[0].'|']=$a[1];
			}
		}

		if(!$strings)
			$strings = $application;
		
		// file path
		$fpath = NX_DOC_ROOT.trim($path,'/')."/{$name}.php";

		$src=$model->page($package,$path,$role,$application,$skin,$fpath,$this,$node);
		
		nxltrace("PAGE: $fpath = [<pre>$src</pre>]",'listing');	
		if (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES) {
			if($appType=='angular')
				$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/api/{$name}.php";
			else
				$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/{$name}.php";
			$this->context->saveSrc($fpath,$src);
			nxltrace("PAGE: $fpath = [<pre>(copy)</pre>]",'listing');	
		}

		//============ FICHIERS ANNEXES ===============
		$packagePath = GENERIS_PACKAGES.$package.'/';	
		$fs = Framework::fs();
		
		// ========= DESCRIPTEUR APPLICATION 
		if(!is_dir($d=$packagePath.'application/'))
		{
			$fs->createTree($d,0777);
		}

		if(!is_file($f=$d."{$application}.inc"))
		{
			$s = $model->appDesc($package,$application,$processus,$role,$table,$base_data,$strings,$perms,$isCreator,$isValidator);
			$this->context->saveSrc($f,$s.'?>');
			nxltrace("Application: $f = [<pre>$s</pre>]",'listing');				
		}

		// ========= VIEWS ===== 

		if(!is_dir($d=$packagePath.'view/'))
			$fs->createTree($d,0777);		

		if(!is_dir($d=$packagePath.'view/'.$skin.'/'))
			$fs->createTree($d,0777);		

		// ========= NXL ===== 
		if(!is_dir($d=$packagePath.'nxl/'))
		{
			$fs->createTree($d,0777);
		}
		
		$xmlstart = '<?xml version="1.0"?' .'>';

		// ========= nxl/all.nxl ===== 
		if(!is_file($f=$packagePath.'nxl/all.nxl'))
		{
			$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<!-- include data model -->
<include_once file="/data_model/all.nxl" />

<!-- include strings -->
<include_once file="/strings/all.nxl" />

<!-- include applications -->
<include_once file="/{$page}.nxl" />

</nxl>
EOH;
			$fs->saveFile($f,$s);
			nxltrace("Application XML root: $f = [<pre>$s</pre>]",'listing');			
		}
		else
		{
			$allS = $fs->file_get_contents($f);
			if(!trim($allS))
			{
				$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<!-- include data model -->
<include_once file="/data_model/all.nxl" />

<!-- include strings -->
<include_once file="/strings/all.nxl" />

<!-- include applications -->
<include_once file="/{$page}.nxl" />

</nxl>
EOH;
				$fs->saveFile($f,$s);
				nxltrace("Application XML root: $f = [<pre>$s</pre>]",'listing');			
			}
			elseif(!preg_match("%/{$page}\.nxl%",$allS))
			{
				$allS = preg_replace('%</nxl>%',<<<EOH
<include_once file="/{$page}.nxl" />
</nxl>
EOH
					, $allS);

				$fs->saveFile($f,$allS);
				nxltrace("Application XML root: $f = [<pre>$s</pre>]",'listing');			
			}
		}


		// nxl/data_model/
		if(!is_dir($d=$packagePath.'nxl/data_model/'))
		{
			$fs->createTree($d,0777);
		}

		// nxl/data_model/all.nxl
		if(!is_file($f=$d.'all.nxl'))
		{
			$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<!-- include data model -->
<include_once file="/data_model/{$application}.data.nxl" />

</nxl>
EOH;
			$fs->saveFile($f,$s);
			nxltrace("Application XML data model root: $f = [<pre>$s</pre>]",'listing');			
		
		}
		else
		{
			$allS = $fs->file_get_contents($f);
			if(!trim($allS))
			{
				$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<!-- include data model -->
<include_once file="/data_model/{$application}.data.nxl" />

</nxl>
EOH;
				$fs->saveFile($f,$s);
				nxltrace("Application XML data model root: $f = [<pre>$s</pre>]",'listing');			
			}
			elseif(!preg_match("%/data_model/{$application}\.data\.nxl%",$allS))
			{
				$allS = preg_replace('%</nxl>%',<<<EOH
<include_once file="/data_model/{$application}.data.nxl" />
</nxl>
EOH
, $allS);
				$fs->saveFile($f,$allS);
				nxltrace("Application XML root: $f = [<pre>$s</pre>]",'listing');			
			}
		}

		// nxl/data_model/application.data.nxl
		if(!is_file($f=$d.$application.'.data.nxl'))
		{
			$s = $model->data_model($package,$base_data,$table,$isCreator,$isValidator);
			$fs->saveFile($f,$s);
			nxltrace("Application XML data model: $f = [<pre>$s</pre>]",'listing');			
		}

		// add data handlers if any
		$model->data_handlers($fs,"{$packagePath}lib/{$package}/records/",$package,$base_data,$table,$application);

		// add task handlers if any
		$model->tasks_handlers($fs,"{$packagePath}lib/{$package}/",$package,$application);

		// nxl/strings/
		if(!is_dir($d=$packagePath.'nxl/strings/'))
		{
			$fs->createTree($d,0777);
		}

		// nxl/strings/all.nxl
		if(!is_file($f=$d.'all.nxl'))
		{
			$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<!-- include strings -->
<include_once file="/strings/{$application}.strings.nxl" />

</nxl>
EOH;
			$fs->saveFile($f,$s);
			nxltrace("Application XML strings root: $f = [<pre>$s</pre>]",'listing');			
		}
		else
		{
			$allS = $fs->file_get_contents($f);
			if(!trim($allS))
			{
				$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<!-- include strings -->
<include_once file="/strings/{$application}.strings.nxl" />

</nxl>
EOH;
				$fs->saveFile($f,$s);
				nxltrace("Application XML strings root: $f = [<pre>$s</pre>]",'listing');			
			}
			elseif(!preg_match("%/strings/{$application}\.strings\.nxl%",$allS))
			{
				$allS = preg_replace('%</nxl>%',<<<EOH
<include_once file="/strings/{$application}.strings.nxl" />
</nxl>
EOH
, $allS);

				$fs->saveFile($f,$allS);
				nxltrace("Application XML root: $f = [<pre>$s</pre>]",'listing');			
			}
		}

		// nxl/strings/application.strings.nxl
		if(!is_file($f=$d."{$application}.strings.nxl"))
		{
			$s = $model->strings($package,$strings,$string_base);			
			$fs->saveFile($f,$s);
			nxltrace("Application XML strings : $f = [<pre>$s</pre>]",'listing');			
		}

		// deply resources
		$this->deployModelFiles('view',$packagePath,$map,$appType);
		$this->deployModelFiles('conf',$packagePath,$map,$appType);

		$this->deployModelFiles('www',$packagePath,$map,$appType);

		// deploy to www
		$this->deployModelFiles('www',NX_HOST_ROOT,$map,$appType);
		*/

		return '';
	}

	function deployModelFiles($dir,$packagePath,$map,$appType) {
		$fs = Framework::fs();

		$here = dirname(__FILE__).'/';
		$fromDirPath = $here.'applications_types/'.$appType.'/'.$dir.'/';
		$fromDirPath = $fs->formatPath($fromDirPath);
		if(!is_dir($fromDirPath))
		{
			return;
		}

		$toPathDir = $packagePath.$dir.'/';
		if(!is_dir($toPathDir))
		{
			$fs->createTree($toPathDir,0777);
		}

		$files = $fs->listFiles($fromDirPath,10);
		foreach($files as $fpath)
		{
			// remove parent dir
			$baseN = str_replace($fromDirPath,'',$fpath);

			// map file name
			$tobaseN = $this->mapMacros($baseN,$map);
			$tofpath=$toPathDir.$tobaseN;
			if(is_file($tofpath))
				continue;

			// copy file and map macros
			$src = $fs->file_get_contents($fpath);
			$src = $this->mapMacros($src,$map);
			$fs->saveFile($tofpath,$src);
		}
	}

	function mapMacros($src,$map) {
		$res = preg_replace(array_keys($map),array_values($map),$src);
		return $res;
	}

	
	// old plain XML application
	function exec($node,$content,&$level,$pad,$parent=null)
	{
		// decode package (and remove package suffix on "logic" param)
		$package=$this->decodePackage($node,'inherit','logic',true);	
		
		// get cache mode
		$iscache=$this->getBool($this->getLastResource('page_cache',true));
		if(!$parent)
			$parent=$this->context->getParentElement($node);
		
		// get params
		$params=$this->context->getResources('params',"\n");
		$src.= "\n$params";
		
		if ($this->getAttribute($strings,$node,'strings') || 
			$this->getAttribute($strings,$parent,'strings'))
		{
			if($package!='root' && $package!='')
				$strings="{$strings}@{$package}";
				
			$src.="{$pad}\$siteDesc->setProperty('page.strings','$strings');\n";
		}
		
		if ($this->getAttribute($logic,$node,'logic',null,true,true))
		{
			$logic=preg_replace(
				array('*/*',"/^('?)[.]/"),
				array('.','$1'),
					$logic);

			if($package!='root' && $package!='')
				$logic=$logic.".'@{$package}'";
			
			$src.="\n$pad\$page = \$siteDesc->runLogic($logic, array(
{$pad}	'page.logic' => $logic,\n";

			/* get aspect*/
			if ($this->getAttribute($aspect,$node,'aspect',null,true,true))
			{
				$src.="{$pad}	'page.aspect' => $aspect,\n";
			}

			$mode='logic';		
		}
		else
			$mode='view';

	// 'skin' => 'default',

	$skin=$this->context->getLastResource('skin',false);
	// if ($this->getAttribute($skin,$parent,'skin',"'default'",true,true))
		$src .= "{$pad}	'page.skin' => $skin,";
				
	if ($strings)
	{
		$src .= "\n{$pad}	'page.strings' => '$strings',";
	}
		
	$src.="\n$content{$pad}));\n";
	/*
	if ($mode=='logic')
	{
		$src.="{$pad}if (\$page)\n{$pad}	\$page->run();\n";
	}*/
	
	return $src;
	}

	// <data name="part.new" value="#param:ptype#.edit" media="db"/>
	function data($node,&$content,$level,$pad)
	{		
		if (($n=$node->getAttribute('name'))===null)
			return $this->fatal('name attribute required for data element');
			
		if (($v=$node->getAttribute('value'))===null)
			return $this->fatal('value attribute required for data element');
			
		$v=$this->parseVal($v,true);		
		
		$this->getAttribute($media,$node,'media','db');
		if ($media == 'db')
			$sel='db';
		else
			$sel='ds';
			
		$src="{$pad}'{$n}.{$sel}'=>$v,\n";
		$src.="{$pad}'{$n}.media'=>'$media',\n";
		
		return $src;
	}
	
	function param($node,&$content,$level,$pad)
	{				
		/* name */
		if (($name=$node->getAttribute('name'))===null)
			$this->fatal('error in session:get : no "name" attribute');
			
		$node->setDeftAtt('value',$node->getAttribute('name'));
		
		/* value */
		$value = $this->parseVal($node->getAttribute('value'));
		$s="{$pad}\${$name}={$value};";		
		
		/* storage */
		if (($store=$node->getAttribute('store'))!==null)
		{
			$var='_'.strtoupper($store);
			$s.="
{$pad}\${$var}['$name']=\${$name};";
		}
			
		/* add param init */
		$this->context->addResource($s,'params');
		return '';		
	}
	
	// <view name="page.view" value="#param:ptype#.edit" />
	function view($node,&$content,$level,$pad)
	{
		if (($n=$node->getAttribute('name'))===null)
			return $this->fatal('name attribute required element');
		
		$n=$this->parseVal($n,true);
					
		if (!preg_match('/[.]view$/',$n))
			$n .= ".'.view'";
		
		if (($v=$node->getAttribute('value'))===null)
			return $this->fatal('value attribute required');
		$v=$this->parseVal($v,true);		
			
		return "{$pad}{$n}=>$v,\n";
	}

	// <view name="page.view" value="#param:ptype#.edit" />
	function property($node,&$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name',null,true,true))
			return $this->fatal('no name');

		$this->getAttribute($dft,$node,'default',$content);

		if (!$this->getAttribute($value,$node,'value',$dft,true,true))
			return $this->fatal('no value');
				
		return "{$pad}$name=>$value,\n";
	}
	
	// call parent and set property object to site descriptor
	function parseVal($v,$quotes=false,$wrapPhp=false,$dft='',$prop='$this')
	{
		return parent::parseVal($v,$quotes,$wrapPhp,$dft,'$siteDesc');
	}

	// ====== utility functions =======
	
	// require_once("../nx/NxSite.inc");
	function getSitePath($path) 
	{
		// add right / in case it is forgotten
		if (strlen($path)>1)
			$path=rtrim($path,'/').'/';
			
		$a=explode('/',$path);
		$len=count($a);	
		return str_repeat('../',$len-1).'nx/';
	}	

	function cleanupSrc($src) {
		return preg_replace(
			array('/<\?php\s*\?>/','/[.]\'\'[.]/'),
			array('','.'),
			$src);
	}
}

?>
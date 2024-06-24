<?php

class application_base
{
	function __construct($lib,$appType) {
		$this->lib = $lib;
		$this->appType = $appType;
	}

	function generate_page($name,$package,$path,$role,$application,$skin,$lib,$node) {
		// file path
		$fpath = NX_DOC_ROOT.trim($path,'/')."/{$name}.php";

		$src=$this->page($package,$path,$role,$application,$skin,$fpath,$lib,$node);
		
		nxltrace("PAGE: $fpath = [<pre>$src</pre>]",'listing');	
		if (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES) {
			if($this->appType=='angular')
				$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/api/{$name}.php";
			else
				$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/{$name}.php";
			$this->lib->context->saveSrc($fpath,$src);
			nxltrace("PAGE: $fpath = [<pre>(copy)</pre>]",'listing');	
		}
	}

	function genNxlAll($package,$page) {
		$fs = Framework::fs();
		$packagePath = GENERIS_PACKAGES.$package.'/';	

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


	}

	function generate($node,$parent) {
		$lib = $this->lib;
		$model = $this;

		// decode package (and remove package suffix on "logic" param)
		$package=$lib->decodePackage($node,'inherit','logic',true);	
		
		// get cache mode
		$iscache=$lib->getBool($lib->getLastResource('page_cache',true));
		
		if(!$parent)
			$parent=$lib->context->getParentElement($node);

		// get params
		$params=$lib->context->getResources('params',"\n");
		// $src.= "\n$params";
		
		// get path
		$path=$lib->getLastResource('path',true);
		
		// get name
		$lib->getAttribute($name,$node,'name','index');				

		$lib->getAttribute($page,$parent,'page',$name);
		$lib->getAttribute($application,$node,'application',$name);
		$lib->getAttribute($table,$node,'table',$application);
		$lib->getAttribute($base_data,$node,'base_data',$table);
        $lib->getAttribute($strings,$node,'strings',$application);
        $lib->getAttribute($skin,$node,'skin','default');
        $lib->getAttribute($string_base,$node,'string_base','record');
        
        $lib->getAttribute($default_op,$node,'default_operation','list');
        $lib->getAttribute($role,$node,'role','default');
        $lib->getAttribute($isCreator,$node,'isCreator',false);
		$isCreator=($isCreator!='false')&&($isCreator!='no');

        $lib->getAttribute($isValidator,$node,'isValidator',false);
		$isValidator=($isValidator!='false')&&($isValidator!='no');

		if(!$strings)
			$strings = $application;

		$packagePath = GENERIS_PACKAGES.$package.'/';	
		$fs = Framework::fs();

		$this->generate_page($name,$package,$path,$role,$application,$skin,$lib,$node);
		
		/*
		// file path
		$fpath = NX_DOC_ROOT.trim($path,'/')."/{$name}.php";

		$src=$model->page($package,$path,$role,$application,$skin,$fpath,$lib,$node);
		
		nxltrace("PAGE: $fpath = [<pre>$src</pre>]",'listing');	
		if (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES) {
			if($appType=='angular')
				$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/api/{$name}.php";
			else
				$fpath=GENERIS_PACKAGES.$package.'/www/'.trim($path,'/')."/{$name}.php";
			$lib->context->saveSrc($fpath,$src);
			nxltrace("PAGE: $fpath = [<pre>(copy)</pre>]",'listing');	
		}
		*/

		
		// ========= DESCRIPTEUR APPLICATION 
		if(!is_dir($d=$packagePath.'application/'))
		{
			$fs->createTree($d,0777);
		}

		if(!is_file($f=$d."{$application}.inc"))
		{
			$s = $model->appDesc($package,$application,$processus,$role,$table,$base_data,$strings,$perms,$isCreator,$isValidator);
			$lib->context->saveSrc($f,$s.'?>');
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

		$map = array('/evalmag/'=>$package,'/modelcomp/'=>$application);

		$lib->getAttribute($section,$node,'angular_section',null);

		// map module to an angular section (ex. /modules/section/app_mod.html)
		if($section)
		{
			$section = trim($section,'/');
			$map['|/section/|']="/$section/";

			$sectionState = str_replace('/','.',$section);
			$map['|main[.]section|']="main.$sectionState";
		}
		else
		{
			$map['|/section/|']="/";
			$map['|main\.section|']="main";
		}

		$lib->getAttribute($string_mapping,$node,'string_mapping','');
		$stringsMap = explode('|',$string_mapping);
		foreach($stringsMap as $stringMap)
		{
			$a = explode('=',$stringMap);
			if(count($a)>1)
			{
				$map['|'.$a[0].'|']=$a[1];
			}
		}

		$this->deployAndMap($package,$application,$map);

		return '';
	}

	function deployAndMap($package,$application,$map) {

		$appType = $this->appType;

		//============ FICHIERS ANNEXES ===============
		$packagePath = GENERIS_PACKAGES.$package.'/';	
		
		// deply resources		
		$this->deployModelFiles('view',$packagePath,$map,$appType);
		$this->deployModelFiles('conf',$packagePath,$map,$appType);

		$this->deployModelFiles('www',$packagePath,$map,$appType);

		// deploy to www
		$this->deployModelFiles('www',NX_HOST_ROOT,$map,$appType);

		return '';
	}

	function deployModelFiles($dir,$packagePath,$map,$appType) {
		$fs = Framework::fs();

		$here = dirname(__FILE__).'/';
		$fromDirPath = $here.''.$appType.'/'.$dir.'/';
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

	function page($package,$path,$role,$application,$skin,$fpath,$lib,$node)
	{
		$src= '<?php
/* file automatically generated from NXL source ['.$lib->context->getFileURI().']. Do not edit it. */
		
';	
		$lib->getAttribute($perms,$node,'permissions','default');

		// 'protected' => 'no',
		$lib->getAttribute($isprot,$node,'protected','no');
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
		
		if ($lib->getBool($isprot))
			$src .= '$loginPerm="default";
';
	    	
		// pop skin
		$lib->context->getLastResource('skin',true);

		// get mime and encoding
		$lib->getAttribute($type,$node,'type','html');
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
		$lib->getAttribute($mime,$node,'mime',$mime);
		
		$src.="global \$NX_MIME;\n\$NX_MIME='$mime';\n";
		
		// get site conf path
		//$path=$lib->getLastResource('path',false);

		$sitePath = $lib->getSitePath($path).'NxSite.inc';
		$src.="require_once('$sitePath');";
				
		$content = <<<EOH

include_once GENERIS_PACKAGES."workflow/lib/workflow/ExecuteApp2.inc";

/** MODIFIER CES VALEURS (si besoin)**/
\$package = '{$package}';
\$app  = '{$application}';
\$role = '{$role}';
\$skin = '{$skin}';
/** FIN MODIF **/

// nom application et package, par defaut, on utilise le nom du fichier comme application

// use generic application application bootstrap that loads the application from the package
\$page=new ExecuteApp2(
	array(
		'app.id' => "\$app@\$package",
		'app.role' => \$role,
		'app.defaultOperation'=>"{\$default_op}",
		'nx.package' => \$package,
		'application.dir'=>'application',
		'page.skin' => \$skin,
	),
	\$siteDesc);

/* call the application */
try {
\$page->run();
}
catch(Exception \$e) {
	echo \$e;
}

EOH;
		$package=$lib->context->getLastResource('package',true);

		$src.= "\n$content?>";
		$src = $lib->cleanupSrc($src);
		$lib->context->saveSrc($fpath,$src);

		return $src;
	}

	function remapStrings($str,$string_base,$isFemale=null,$startsVowel=null)
	{
		$string_base_mul = $string_base.'s';
		$string_base_maj = ucfirst($string_base);

		// start with vowel
		if($startsVowel===null)
			if(preg_match('/^[a-zéèàê]/i',$string_base))
				$startsVowel=true;
			else
				$startsVowel=false;

		if($isFemale===null)
			if(preg_match('/(lle|té|e)$/i',$string_base))
				$isFemale=true;
			else
				$isFemale=false;

		$re = array(
			'/xxxs/' => $string_base_mul,
			'/xxx/' => $string_base,
			'/Record\b/' => $string_base_maj
		);

		if($startsVowel)
			$re +=  array(
				'/ du /'=>' de l\' ',
			);

		if($isFemale)
		{
			if ($startsVowel)
				$re +=  array(
					'/ du /'=>' de l\' ',
					'/ le /'=>" l' ",
				);

			else
				$re +=  array(
					'/un nouveau/'=>'une nouvelle',
					'/ le /'=>' la ',
					'/ du /'=>' de la ',
				);

			$re +=  array(
				'/Aucun/'=>'Aucune',
				'/enregistré/'=>'enregistrée',
				'/ sélectionné /'=>' sélectionnée ',
				'/un\b/'=>'une',
				'/\bce /'=>'cette ',
			);		
		}
		else 
		{
			if ($startsVowel)
				$re +=  array(
					'/ du /'=>" de l' ",
					'/un nouveau/'=>'un nouvel',
					'/ le /'=>" l' ",
					'/\bce /'=>'cet ',
				);
		}

		$s = preg_replace( 
				array_keys($re),
				array_values($re), $str);

		return $s;
	}
}

?>
<?PHP

class NXL_genPages
{
	function __construct(&$fs,$path)
	{
		$this->fs=&$fs;
		$this->path=$path;
	}
	
	function genPages($pages) 
	{
		foreach ($pages as $pname=>$page)
		{
			$path = $this->path.ltrim($page['path'],'/').$pname.'.php';
			echo "creating page $path;";
			
			// create page
			$this->fs->saveFile($path,
								$this->getPageSrc($page));
			
		}
	}

	function getPageSrc($page)
	{
		$signature="\n/* file automatically generated from NXL source. Do not edit it. */\n";
		$sitePath = $this->getSitePath($page['path']);
		$pageOpts ='	define("NX_SKIN_SERVER","default");
	define("NX_HTTP_Cache",false);
	$NX_MAP_DEVICE= array("IE55"=>true, "N4"=>true, "N6"=>true, "OP"=>true);
';
		$props=$this->getProps($page);
		
		$src= '<?php'.
			$signature.
			$this->getOptions($page).
			'
require_once("'.$sitePath.'");

$siteDesc->runPage(array(
// page template properties
';
	foreach ($this->getProps($page) as $n=>$opt)
	{
		$src .= "'$n'=>$opt,\n";
	}
	$src .="\n// data sources\n";
	foreach ($this->getDSProps($page) as $n=>$opt)
	{
		$src .= "'$n'=>$opt,\n";
	}
	$src .="\n// Views\n";
	foreach ($this->getViewProps($page) as $n=>$opt)
	{
		$src .= "'$n'=>$opt,\n";
	}

	return $src.'));

?>';
	}
	
	// page options
	function getOptions($page)
	{
		$s='';
		
		// 'protected' => 'no',
		if (isset($page['protected']) && $this->getBool($page['protected']))
			$s .= '$loginPerm="default";'."\n";
	    	
		// 'cache' => 'no',
		if (isset($page['cache']) && $this->getBool($page['cache']))
			$s .= 'define("NX_HTTP_Cache",true);'."\n";
		else
			$s .= 'define("NX_HTTP_Cache",false);'."\n";

		// 'skin' => 'default',
		if (isset($page['skin']))
			$s .= 'define("NX_SKIN_SERVER","'.$page['skin'].'");'."\n";
			
		return $s;
	}
	
	function getBool($v) {
		$v=strtolower($v);
		if ($v=='yes' || $v=='true')
			return true;
			
		if ($v=='no' || $v=='false')
			return false;
		
		nxError('unknown value for NXL page attribute:'.$v);	
		return false;
	}
	
	// require_once("../nx/NxSite.inc");
	function getSitePath($path) 
	{
		$a=explode('/',$path);
		$len=count($a);	
		return str_repeat('../',$len-1).'nx/NxSite.inc';
	}
	
	function getProps($page)
	{
		$props=array();

		// 'skin' => 'default',
		if (isset($page['skin']))
			$props['page.skin']=$this->parseVal($page['skin']);
				
		return $props;
	}
	
	function getDSProps($page)
	{
		$props=array();
	
		// data sources
		foreach($page['ds'] as $n=>$ds)
		{
			$props[$n.'.ds']=$this->parseVal($ds['value']);
			if ($ds['media'])
				$props[$n.'.media']=$this->parseVal($ds['media']);
		}
				
		return $props;
	}
	
	function getViewProps($page)
	{
		$props=array();
				
		// tpl views
		foreach($page['display'] as $n=>$view)
		{
			if (!preg_match('/[.]view$/',$n))
				$n .= '.view';
				
			$props[$n]=$this->parseVal($view['value']);
		}
		
		return $props;
	}
	
	function parseVal($v)
	{
		return preg_replace(
			array(
				'/#(?:user):([^:#]+(?:[:]([^#:]+))?)#/i',
				'/#(?:prop(?:erty)?|=):([^:#]+(?:[:]([^#:]+))?)#/i',
				'/#(?:get|param(?:eter)?):([^:#]+(?:[:]([^#:]+))?)#/i',
				'/#(?:post):([^:#]+(?:[:]([^#:]+))?)#/i',
				'/#(?:req(?:uest)?):([^:#]+(?:[:]([^#:]+))?)#/i',
				'/#(?:session):([^:#]+(?:[:]([^#:]+))?)#/i',
				'/#(?:const(?:ant)?):([^:#]+(?:[:]([^#:]+))?)#/i',
				"/(^''[.])|([.]''$)/"
			),
			array(
				'\'.$GLOBALS["_NX_user"]->getProperty("$1","$2").\'',
				'\'.$GLOBALS["$_NX_site"]->getProperty("$1","$2").\'',
				'\'.(isset($_GET[\'$1\'])?$_GET[\'$1\']:\'$2\').\'',
				'\'.(isset($_POST[\'$1\'])?$_POST[\'$1\']:\'$2\').\'',
				'\'.(isset($_REQUEST[\'$1\'])?$_REQUEST[\'$1\']:\'$2\').\'',
				'\'.(isset($_SESSION[\'$1\'])?$_SESSION[\'$1\']:\'$2\').\'',
				'\'.$1.\'',
				'',
			),
			"'$v'");
	}
}

?>
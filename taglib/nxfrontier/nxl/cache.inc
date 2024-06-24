<?PHP

// data sources
class taglib_nxfrontier_nxl_cache extends NxTagLib
{	
	function header()
	{
		return "// import library : cache";
	}
	
	/** Set cache parameters
	 <cache:set dir="/article/#global:NX_DOMAIN#/#get:topic#/" 
	  	name="#get:page#.html" 
			default="index.html"/>
	 	$NX_CACHE=NX_DOC_ROOT."article/$NX_DOMAIN/{$_GET['topic']}/";
		$NX_CACHE_ID=(($_GET["page"]!='')?$_GET["page"]:'index'). '.html';
	*/
	
	function buffer($node,$content,$level,$pad)
	{
		/* name */
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('error in nxl:get : no "name" attribute');
			
		$an=explode(":",$name);
		if (isset($an[1]))
		{
			$name=$an[1];
			$sel=$an[0];
		}
		else
		{
			$sel='property';
		}

		// get content in buffer
		$ret=$this->createVarID('ret');
		$s="\n{$pad}ob_start();\n{$pad}{$content}\n{$pad}\${$ret}=ob_get_contents();ob_end_clean();";
		$value="\${$ret}";
		
		// parse TPL content (only properties, in real time)
		if ($this->getAttribute($parse,$node,'parser'))
		{
			if (strtolower($parse)!=='tpl')
				return $this->fatal('unknown tpl parser for property '.$name);
				
			$value= "preg_replaceX(array('/{@?(?:=|property:)([^}]+)}/ie',),
				{$pad}	array('\$this->getProperty(\"$1\")'),
				{$pad}	{$value},\$this)";
		}	

		// store content in variable
		switch(strtolower($sel))
		{
			case 'request':
			case 'post':
			case 'cookie':
			case 'session':
			case 'get':
				$var='_'.strtoupper($sel)."['$name']";
				return "$s\n{$pad}\${$var}=$value;";	
				break;			
			case 'prop':
			case 'property':
				return "$s\n{$pad}\$this->setProperty('{$name}',$value);";
				break;			
			case 'var':
			case 'param':
			case 'parameter':
				return "$s\n{$pad}\${$name}={$value};";		
				break;			
			default:
				return "$s\n{$pad}/*set : unknown variable $sel:$name*/";
		}
	}

	function set($node,$content,$level,$pad)
	{
		$s='';
		$this->getAttribute($default,$node,'default','index',true,true);
		$this->getAttribute($type,$node,'type','html',true,true);
		
		if($this->getAttribute($name,$node,'name',null,true,true))
		{
			$s.="$pad\$cache_id=$name;\n";
			$s.="$pad\$GLOBALS['NX_CACHE_ID']=((\$cache_id!='')?\$cache_id:($default)).'.'.($type);\n";
		}

		if($this->getAttribute($dir,$node,'dir',null,true,true))
		{
			$s.="$pad\$GLOBALS['NX_CACHE']=NX_DOC_ROOT.rtrim($dir,'/ ').'/';\n";
		}

		return $s;
	}

	function _section(&$node,$level)
	{
		$this->context->declareBlock($node,true);
		return true;
	}
	
	//<cache:section path="article.#$aid#.edit">...</cache:section>
	function section($node,$content,$level,$pad)
	{
		return $content;
	}
	
	function _page_section($node) {
			// is cache ?
		$this->getAttribute($cache,$node,'cache','no');
		$this->addResource($cache,'page_cache');
	}	
	
	function page_section($node,$content,$level,$pad) {
		$s = "\ndefine('NX_HTTP_Cache',true);\n";
		$this->getAttribute($default,$node,'default','index',true,true);
		$this->getAttribute($type,$node,'type','html',true,true);
		
		if($this->getAttribute($name,$node,'name',null,true,true))
		{
			$s.="\$cache_id=$name;\n";
			$s.="\$GLOBALS['NX_CACHE_ID']=((\$cache_id!='')?\$cache_id:($default)).'.'.($type);\n";
		}
		
		$this->getAttribute($storage,$node,'storage','document_root',true,false);
		switch (strtolower($storage))
		{
			case 'public':
			case 'document_root':
				$root = 'NX_DOC_ROOT';
				break;
			case 'nx_gen':
			case 'protected':
			default:
				$root = 'NX_GEN';
				break;
		}

		if($this->getAttribute($dir,$node,'dir','/cache',true,true))
		{
			$s.="\$GLOBALS['NX_CACHE']=$root.trim($dir,'/ ').'/';\n";
		}
		
		if(!$this->getAttribute($compress,$node,'compress',null,true,false))
		{
			// compress only html..
			if ($type=='html')
				$compress='yes';
			else
				$compress='no';
		}
		$compress=$this->getBool($compress)?'true':'false';
		
		if(!$this->getAttribute($timeout,$node,'timeout',null,true,false))
		{
			$timeout='TIME_OUT';
		}
		
		$this->getAttribute($cancel,$node,'cancel','false');
		
		return $s.'
include(NX_LIB.\'cache/NxCache_Page2.inc\');
$cache=new NxCache_Page2();	
if (!$cache->load($NX_CACHE,$NX_CACHE_ID,'.$timeout.','.$compress.'))
{
'.$content.'
	$cache->endCache('.$cancel.');
}
';	
		// reset cache param
		$cache=$this->getLastResource('page_cache',true);
	}
	
	//<cache:start_section path="article.#$aid#.edit" />...<cache:end_section>
	// equivalent of "section" tag but can be used when an xml element does not fit
	// the code structure...
	function start_section($node,$content,$level,$pad)
	{		
		if (!$this->getAttribute($dir,$node,'dir',null,true,true))
			$dir='/cache';

		if(!$this->getAttribute($name,$node,'name',null,true,true))
			return $this->fatal('no name attribute');

		$this->getAttribute($type,$node,'type','html',true,true);		

		$this->getAttribute($storage,$node,'storage','document_root',true,false);
		switch (strtolower($storage))
		{
			case 'public':
			case 'document_root':
				$root = 'NX_DOC_ROOT';
				break;
			case 'nx_gen':
			case 'protected':
			default:
				$root = 'NX_GEN';
				break;
		}

		$dir="$root.trim({$dir},'/ ').'/'";
		$resID="{$name}.'.'.{$type}";
		$this->context->addInclude('cache/NxCache_Section2.inc');
		
		return '
	if (!isset($sec_cache))
	{
		$sec_cache=new NxCache_Section2();	
		if ($sec_cache->load('.$dir.','.$resID.','.$type.'))
			return true;
	}
';
	}
	
	function end_section($node,$content,$level,$pad)
	{
		$this->getAttribute($cancel,$node,'cancel','false');
		return 'if (isset($sec_cache))
	$sec_cache->endCache('.$cancel.');
';
	}
	
	function invalidate($node,$content,$level,$pad)
	{
		$this->getAttribute($storage,$node,'storage','document_root',true,false);
		switch (strtolower($storage))
		{
			case 'public':
			case 'document_root':
				$root = 'NX_DOC_ROOT';
				break;
			case 'nx_gen':
			case 'protected':
			default:
				$root = 'NX_GEN';
				break;
		}

		if(!$this->getAttribute($dir,$node,'dir',null,true,true))
			return $this->fatal('no dir attribute');
			
		if($this->getAttribute($runat,$node,'runat','runtime',true,false) && strtolower($runat)=='runtime')
		{			
			return "{$pad}\$fs=\$this->getPlugin('FileSys');\n{$pad}\$fs->rmDir($root.trim($dir,'/'));\n";
		}
		else
		{
			$pathE=trim($root,'\'').'.'.trim($dir,'/"');
			eval("\$path=$pathE;");
			$path=preg_replace('#[\\\\/]+#','/',$path);
			if (is_dir($path))
			{
				$fs=$this->props->getPlugin('FileSys');
				$fs->rmDir($path);
			}
			return '';
		}
	}

}

?>
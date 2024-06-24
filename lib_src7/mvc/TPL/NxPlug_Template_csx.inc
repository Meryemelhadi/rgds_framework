<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 	transform a csx file into a css file.
 	
 	Supports the same directive set as HTML templates (tpl) but in the form of css2 directive syntax.
 	ex: 
 		@property(prop) or @prop
 		@if(cond) css_action @elseif(cond) action @else action @endif
 		@include(page.othercss.css)
 		@set:variable v = ?value?;
*/
class NxPlug_Template_csx extends NxPlug_Template
{
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->ext='css';
		$this->t_ext='csx;tpl';
		$this->d_ext='csx.php';
		$this->pathOffset='res/';
	}

	function parseLess($src,$src_path)
	{
		require "lessc.inc";
		$less = new lessc;
		try 
		{
		  return "/* LESS CODE COMPILED : */\n".$less->compile($src);
		} 
		catch (exception $e) {
		  return " ERROR CANT COMPILE LESS FILE $src_path\n".$src;
		}
	}


	// preprocess includes: read files and insert them in source
	// NB. accept tpl and css2 type of syntax
	function processIncludes($src,$t_ext='',$src_path=null)
	{
		$cur_dir=dirname($src_path);
		
		// csx supports the inclusion of tpl files (ex. for loading variables)
		if ($t_ext == 'tpl')
		{
			return parent::parse($src);
		}

		if ($t_ext == 'less')
		{
			return $this->parseLess($src,$src_path);
		}
		
		$patterns = array(
			'/{--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{!--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{@?include:([^=:}]+)(:([^:{}=]+))?}/e',
			'/@include\(([^=:,)]+)(,([^,{}()=]+))?\)/e',
			'/@import\s*\(?\s*"?\s*([^=:",)]+)"?\s*\)?\s*;?/e'
			);
			
		$replace = array(
			'',
			'<!--\\1-->',
			'$this->mapInclude("$1","$2")',
			'$this->mapInclude("$1","$2")',
			'$this->mapImport("$1","",\''.$cur_dir.'\')',
			);
			
		return preg_replaceX($patterns, $replace, $src,$this); 		
	}
	
	function transformNXL($src)
	{
 		return $src;
	}
	
	// =================== RESOURCES ==============
	function processResources($src)
	{
		// nxLog("process resources for:[$src]",'TPL_CSS');
		
		$this->reqs = array();	
		$patterns = array(
			'#url\s*[(]\s*"?\s*(?:[.][.](?:[/]res)?[/])?(images|img)[/]([^)"]+)\s*"?\s*[)]#ie',
			);
			
		$replace = array(
			'$this->requireRes("$1","$2")',
			);
			
		return preg_replaceX($patterns, $replace,$src,$this);	
	}
	
	function compress($src,$path) {
		if (@NX_COMPRESS_RES===true)
			$src=preg_replace('/([ \t])+/',' ',$src);
			
		return "/* generated
*/\n\n
$src";
	}
	
	function requireRes($type,$name)
	{
		nxLog("RequireRes - start: {$name}.{$type}",'TPL_CSX');
		global $NX_SKIN_URL;
		
		nxLog("require/map url: ($type)/{$name} to NX_SKIN_URL/{$type}/{$name}","TPL_CSX");
		$url=$NX_SKIN_URL.$type.'/'.$name;
		
		// get image extension, add resource for parsing/deployment
		$ext = substr(strrchr($name,'.'),1);
		$n=substr($name,0,strlen($name)-strlen($ext)-1);
		nxLog("resource full name:$name, basename=$n, ext:$ext","requireImg");	
		$this->addDeployRes(array("name"=>"{$type}/{$n}","type"=>$ext,"ext"=>$ext,"isDevice"=>"false","isSkin"=>"true"));
		
		nxLog("RequireRes - end",'TPL_CSX');
		return "url({$url})";

	}
		
	// parse a string in csx format and returns a php view
	function parse($src,$ext='tpl')
	{
		// process image, csx and jsx files
		$src=$this->processResources($src);
		
		$patterns = array(
			'/<?php([^?]+)\?\>/si',
			'/@(property\w*\()([^})|=]+)([|=]([^)]+))?\)/e',
			'/@(get|post|request|cookie|param|user|session)\w*\(([^)}|:]+)([|:]([^)}]+))?\)/e',
			'/@(const(?:ant)?)\w*[(]([^()}{|:=]+)(([|])([^()}{]+))?\)/e',
			'/@(global)\w*[(]([^)}|:=]+)(([:=|])([^()}{]+))?\)/e',
			'/@var(iable)?\w*[(]([^)}|:=]+)(([:=|])([^(){}]+))?\)/e',
			'/@require\w*[(]([^=:.}]+)[.]([^ :,}{=.]+)([:,]([^:{}=]*)(:([^{};=]+))?)?[)]/e',
			// '/@require[(]([^=):.}]+)[.]([^ :}{()=.]+)(:([^:(){}=]+)(:([^(){};=]+))?)?\)/e',
			'/@if\w*[(]([^)]+)[)]/e',
			'/@elseif\w*[(]([^=:)]+)\s*(:([^:)]*)(:([^)]+))?)?\)/e',
			'|@else\w*([(]\w*[)])?|',
			'|@endif\w*([(]\w*[)])?|',
			);
			
		$replace = array(
			'$0', // '$this->mapPHP("$1")',
			'$this->mapProp("$2","$4",false,false)',
			'$this->mapQProp("$1","$2","$4",false,false)',
			'$this->mapK("$2","$4","$5",false,false)',
			'$this->mapGlobal("$2","$4","$5",false,false)',
			'$this->mapLocal("$2","$4","$5",false,false)',
			'$this->mapReq("$1","$2","$4","$6")',
			'$this->mapIf("$1","","")',
			'"<?php }else ?>".$this->mapIf("$1","","")',
			'<?php
		} /*if*/
		else
		{ 
		?>',
			'<?php 
		} /*if*/ 
		?>',
			);
			
		return $this->cleanup(preg_replaceX($patterns, $replace,$src,$this)); 	
	}
	
	function cleanup($s)
	{
		$matches=array(
			'/<\?=\s*(.*[^\s]+)\s*\?>/',
			'/\?>(\s*)<\?(php)?([^=])/'
			);
		
		$replace=array(
			'<?php echo \\1; ?>',
			'\\1\\3'
			);
		
		return preg_replace($matches, 
					$replace,
					$s);			
	
	}
	
	function mapInclude($path,$opts)
	{
		$path=str_replace('.','/',$path);

		// $isSkin=(stristr($opts,"skin")!=''?"true":"false");
		$isDevice=(stristr($opts,"device")!=''?"true":"false");
		if ($isDevice=='true')
			$device=$this->device;
		else
			$device='';

		// include another view at build time
		$aexts=explode(';',$this->t_ext);
		//$apaths=explode(';',$this->rootPaths);

		foreach ($this->rootPaths as $srcRoot)
		{
			$basePath = $srcRoot.$this->pathOffset.$path;

			foreach ($aexts as $t_ext)
			{
				if (is_file($f="{$basePath}{$device}.{$t_ext}") ||
					is_file($f="{$basePath}.{$t_ext}") )
				{
					// return $this->file_get_contents($f);
					nxLog("include TPL file:$f",'TPL_INC');
					return $this->processIncludes($this->fs->file_get_contents($f),$t_ext,$f);
				}
			}
			
			// no template => search raw format files (not templated)
			$ext=$this->ext;
			if (is_file($f="{$basePath}{$device}.{$ext}") || 
				is_file($f="{$basePath}.{$ext}"))
			{
				// return $this->file_get_contents($f);
				nxLog("include PHP file: $path ",'TPL');
				return "/* import : $path (start) */".$this->fs->file_get_contents($f)."/* import : $path (end) */";
			}
		}
		
		nxError("failure 3 in template inclusion : $path",'NxPlug_Template');
		return "{-- failed include: $path --}";
	}
	
	function mapImport($path,$opts,$cur_dir)
	{
		nxLog("CSX import:$path (from $cur_dir)",'TPL:CSX');

		// search in same directory first
		nxLog("CSX import try :".$cur_dir."/{$path}",'TPL:CSX');
		if (is_file($f=$cur_dir."/{$path}"))
		{
			// return $this->file_get_contents($f);
			nxLog("import TPL file:$f",'TPL');

			$src = $this->fs->file_get_contents($f);
			$af = explode('.',$f);
			$ext = end($af);
			if($ext=='less')
			{
				$src = $this->parseLess($src,$f);
			}

			// return $this->processIncludes($this->fs->file_get_contents($f),'css',$f);
			return "/* ====== @import($path) : START ======= */\n\n".$src."\n/* ====== @import($path) : END ======= */\n\n";
		}

		foreach ($this->rootPaths as $srcRoot)
		{
			$basePath = $srcRoot.$this->pathOffset.$path;
			$basePath2 = $srcRoot.$this->pathOffset.'css/'.$path;

			nxLog("CSX import try :{$basePath}",'TPL:CSX');
			if (is_file($f="{$basePath2}")||is_file($f="{$basePath}"))
			{
				// return $this->file_get_contents($f);
				nxLog("include TPL file:$f",'TPL_INC');

				$src = $this->fs->file_get_contents($f);
				$ext = end(explode('.',$f));
				if($ext=='less')
				{
					$src = $this->parseLess($src,$f);
				}
				// return $this->processIncludes($this->fs->file_get_contents($f),$t_ext,$f);
				return "/* ====== @import($path) : START ======= */\n\n".$f."\n/* ====== @import($path) : END ======= */\n\n";
			}
		}
		
		nxError("failure 4 in template inclusion : $path from $cur_dir",'NxPlug_Template');
		return "{-- failed include: $path in $cur_dir --}";
	}
	/*
	// add the resource requirement
	function requireRes($name,$ext)
	{
		nxLog("Require Res: $name.$ext","TPL_RES");
		
		$baseName = preg_replace('/([.].+)$/','',$name);
		$noDeviceName = preg_replace('/(?:_IE|_N4|_GK|_IE55)$/','',$baseName);

		if ($noDeviceName==$baseName)
			$isDevice = false;
		else
			$isDevice = true;
		
		$this->appView->requireRes($ext,$noDeviceName,true,$isDevice);
		
		nxLog("Require Res - end","TPL_RES");
		return '';
	}
	*/
}
	

?>
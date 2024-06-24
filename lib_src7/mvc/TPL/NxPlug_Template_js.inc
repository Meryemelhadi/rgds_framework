<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 	transform a jsx file into a js file.
 	
 	Supports the same directive set as HTML templates (tpl) but in the form of @directive syntax.
 	ex: 
 		@property(prop) or @prop
 		@if(cond) css_action @elseif(cond) action @else action @endif
 		@include(page.othercss.css)
 		@set:variable v = ?value?;
*/
class NxPlug_Template_js extends NxPlug_Template
{
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->ext='js';
		$this->t_ext='jsx;js';
		$this->d_ext='jsx.php';
		$this->pathOffset='res/';
	}

	function execute($exe,&$res)
	{
		global $NX_SKIN_RES;
		global $NX_DEFT_SKIN_RES;
		
		if ($res['isSkin']=="true")
			$skin=$NX_SKIN_RES;
		else
			$skin=$NX_DEFT_SKIN_RES;
			
		if ($res['isDevice']=="true")
			$device=_NX_DEVICE_MAP;
		else
			$device='';
			
		$ext=$this->ext;	
		$name=$res["name"];

		$resPath="{$skin}{$name}{$device}.{$ext}";

		// transform it and write to file
		if ($fp = $this->fs->fopen($resPath,'w',true))
		{
			// get source
			ob_start();
			readfile($exe);
//			include($exe);
			$src=ob_get_contents();
			ob_end_clean();
		
			// parse and copy to file
			fputs($fp, $src);
			fclose($fp);
			
			nxLog("executed template $exe to $resPath","NxPlug_Template",NX_COMPILE_EVENT);
			return true;
		}
		else {
			nxError("cannot create template transformation to file:{$resPath}");
			return false;
		}
	}	
	
	function compress($src,$path,$force=false,$fullpath=false) {

		if ($force=='false' && @NX_COMPRESS_RES!==true)
			return $src;

		if ($force=='hard' && @NX_COMPRESS_RES!==true)
			return $this->hardCompress($src,$path,$force,$fullpath);
			
		// get rid of comments and unwanted whites
		$res = "/* compressed from: $path */\n\n".
		preg_replace(array(
				"/[ \t]+/", // tabs
				"#\n//.*\n#", // comments 1
				
				// BUG PHP 4
				//'#/\*(((.|\n)(?!(\*/|[\'"])))*)(.)?\*/#s', // comments 2 (but do not cross string)
				"/([ ]*\n[ ]*)+/", // empty lines
				),
			array(
				' ',
				"\n",
				//' ',
				"\n"
				),$src);
		
		return $res;					
	}

	function hardCompress($src,$path,$force=false,$fullpath=false) {
		
		if (!$fullpath)
			return $src;	

		$fullpath=preg_replace(array('#/./#','%[/]+%'),array('/','\\'),$fullpath);

		$cmd = 'java -jar \webs\nx_dis_1.5\tools\yuicompressor-2.4.2.jar -v "'.$fullpath.'" 2>1';
		$result = array();
		exec( $cmd,$result);
		$src = implode("\n",$result);
       
		// get rid of comments and unwanted whites
		$res = "/* hard compressed from: $path */\n\n".$src;
		
		return $res;					
	}

	// preprocess includes: read files and insert them in source
	// NB. accept tpl and css2 type of syntax
	function processIncludes($src,$ext='',$src_path=null)
	{
		$cur_dir=dirname($src_path);

		// supports the inclusion of tpl files (ex. for loading variables)
		if ($ext == 'tpl')
		{
			return parent::parse($src);
		}

		// @include directives
		$patterns = array(
			//'/{--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{!--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{@?include:([^=:}]+)(:([^:{}=]+))?}/e',
			'/@include\(([^=:,)]+)(,([^,{}()=]+))?\)/e',
			'/\b@?require\s*\(?\s*[\'"]\s*([^=:\'",)]+)[\'"]\s*(,[\'"]?\s*([^=:"\',)]+)[\'"]?)?\)?\s*;?/e'
			);
			
		$replace = array(
			// '',
			'<!--\\1-->',
			'$this->mapInclude("$1","$2")',
			'$this->mapInclude("$1","$2")',
			'$this->mapImport("$1","$3",\''.$cur_dir.'\')',
			);
			
		return preg_replaceX($patterns, $replace, $src,$this); 		
	}
	
	// parse a string in tpl format and returns a php view
	function parse($str,$ext='')
	{
		return $str;	
	}
	
	function cleanup($s)
	{
		$matches=array(
			'/<\?=\s*(.*[^\s]+)\s*\?>/',
			'/\?>(\s*)<\?(php)?([^=])/'
			);
		
		$replace=array(
			'<'.'?php echo \\1; ?'.'>',
			'\\1\\3'
			);
		
		return preg_replace($matches, 
					$replace,
					$s);
	}
	
	function mapImport($path,$compress,$cur_dir)
	{
		nxLog("JSX import:$path (from $cur_dir), compress=$compress",'TPL:JSX');
		$path=preg_replace(array('#^.*js/#','#/./#'),array('js/',''),$path);

		// search in same directory first
		nxLog("JSX import try :".$cur_dir."/{$path}",'TPL:JSX');
		if (is_file($f=$cur_dir."/{$path}"))
		{
			// return $this->file_get_contents($f);
			nxLog("import TPL file:$f (1)",'TPL');
			$src=$this->fs->file_get_contents($f);
			nxLog("import TPL file:$f : got source",'TPL');
			if ($compress!='false')
				$src=$this->compress($src,$path,$compress,$f);

			nxLog("import TPL file:$f : compressed source",'TPL');
				
			// return $this->processIncludes($this->fs->file_get_contents($f),'css',$f);
			return "/* ====== @import($path) : START ======= */\n\n".$src."\n/* ====== @import($path) : END ======= */\n\n";
		}

		foreach ($this->rootPaths as $srcRoot)
		{
			$basePath = $srcRoot.$this->pathOffset.$path;

			nxLog("JSX import try :{$basePath}",'TPL:JSX');
			if (is_file($f="{$basePath}"))
			{
				// return $this->file_get_contents($f);
				nxLog("include TPL file:$f (JSX)",'TPL_INC');
				$src=$this->fs->file_get_contents($f);
				
				nxLog("import TPL file:$f : got source",'TPL');
				if ($compress!='false')
					$src=$this->compress($src,$path,$compress,$f);

				nxLog("import TPL file:$f : compressed source",'TPL');
					
				// return $this->processIncludes($this->fs->file_get_contents($f),$t_ext,$f);
				return "/* ====== @import($path) : START ======= */\n\n".$src."\n/* ====== @import($path) : END ======= */\n\n";
			}
		}
		
		// finally search in doc root...
		if (is_file($f=NX_DOC_ROOT.$path))
		{
			nxLog("include PHP file:$f",'TPL');
			return $this->fs->file_get_contents($f);
		}

		nxError("failure 5 in template inclusion : $path from $cur_dir",'NxPlug_Template');
		return "{-- failed include: $path in $cur_dir ($f)--}";
	}
	
}
	

?>
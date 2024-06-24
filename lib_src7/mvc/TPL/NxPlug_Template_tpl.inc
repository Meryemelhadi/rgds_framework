<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 	transform a tpl file into a php file to be used as a nx html view.
 	syntax is described here after. NB. [] denotes part of expressions that are not mandatory.
 	
 	- properties:
 	 	{= prop_name[|default]} : output a property if exist or default text
 	
 	- records:
 		{records[:records_prop]} {/records} : loop on records found in records_prop property or "record".
 	- record:
 	{record[:record_prop]} {/records} : 
 	loop on record fields, record is found in record_prop property. If inside a records loop, can be omitted.
 	
 	- {form:props} {record[:record_prop]} {field}

 	- conditional
 	{if:cond:content[:else_content]} or
 	{if:cond} content [{elseif:cond}} elseif_content] [{else} else_content] {/if}
 	
 	- direct php code: <?[php] code ? > 

*/
require_once('NxPlug_Template.inc');

class NxPlug_Template_tpl extends NxPlug_Template
{
	var $buildlocals;
	
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->pathOffset='';
		$this->ext='inc';
		$this->t_ext='html;tpml;xhtml;tpl;xml;xsl;doc';
		$this->d_ext='inc';
	}

	function execute($exe,&$res)
	{
		return true;
	}

	// perform the transformation from source file to dest file. Call parse() for the actual
	// processing.
	function transform($src_path,$out_path,$checkDir=true,$ext="tpl",$res='')
	{
		// check if src file exist, dest dir exist and output up to date
		if (!is_readable($src_path))
		{
			nxError("template file does not exist or is not readable:{$src_path}");
			return false;
		}
		
		// obsolete ?
		$checkTime=true;
		
		// check if output is up to date
		if ( $this->mode==="build" &&
			 $checkTime && is_file($out_path) &&
			 @filemtime($src_path) < @filemtime($out_path))
		{
			// up to date...
			return true;
		}
		
		if ((isset($res['ispage']) && $res['ispage']) ||$ext=="dwt" || strstr($src_path,'/page/')!=false)
			$this->isPage=true;
		else
			$this->isPage=false;

		if ($ext=='tpml'|| $ext=='xhtml')
		{
			$srcT = str_replace(array('.tpml','.xhtml'),array('.tpl','.tpl'),$src_path);
			if (defined('NX_CHECK_TPML'))
				$mode2 = NX_CHECK_TPML;
			else
				$mode2='build';

			if (is_file($srcT) && ($mode2==='once ' || ($mode2=='build' && @filemtime($src_path) > @filemtime($srcT))))
			{
				$ext='tpl';
				$src_path=$srcT;
			}
		}

		if ($ext!='tpl')
			$this->translator=$this->props->getPlugin('TplMap_'.$ext,'NxPlug_','mvc/TPL');
		else
			$this->translator=$this;
		
		// check if output directory exist
		if ($checkDir && !is_dir($d=dirname($out_path)))
			if ($this->fs->createTree($d)==null)
				return false;
			
		// transform it and write to file
		
		// get file
		$src1 = $this->fs->file_get_contents($src_path);        
		// nxLog("initial file ------- \n".$src1,'TPL_TRANSF');
		
		// translate it from initial language (ex. dwt or html) to tpl
		$src =  $this->translator->translate($src1,$this);
		// nxLog("-----------\n 1) after translation \n------- \n".$src,'TPL_TRANSF');
        		
        if(!$src) {            
            $src =  $this->translator->translate($src1,$this);
        }
                
		// if template has a base property, rebase them
		/* obsolete?
		if ($this->propBase!='')
		{
			$src =  $this->rebaseProperties($src,$this->propBase);
			//nxLog("-----------\n 2) after document rebase \n------- \n".$src);
		}
		*/

		// process includes and objects            
		$src =  $this->processIncludes($src);
 		//nxLog("-----------\n 3) after include processing \n------- \n".$src,'TPL_TRANSF');
		
		// process NXL
		$src = $this->translator->transformNXL($src,$src_path);
		if (($ext=='tpml' || $ext=='xhtml') && $fp = fopen($srcT,'w'))
		{
			$src =  $this->processIncludes($src);
			// store NXL compiled
			fputs($fp, $src);
			fclose($fp);
		}
		
		// process image, csx and jsx files
		$src =  $this->processResources($src);
		//nxLog("-----------\n 3) after ressources processing \n------- \n".$src,'TPL_TRANSF');
		 
		// finally parse tpl language and produce php
		$src =  $this->parse($src,$ext);
		//nxLog("-----------\n 4) after tpl transformation \n------- \n".$src);
		
		$src =  $this->cleanup($src);
		//nxLog("-----------\n 5) after cleanup \n------- \n".$src);
		 
		$src =  $this->compress($src,$src_path);
		
		if ($fp=$this->fs->fopen($out_path,'w'))
		{
			// get source
			fputs($fp, $src);
			fflush ($fp);
			fclose($fp);
			nxLog("compiled template $src_path to $out_path",'NxPlug_Template',NX_COMPILE_EVENT);
			return true;
		}
		else {
			nxError("cannot create template transformation to file:{$out_path}");
			return false;
		}
	}

	function transformNXL($src)
	{
		return $src;
	}
	
	function compress($src,$path) {
		if (@NX_COMPRESS_RES===true)
		// get rid of comments and unwanted whites
			$src=preg_replace(array(
					"/[ \t]+/", // tabs
					'/<!--(((.)(?!-->))*)(.)?-->/s'
					),
				array(
					' ',
					' ',
					),$src);
						
		return $src;
	}

	
	// ================== PREPROCESSOR ===================
	/**
	 * extract TPL macros from html or javascript comments.
	 * ex. <!--! {record} --> or /*!{field} *
	 *
	 * @param string $src
	 * @param unknown_type $tpl
	 * @return string tpl source after translation
	 */
	function & translate(& $src,$tpl)
	{
		$src=preg_replace(array(
				'/<!--!((((.)(?!-->))*)(.)?)-->/s',
				'#[/][*]!((((.)(?![*]/))*)(.)?)[*]/#s'
				),
			array(
				'$1',
				'$1',
				),$src);
				
		return $src;
	}
	
	// preprocess includes: read files and insert them in source
	function processIncludes($src,$t_ext='',$src_path=null)
	{
		$patterns = array(
			// '/{--(((.|\n)(?!--}))*.)--}/',
			'/{--(((.)(?!--}))*)(.)?--}/s',
			'/{!--(((.)(?!--}))*)(.)?--}/s',
			// '/{preload:([^=|:}]+)(:([^=|:}]+))?([|]([^{}]+))?}/e',
			'/{@?include:([^=:}]+)(:([^:{}=]+))?}/e',
			'/<!--!((((.)(?!-->))*)(.)?)-->/s',
			'#[/][*]!((((.)(?![*]/))*)(.)?)[*]/#s',
			// '#{def-content:([^=:}]+)}(.*){/def-content}#e',	
			'/{content:([^}]+)}/e',
			'#{/content}#e',
			// '/{content:([^=|:}]+)([|]([^{}]+))?}/e',	
			'/{object:([^=|:}]+)(?:[:]([^=|:}]+))?(?:[|]([^{}]+))?}/e',
			'/{menu:([^=:}]+)(:([^:{}=]+)(:([^:{}=]+))?(:([^:{}=]+))?)?}/e',	
			'/{doc:([^=:}]+)(:([^:{}=]+)(:([^:{}=]+))?)?}/e',	
			);
			
		$replace = array(
			'','<!--\\1-->',
			// '$this->preload("$1","$2")',
			'$this->mapInclude("$1","$2")',
			'$1',
			'$1',
			'$this->mapContentStart("$1")',
			'$this->mapContentEnd("$1")',		
			// '$this->defContent("$1","$2")',
			// '$this->getContent("$1","$3")',
			'$this->getObject("$1","$2","$3")',
			'$this->mapMenu("$1","$3","$5","$7")',
			'$this->mapContent("$1","$3","$5")',
			);
			
		return preg_replaceX($patterns, $replace, $src,$this); 	
	}

	function preload($prop,$dft)
	{
		$static = true;
		
		// get path/name of module to load from property or default
		$dft = addslashes(trim($dft));
		$prop = trim($prop);
		$path=$this->props->getProperty($prop,$dft);

		if ($path=='')
			return "<!-- no TPL config set (prop:$prop|$dft) -->\n";
			
		// include the file
		return "<!-- TPL config set = $path -->\n".$this->mapInclude($path,$opts);
	}

	function mapContentStart($prop)
	{
		if (!$prop)
			$prop='page.content';
			
		
		return '<?php $_nx_content_stack[]="'.$prop.'";	ob_start(); ?>';	
	}

	function mapContentEnd()
	{
		return '<?php $this->setProperty(array_pop($_nx_content_stack),ob_get_contents());ob_end_clean();?>';
	}
	
	function defContent($name,$s)
	{
		if ($name!='')
		{
			$prop = "page.$name";
			// $this->content[$name]=$s;
			$this->props->setProperty($prop,$s);
		}
			
		return '';
	}
	
	// obsolete ?
	function getContent($name,$dft)
	{
		if ($name!='')
		{
			$prop = "page.$name";
			return $this->props->getProperty($prop,$dft);
		}
			
		return $dft;	
/*
		if ($name!='' && isset($this->content[$name]))
			return $this->content[$name];
			
		return $dft;
*/
	}
	
	function getObject($view,$propBase,$build)
	{
		// define property base for object
		$propBase=trim($propBase);
		if($propBase=='')
			$propBase=$view;
			
		// get object template path
		$path = str_replace('.','/',$view);
		$curpath = end($this->basePath);
		if ($curpath!='')
		{
			$curview = str_replace('/','.',$curpath).'.';
			$path=$curpath.'/'.$path;
			$view = $curview.$view;
		}
		else
			$curview='';

		// push current object base
		$this->basePath[] = $path;
		$this->objectBase=$view.'.';
		
		nxLog("mapping object:[$view]","TPL");

		// remap all includes/objects in the object file
		$code = $this->rebaseProperties($this->mapInclude($path,''),$propBase);
		$code = $this->rebaseObjectPaths($code,$view);

		// pop previous object base
		array_pop($this->basePath);		
		
		//nxLog("rebased object:[$view]------------\n".$code);
		return $code;
	}

	function rebaseProperties($code,$base)
	{
		if ($base=='')
			return $code;

		//nxLog("Rebase properties to :[$base]");
		
		// change all property names by adding the object "base property" as a prefix.
		$patterns = array(
			'/{(@?(?:=|prop(?:erty)?:))(([^}|:=]+)(([|=]([^}]+))?))}/',
			'/{(@?button:)([^=:}]+)}/',	
			'/{(@?object:)([^=:}]+)}/',	
			'/{(@?object:[^=:}]+):([^=:}]+)}/',	
			'/{submit:([^=:}]+)}/',
			'/{records:([^=:}]+)}/',
			'/{record:([^=:}]+)}/',
			);
			
		$replace = array(
			'{$1'.$base.'.$2}',
			'{$1'.$base.'.$2}',
			'{$1$2:$2}',
			'{$1:'.$base.'.$2}',
			'{submit:'.$base.'.$1}',
			'{records:'.$base.'.$1}',
			'{record:'.$base.'.$1}',
			);
			
		$code = preg_replace($patterns, $replace, $code); 	
			
		// nxLog("Rebased properties to :[$base]------------\n".$code);
		return $code;
	}

	// rebase object path (where to find the object in the template directory tree)
	function rebaseObjectPaths($code,$base)
	{
		nxLog("Rebase objects to :[$base]");
		if ($base=='')
			return $code;
		
		// change all property names by adding the object "base property" as a prefix.
		$patterns = array(
			'/{(@?object:)([^=:}]+)(:([^=:}]+))?}/',	
			);
			
		$replace = array(
			'{$1:'.$base.'.$2$3}',
			);
			
		$code = preg_replace($patterns, $replace, $code); 	
			
		// nxLog("Rebased code to :[$base]------------\n".$code);
		return $code;
	}
	
	// =================== RESOURCES ==============
	function processResources($src)
	{
		global $NX_SKIN_URL;
		$this->reqs = array();	
		$patterns = array(
			'#(src|href|background)\s+=\s+#i',
			'#((?:src|data|href|background|value)=)"[^">]*[/]default[/]res[/]([^"]+)"#i',

			'#((?:src|data|href|background|value)=)"((?:img|image|css)[^"]+)"#ie',
			'#((?:src)=)"((?:js)[^"]+)"#ie',
			'#((?:src|data|href|background|value)=)"(?:[^">]*[/]?res[/])([^"]+)"#ie',
			
			'#<script\s+[^>]*src="(?:\[skin\]|[{]global:NX_SKIN_URL[}])([^{}"]+)".*</script>#ie',
			'#<link\s+[^>]*href="(?:\[skin\]|[{]global:NX_SKIN_URL[}])images[/]([^{}"]+)"[^>]*rel="SHORTCUT ICON"[^>]*>#ie',
			'#<link\s+[^>]*rel="SHORTCUT ICON"[^>]*href="(?:\[skin\]|[{]global:NX_SKIN_URL[}])images[/]([^{}"]+)"[^>]*>#ie',
			'#<link\s+[^>]*href="(?:\[skin\]|[{]global:NX_SKIN_URL[}])([^{}"]+)"[^>]*>#ie',
			'#(<img\s+[^>]*src="(?:\[skin\]|[{]global:NX_SKIN_URL[}])images[/]([^{}"]+)"[^>]*>)#ie',
			'#((?:background|value)="(?:\[skin\]|[{]global:NX_SKIN_URL[}])images/([^{}"]+)"[^>]*>)#ie',
			'#<fav_link#',
			'|(</head>)|ie',
			);
			
		$replace = array(
			'$1=',
			'$1{global:NX_DEFT_SKIN_URL}$2',

			'$this->mapResPath("$1","$2")',
			'$this->mapResPathJs("$1","$2")',
			'$this->mapResPath("$1","$2")',
			
			'$this->requireRes("$1","js","@")',
			'$this->requireLinkRel("$2","$1","$0")',
			'$this->requireLinkRel("$2","$1","$0")',
			'$this->requireRes("$1","css","@")',
			'$this->requireImg("$2","$1")',
			'$this->requireImg("$2","$1")',
			'<link',
			'$this->finishHead("$1")',
			);
		
		//$res=preg_replace($patterns, $replace,$src);	
		//return $res;
		
		return preg_replaceX($patterns, $replace,$src,$this);	
		
		preg_replaceX(
			'#(<img\s+[^>]*src="(?:\[skin\]|[{]global:NX_SKIN_URL[}])images[/]([^{}"]+)"[^>]*>)#ei',
		 	'$this->requireImg("$2","$1")',$src,$this);
	}

	function mapResPath($attr,$path)
	{
		if ($path[0]=='#' || strstr($path,'.htm')!==FALSE|| strstr($path,'.php')!==FALSE)
			return $attr.$path;

		//global $NX_SKIN_URL;
		// $p=$attr.$NX_SKIN_URL.$path;
//		return $p;
		$p="{$attr}".'{global:NX_SKIN_URL}'.$path;
		nxLog("map path: {$attr}{$path} to {$p}","TPL_RES");
		return $p;
	}

	function mapResPathJs($attr,$path)
	{
		if ($path[0]=='#' || strstr($path,'.htm')!==FALSE|| strstr($path,'.php')!==FALSE)
			return $attr.$path;

		//global $NX_SKIN_URL;
		// $p=$attr.$NX_SKIN_URL.$path;
//		return $p;
		$p="{$attr}".'{global:NX_SKIN_URL}'.$path;
		nxLog("map path and deploy: {$attr}{$path} to {$p}","TPL_RES");

		$ext = 'js';
		$n=substr($path,0,strlen($path)-strlen($ext)-1);
		$this->deployRes($res=array('name'=>$n,'type'=>$ext,'ext'=>$ext,'isDevice'=>'false','isSkin'=>'true'));

		return $p;
	}
		
	// add the resource requirement
	function requireRes($name,$ext)
	{
        global $NX_SKIN_URL;
		if (strstr($name,'http')!==FALSE)
			return '';
			
		nxLog("Require Res: $ext / $name ","TPL_RES");
		// in a page, returns a TPL macro
		if ($this->isPage)
        {
			if(defined('TPL_OLD_LINKS')&&TPL_OLD_LINKS)
			{
				// if resource has a device suffix, add device option
				$res = preg_replace('/(?:_IE|_N4|_GK|_IE55)([.][^}]+)}$/','$1 and device}',
					'{@require:'.$name.":skin}");
			}
			else
			{
				$n=substr($name,0,strlen($name)-strlen($ext)-1);
				$this->deployRes($res=array('name'=>$n,'type'=>$ext,'ext'=>$ext,'isDevice'=>'false','isSkin'=>'true'));

				// if resource has a device suffix, add device option
				$res = preg_replace('/(?:_IE|_N4|_GK|_IE55)([.][^}]+)}$/','$1 and device}',
					'{@require:'.$name.":skin}");

				if($ext == 'js')
				{
					$res .= '<script src="'.$NX_SKIN_URL.$name.'"></script>';
				}
				else
				{
					$res .= '<link href="'.$NX_SKIN_URL.$name.'" rel="stylesheet" type="text/css" />';
				}
			}

            return $res;
        }
		
		// else register the requirement in property page.head.ext.req
		$baseName = preg_replace('/([.].+)$/','',$name);
		$noDeviceName = preg_replace('/(?:_IE|_N4|_GK|_IE55)$/','',$baseName);

		if ($noDeviceName==$baseName)
			$isDevice = false;
		else
			$isDevice = true;
		
		$this->appView->requireRes($ext,$noDeviceName,true,$isDevice,'',true);
		
		// nxLog("Require Res - end","TPL_RES");
		return '';
	}
	
	// add the image requirement in the head
	function requireLinkRel($name,$s,$tag)
	{
		nxLog("Require Rel (fav icone: $name [$s]",'TPL_RES');
		
		// get image extension, add resource for parsing/deployment
		$ext = substr(strrchr($url,'.'),1);
		$n=substr($name,0,strlen($name)-strlen($ext)-1);
		// nxLog("resource full name:$name, basename=$n, ext:$ext","requireImg");	
		$this->deployRes($res=array('name'=>"images/{$n}",'type'=>'images','ext'=>$ext,'isDevice'=>'false','isSkin'=>'true'));
		
		return preg_replace(array("#\'#",'#<link#i'),array("'",'<fav_link'),$tag);
	}	
	
	// add the image requirement in the head
	function requireImg($name,$s)
	{
		nxLog("Require Img: $name [$s]",'TPL_RES');
		
		// $res = str_replace("\'","'",$s);
		/*
		if ($this->isPage)
		{
			nxLog("Require IMG - is a page end","TPL_RES");
			$this->reqs[] = '{@require:images/'.$name.":skin}";
			return $res;
		}
		*/

		// get image extension, add resource for parsing/deployment
		$ext = substr(strrchr($name,'.'),1);
		$n=substr($name,0,strlen($name)-strlen($ext)-1);
		// nxLog("resource full name:$name, basename=$n, ext:$ext","requireImg");	
		$res=array('name'=>"images/{$n}",'type'=>'images','ext'=>$ext,'isDevice'=>'false','isSkin'=>'true');
		$this->deployRes($res);
		
		/* return preg_replace(array("/\\'/",'/\[theme\]/'),array("'",'<?php echo $this->getProperty(\'theme.id\');?>'),$s); */
		return str_replace("\'","'",$s);
	}
	
	// complete header with resource requirements
	function finishHead($tag)
	{
		nxLog("finish head $tag - start","TPL_RES");
		if (!$this->isPage)
		{
			nxLog("finish head - end: $tag","TPL_RES");
			return $tag;
		}
			
		global $NX_MIME,$NX_CHARSET;
		$src = '
<meta http-equiv="Content-Type" content="'.$NX_MIME.'; charset='.$NX_CHARSET.'" />
{@css}
{@js}
{@heads}
{css}{js}
';
		
		foreach ($this->reqs as $res)
		{
			$src.= "{$res}\n";
		}
		
		nxLog("finish head - end: ","TPL_RES");
		return $src."\n$tag";
	}

	// =========== html resource link handlers ===========
	
	function mapCSS($s,$dft,$build)
	{
		if ($build=='@')
		{
			return $this->appView->getStyleSheets(trim($s));
		}
		else
			return '<?php echo $this->getStyleSheets("'.trim($s).'"); ?>';
	}

	function mapHeads($s,$dft,$build)
	{
		if ($build=='@')
		{
			return $this->appView->getHeads(trim($s));
		}
		else
			return '<?php echo $this->getHeads("'.trim($s).'"); ?>';
	}

	function mapJS($s,$dft,$build)
	{
		if ($build=='@')
		{
			return $this->appView->getScripts(trim($s));
		}
		else
			return '<?php echo $this->getScripts("'.trim($s).'"); ?>';
	}

	function mapSubmit($name)
	{
		$recProp='$_nx_record_prop';
		$recsProp='$_nx_records_prop';

		if ($name != "")
			return '<?php echo $this->ui->getSubmitButton("'.$name.'",$form["name"]); ?>';
			
		// no name => default to form/record prop if exist or default submitButton property
		return <<< EOH
<?php
		echo \$form["submit"];
		?>
EOH;
	}

	function getButton($name,$build,$urlParams)
	{
		// get/decode url params if any
		$urlParams=trim($urlParams);
		if ($urlParams=='field.url_param')
			$urlParams='$_nx_field->toUrlParam()';
		elseif ($urlParams=="record.url_key")
			$urlParams='$_nx_record->getUrlKey()';
		else
			$urlParams='""';
		
		if ($build=='@')
			return $this->ui->getButton($name,null,$urlParams);
		else
			return '<?php echo $this->getButton("'.$name.'",null,'.$urlParams.'); ?>';	
	}
	
	function mapSpacer($w,$h,$static)
	{
		global $NX_DEFT_SKIN_URL;
		
		if (!$w)
			$w=1;
		else
			$w=$this->expression($w,$static,false);
			
		if (!$h)
			$h=1;
		else
			$h=$this->expression($h,$static,false);
			
		return '<img src="'.$NX_DEFT_SKIN_URL."images/spacer.gif".'" width="'.$w.'" height="'.$h.'">';
	}

}
?>
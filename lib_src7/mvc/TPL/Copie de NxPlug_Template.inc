<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_Template
{
	function __construct(&$props)
	{
		$this->props=&$props;

		// document level property base
		$this->propBase='';
		$this->pathOffset='res/';
		
		// sub object base property management
		$this->objectBase='';
		$this->basePath[]=$this->objectBase;
		
		$this->mode=NX_CHECK_TPL;
		
		$this->fs=$this->props->getPlugIn('FileSys');
	}
		
	function build(&$viewMod,$rootPaths,$outRoot,&$res,&$resources,$mode)
	{
		// compilation mode
		$this->mode=$mode;
		
		$this->appView=&$viewMod;
		
		// 1) search source file
		// store paths for include
		$this->rootPaths=$rootPaths;
		$this->outRoot = $outRoot;
		$this->device = $device = $res['device'];
		$this->resources = &$resources;
		$path = $res['name'];
		
		// set base for property names
		if (isset($res['propBase']))
			$this->propBase=$res['propBase'];

		$ext=$this->ext;
		$d_ext=$this->d_ext;
		$offset=$this->pathOffset;
		$dest = "{$outRoot}{$offset}{$path}{$device}.{$d_ext}";

		$aexts=explode(';',$this->t_ext);
		// $apaths=explode(';',$rootPaths);
		nxLog("building [$path]",'TPL');
		
		$src=null;
		foreach ($rootPaths as $srcRoot)
		{
			foreach ($aexts as $t_ext)
			{
				$f="${srcRoot}{$offset}${path}${device}.{$t_ext}";
				if (is_file($f))
				{
					$src=$f;
					break;
				}
				
				$f="${srcRoot}{$offset}${path}.{$t_ext}";
				if(is_file($f))
				{
					$src=$f;
					break;
				}
			}
			
			// search for file in destination format (ex. php or css)
			if (!$src)
			{
				$f=$this->copyRes($srcRoot,$path,$d_ext,$device,$dest);
				if($f!=null)
					return $f;
			}
			else
				break;

		}
				
		if (!$src)
		{
			nxError("can't find resource: $path.$t_ext","NxPlugTemplate");
			return false;
		}
			
		// 3) if no dest or outdated, recompile and regenerate static resource
		if (!is_file($dest) || $mode==="compile" ||@filemtime($src) < @filemtime($dest))
			return $this->compile($src,$dest,$res,$t_ext);
		else
			return $dest;
	}
		
	// search for a view as tpl file or php script (with or without device)
	// if tpl, compile it, otherwise only copy it.
	function compile($src,$exe,&$res,$t_ext)
	{
		// found a template, compile it
		if ($this->transform($src,$exe,true,$t_ext,$res))
		{
			// now execute the php file to produce the final output.
			// NB. only for static resources such as css or image, other redefine execute() to do nothing...
			if ($this->execute($exe,$res))
				return $exe;
			else
				nxError("can't execute file $exe to $exe","NxPlug_Template");
		}
		else
			nxError("can't transform template file $src to $exe","NxPlug_Template");

		return null;
	}
	
	function execute($exe,&$res)
	{
		global $NX_SKIN_RES;
		
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
			include($exe);
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
	
	// perform the transformation from source file to dest file. Call parse() for the actual
	// processing.
	function transform($src_path,$exe,$checkDir,$t_ext)
	{
		// check if src file exist, dest dir exist and output up to date
		if (!is_readable($src_path))
		{
			nxError("template file does not exist or is not readable:{$src_path}","NxPlug_Template_csx");
			return false;
		}
		
		// check if output directory exist
		if ($checkDir && !is_dir($d=dirname($exe)))
			if ($this->fs->createTree($d)==null)
				return false;
			
		// transform it and write to file
		if ($fp = fopen($exe,'w'))
		{
			// get source
			ob_start();
			readfile($src_path);
			$src=ob_get_contents();
			ob_end_clean();
		
			// process includes
			$src= $this->processIncludes($src,$t_ext,$src_path);
			
			// parse and copy to file
			fputs($fp, $this->compress($this->parse($src),$src_path));
			fclose($fp);
			
			nxLog("compiled template $src_path to $exe","NxPlug_Template",NX_COMPILE_EVENT);
			return true;
		}
		else {
			nxError("cannot create template transformation to file:{$exe}");
			return false;
		}
	}	
	
	function compress($src,$path) {
		if (@NX_COMPRESS_RES===true)
			$src=preg_replace('/(\s)+/',' ',$src);
			
		return $src;
	}
	
	//========= resource deployment =============
	function deployRes(&$res)
	{
		// NB. replace with path... (copy res)
		global $NX_SKIN_ROOT, $NX_DEFT_SKIN_ROOT, $NX_DIS_SKIN_ROOT, $NX_DEFT_SKIN_RES, $NX_SKIN_RES,$NX_SKIN_PATH;
		
		if ($res['isSkin']=="true")
			$skin=$NX_SKIN_RES;
		else
			$skin=$NX_DEFT_SKIN_RES;
			
		if ($res['isDevice']=="true")
			$device=_NX_DEVICE_MAP;
		else
			$device='';
			
		$ext=$res['ext'];
		$name=$res['name'];

		$deployPath = "{$skin}{$name}{$device}.{$ext}";

		nxLog("deploying resource name:$name, ext:$ext, isdevice:{$res['isDevice']} isSkin:{$res['isSkin']} to:$deployPath",'TPL_RES');
		
		// search for views in view repository except if we want to check tpl file anytime
		if ($this->mode!="compile")
		{
			if (is_file($f=$deployPath))
				return $f;
		}
			 
		// file not found => search it in source directories and compile it
		foreach ($NX_SKIN_PATH as $srcRoot)
			// in current skin
			if ($f=$this->copyRes("{$srcRoot}res/",$name,$ext,$device,$deployPath))
				return $f;
		
		// not found
		nxLog("can't find required resource:$deployPath");
		return null;
	}
	
	// search for a view as tpl file or php script (with or without device)
	// if tpl, compile it, otherwise only copy it.
	function copyRes($srcRoot,$name,$ext,$device,$dest)
	{
		// prefix for resources in skin
		// $res="res/";

		$base = $srcRoot.$name;
		nxLog("Trying resource file: ${base}{$device}.{$ext}",'TPL_RES_XX');
		nxLog("Trying resource file: ${base}.{$ext}",'TPL_RES_XX');
		
		// no template =>search for plain files
		if (is_file($f="${base}{$device}.{$ext}") ||
			is_file($f="${base}.{$ext}"))
		{
			// optimistic: assume dir existence
			if ($this->fs->copy($f, $dest))
			{
				nxLog("deployed resource from $f to $dest","NxPlug_Template",NX_COMPILE_EVENT);
				return $dest;
			}
				
			// failed: check directory
			if (!is_dir($d=dirname($dest)))
			{
				// create dir
				if ($this->fs->createTree($d))
				{
					// try again to copy the file
					if ($this->fs->copy($f, $dest))
					{
						nxLog("imported view from $f to $dest","NxPlug_Template",NX_COMPILE_EVENT);
						return $dest;
					}
				}
			}

			// can't copy file => log and run view from source
			nxError("can't copy file:$f to $dest.","NxPlug_Template");
			return $f;
		}
		
		return false;
	}
	
	// base functions for managing variables
	function cleanup($s)
	{
		// cleanup build time locals
		$this->buildlocals=null;

		// remove useless php tags
		$matches=array(
			'/<\?=\s*([^\s?]+)\s*\?>/',
			'/\?>(\n*)<\?(php)?([^=])/',
			/*
			//'/[ \t]+/',
			//'/[\n]+/',
			*/
			);
		
		$replace=array(
			'<?php echo \\1 ; ?>',
			'\\1\\3',
			//' ',
			//"\n"
			);
		
		return preg_replace($matches, 
					$replace,
					$s);			
	}
	
	
	// ==================== includes ========================
	// preprocess includes: read files and insert them in source
	function processIncludes(&$src,$t_ext)
	{
		$patterns = array(
			'/{--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{!--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{@?include:([^=:}]+)(:([^:{}=]+))?}/e',
			);
			
		$replace = array(
			'',
			'<!--\\1-->',
			'$this->mapInclude("$1","$2")',
			);
			
		return preg_replace($patterns, $replace, $src); 	
	}
		
	function mapInclude($path,$opts)
	{
		$path=str_replace('.','/',$path);

		// $isSkin=(stristr($opts,"skin")!=''?"true":"false");
		$isDevice=(stristr($opts,"device")!=''?"true":"false");
		if ($isDevice=="true")
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
					return $this->processIncludes($this->fs->file_get_contents($f),$t_ext);
				}
			}
			
			// no template => search raw format files (not templated)
			$ext=$this->ext;
			if (is_file($f="{$basePath}{$device}.{$ext}") || 
				is_file($f="{$basePath}.{$ext}"))
			{
				// return $this->file_get_contents($f);
				nxLog("include PHP file:$f",'TPL');
				return $this->fs->file_get_contents($f);
			}
		}
		
		nxError("failure 1 in template inclusion : $path",'NxPlug_Template');
		return "{-- failed include: $path --}";
	}
	
	function mapMenu($menuID,$displayID,$dataSource,$opts)
	{
		// manage default parameters
		if (!$menuID)
			$menuID = "default";
			
		if (!$displayID)
			$displayID = "enveloppe.menu";
			
		if (!$dataSource)
			$dataSource = "links";
			
		nxLog("menu: id=$menuID, display=$displayID, ds=$dataSource",'TPL');
		
		// add link load code
		$load = '<?php $this->getRecords("'.$menuID.'","page.links","'.$dataSource.'");?>';
		
		// include menu
		return $load . $this->mapInclude($displayID,"device");
	}

	function mapContent($docID,$displayID,$dataSource,$opts)
	{
		// manage default parameters
		if (!$docID)
			$docID = "default";
			
		if (!$displayID)
			$displayID = "page.page";
			
		if (!$dataSource)
			$dataSource = "docs";
			
		// add load code
		$load = '$docRecords=$this->getRecords("'.$docID.'","page.doc","'.$dataSource.'");
		if (!$docRecords)
		{
			nxError("can\'t load document:[$path]");
			$docRecords = $this->getRecords(\'default\',\'page.doc\',\'Doc\');
		}
		
		// adds properties to current set (NB. assumes only one record...)
		$this->addProperties($docRecords->getProperties());
		';
		
		// include menu
		return '<?php '.$load . $this->mapInclude($displayID,"device").'?>';
	}
	
	/**
	 * @return code
	 * @param s property name
	 * @param dft default value
	 * @param baseProp property base name (prefix)
	 * @param static is executed at build time or runtime
	 * @param inPHP results to copy inside PHP section or html
	 * @desc remap a property from TPL to php
	 */
	function mapProp($s,$isDeft,$dft,$baseProp,$static,$inPHP)
	{
		nxLog("map property:$s on base:[$baseProp],sep=[$isDeft],".($static?"static":"runtime"),"TPL:mapProp");
		//$dft = addslashes($dft);
		$prop = $baseProp.trim($s);
		
		if ($static)
		{
			if ($isDeft=='=')
			{
				$v=$this->expression($dft,$static,true);
				$this->props->setProperty($prop,$v);
				nxLog("setting up property:[$prop]=[$v]","TPL:mapProp");
				return '';
			}
			elseif ($isDeft=='|')
			{
				$v=$this->expression($dft,$static,true);
				$res=$this->props->getProperty($prop,$v);
			}
			else
				$res=$this->props->getProperty($prop,'');
		}
		else
		{
			if ($isDeft=='=')
				$op='set';
			else
				$op='get';
			
			// remove " delimiters
			$dft=rtrim(ltrim($dft,'"'),'"');
			
			$res='$this->'.$op.'Property("'.$prop.'","'.$dft.'")';
			if ($inPHP)
				return $res;
			else
				return '<?php echo'.$res.';?>';
		}

		nxLog("map property:$s on base:$baseProp,".($static?"static":"runtime")." res=$res"	,"TPL:mapProp");
			
		return $res;
	}

	function mapQProp($media,$name,$dft,$static,$inPHP)
	{
		// get superglobal related to media (get|post|request|cookie|session)
		$media = strtoupper(trim($media));

		// build time
		if ($static)
		{
			$av =& $GLOBALS["_$media"];
			if (isset($av[$name]))
				return $av[$name];
			else
				return $dft;
		}
		
		// runtime
		$vN = '$_'.$media.'["'.$name.'"]';
		$res = '(isset('.$vN.')?('.$vN.'):"'.$dft.'")';
		if (!$inPHP)
			return '<?php echo '.$res.';?>';
		else
			return $res;
	}

	function mapSystemProp($selector,$prop,$dft,$staticChar,$inPHP)
	{
		$dft = addslashes($dft);
		$prop = trim($prop);
		
		$varN='_NX_'.$selector;
		if ($staticChar=='@')
		{
			$res=$GLOBALS[$varN]->props->getProperty($prop,$dft);
		}
		else
		{
			$res='$GLOBALS[\''.$varN.'\']->getProperty("'.$prop.'","'.$dft.'")';
			if ($inPHP)
				return $res;
			else
				return '<?php echo '.$res.';?>';
		}

		nxLog("map property:{$varN}::{$prop},".($staticChar=="@"?"static":"runtime")." res=$res"	,"TPL:mapProc");
			
		return $res;
	}

	function mapK($s,$isDeft,$dft,$static,$inPHP)
	{
		$n= trim($s);
		
		// static
		if ($static)
			if (defined($n))
				return constant($n); 
			else				
				return $this->expression($dft,$static,true);

		// runtime
		if ($isDeft!='')
		{
			$v=$this->expression($dft,'',true);
			$res="(defined('{$n}')?{$n}:{$v})";
			// $res = '<?php ((defined("'.$n.'"))?'.$n.':"'.str_replace('"','\"',$dft).'")';
		}
		else
		{
			$res = "constant('{$n}')";
		}

		if ($inPHP)
			return $res;
		else
			return '<?php echo '.$res.';?>';

	}

	function mapGlobal($s,$isDeft,$dft,$static,$inPHP)
	{
		$s=trim($s);
		
		// static
		if ($static)
		{
			if ($isDeft==='=')
				$GLOBALS[$s]=$this->expression($dft,$static,true);
				
			if(isset($GLOBALS[$s]))
				return $GLOBALS[$s];
			
			return $this->expression($dft,$static,true);
		}
		
		// runtime

		// set var (never as part of an expression)
		$v=$this->expression($dft,'',true);
		if ($isDeft==='=')
			return '<?php '."{$n}={$v};".' ?>';
		
		$n= '$GLOBALS["'.$s.'"]';
		if ($isDeft==='|')
			$res="(isset({$n})?{$n}:{$v})";
		else
			$res=$n;
			
		// in php section/expression ?
		if ($inPHP)
			return $res;
		else
			return '<?php echo '.$res.';?>';
	}

	function mapLocal($s,$isDeft,$dft,$static,$inPHP)
	{
		nxLog("mapping local:[$s|$isDeft,$dft] $static","TPL");
		$s=trim($s);
		if ($static)
		{
			if ($isDeft=='=')
			{
				$v=$this->expression($dft,$static,true);
				//$v=$this->expression(trim($dft,'"\''),$static,true);
				$this->buildlocals[$s]=$v;
				nxLog("setting up variable:[$s]=[$v]","TPL");
				return '';
			}
				
			if(isset($this->buildlocals[$s]))
			{
				nxLog("getting variable:[$s] :".$this->buildlocals[$s]);
				return $this->buildlocals[$s];
			}

			nxLog("undefined variable[$s] => deft [$dft]");
			
			return $this->expression($dft,$static,true);
		}
		else
		{
			$v=$this->expression($dft,'',true); 
			$n= '$'.$s;
			
			if ($isDeft==='=')
				return '<?php '."{$n}={$v};".' ?>';
				
			if ($isDeft==='|')
				$res="(isset({$n})?{$n}:{$v})";
				// $res= '((isset('.$n.'))?'.$n.': "'.str_replace('"','\"',$v).'")';
			else
				$res = $n;
				
			if (!$inPHP)
				return '<?php echo '.$res.';?>';
			else
				return $res;
		}
	}

	function expression($e,$static,$inPHP=false)
	{
		if ($e==="")
			return '';
			
		nxLog('found expression: ['.$e.']', "TPL");
		$res = $this->mapExprData($this->mapRecordExpr($this->mapFieldMeth(str_replace("\'","'",trim($e)))),$static);
		
		nxLog("mapped expression to evaluate: [$res]","TPL");
		
		if ($static!='')
		{
			if ($res!='')
			{
				$ev='$res = '.$res.';';
				//nxLog("evaluating expression: [$ev]","TPL");
				eval($ev);
			}
			else
				$res='';
		}

		nxLog("result of expression is: [$res]","TPL");
		
		if ($inPHP)
			return $res;
		else
			return '<?php echo '.$res.';?>';
	}

	function mapReq($name,$ext,$opts,$prop,$build)
	{
		nxLog("map Require: [name:$name][ext:$ext]","TPL");
		
		$isSkin=(stristr($opts,"skin")!=''?"true":"false");
		$isDevice=(stristr($opts,"device")!=''?"true":"false");
		$ext=strtolower(trim($ext));
		$name=trim($name);

		// register this resource for compilation/deployment
		$this->addDeployRes(array("name"=>$name,"type"=>$ext,"ext"=>$ext,"isDevice"=>$isDevice,"isSkin"=>$isSkin));
		
		if ($build=='@')
		{
			$this->appView->requireRes($ext,$name,$isSkin,$isDevice,trim($prop),true);
			return '';
		}
		else
			return '<?php $this->requireRes("'.$ext.'","'.$name.'",'.$isSkin.','.$isDevice.',"'.trim($prop).'"); ?>';
	}
	
	function addDeployRes($res)
	{
		nxLog('resource required: name:'.$res['name'].', ext:'.$res['ext'].', isDevice:'.$res['isDevice'].', isSkin:'.$res['isSkin'],"TPL Template/mapReq");
		$this->resources[] = $res;
	}
	
	function mapPHP($s)
	{
		return '<?php '.$s.'?>';
	}
	
	
	// ================== RECORDS/FIELDS macros ==========================
	function mapRecordsH($prop)
	{
		$this->inRecords=true;
		if (($prop=trim($prop))=='')
			$prop='records';
		$records = '$_nxvar_'.str_replace ('.','_',$prop);
		
		return '<?php
// <records>
		$_nxvar_records = $this->getProperty($_nx_records_prop="'.$prop.'",null,true);
		$_nx_recIndex=0;
		if ($_nxvar_records===null)
			nxError("No records in property ['.$prop.']","VIEW");
		elseif($_nxvar_records->count() > 0)
		{
			// pages variables
			if ($_nxvar_records->isPaged() > 0)
			{
				$_nxvar_context =& $_nxvar_records->getPageContext();
				$_nxvar_page_index=$_nxvar_context["currentPageOffset"];
				$_nxvar_paged=true;
				$_nxvar_nbpages=$_nxvar_context["nbPages"];
				$_nxvar_page_alias= $_nxvar_context["pageAlias"];
				
				// remove page parameter
				$_nxvar_page_url_= str_replace(array(\'&&\',\'&\'),array(\'&\',\'&amp;\'),preg_replace("/${_nxvar_page_alias}=[^&]+&?/",\'\',$GLOBALS["NX_CURRENT_URL"]).(strpos($GLOBALS["NX_CURRENT_URL"],\'?\')===FALSE?\'?\':\'&\')."$_nxvar_page_alias=");
			}
			else {
				$_nxvar_nbpages=1;
			}

// </records>
		?>';
	}
	
	function mapRecordsF()
	{
		return '<?php
		} //if
		?>';
	}
	
	function mapRecordH($s,$opts='')
	{		
		$code = '<?php
		// <record>
		$fmethod = $this->getProperty("fmethod","toHTML",true);
		if ($fmethod=="toForm")
			$_nx_isViewForm=true;
		else
			$_nx_isViewForm=false;
			
		$_nx_recIndex=0;
		';

		if (!isset($this->inRecords) && ($s=trim($s)) == '')
		{
			$s = 'page.record';
		}

		$record='$_nx_record';
		if ($s != '')
		{
			$code .= 
			"{$record}=\$this->getProperty(\$_nx_record_prop='{$s}',null,true);".'
			if ($_nx_record==null)
				nxError("No record in property ['.$s.']","VIEW");
			else
			{
			$_nx_records_prop="";
			$_nx_fieldCount=$_nx_record->fieldCount();
			$_nx_record_key=$_nx_record->getUrlKey(); // record.url_key
			';
		}
		else
		{
			$code .='
		for ($_nx_iterRec = $_nxvar_records->getRecordIterator(); $_nx_iterRec->isValid(); $_nx_iterRec->next())
		{
			$_nx_record =& $_nx_iterRec->getCurrent();
			$_nx_fieldCount=$_nx_record->fieldCount();
			$_nx_record_key=$_nx_record->getUrlKey(); // record.url_key
			$_nx_record_prop="";
			';
		}
		
		return $code.'
			if ($_nx_isViewForm)
			{
				$form = $this->getForm($_nx_record, $_nx_record_prop, $_nx_records_prop);'.
(($opts===''|| ($opts!='no_header' && $opts=='true'))?'
				echo $form["header"];':'').'
			}
			else
				$form = null;
		// </record>
		?>';
	}

	function mapRecordF()
	{
		return '<?php 
	// <record-end>
		
	// form footer
		if ($form)
			echo $form["hiddenFields"].\'</form>\';
		
		$_nx_recIndex++;			
		} // for
		
	// </record-end>	
		?>';
	}

	
	function mapFormH($s,$opts='')
	{
		$code = '<?php
		// <form>
		$fmethod = "toForm";
		$_nx_isViewForm=true;			
		$_nx_recIndex=0;
		';

		if (!isset($this->inRecords) && ($s=trim($s)) == '')
		{
			$s = 'page.record';
		}

		$record='$_nx_record';
		if ($s != '')
		{
			$code .= 
			"{$record}=\$this->getProperty(\$_nx_record_prop='{$s}',null,true);".'
			if ($_nx_record==null)
				nxError("No record in property ['.$s.']","VIEW");
			else
			{
			$_nx_records_prop="";
			$_nx_fieldCount=$_nx_record->fieldCount();
			$_nx_record_key=$_nx_record->getUrlKey(); // record.url_key
			';
		}
		else
		{
			$code .='
		for ($_nx_iterRec = $_nxvar_records->getRecordIterator(); $_nx_iterRec->isValid(); $_nx_iterRec->next())
		{
			$_nx_record =& $_nx_iterRec->getCurrent();
			$_nx_fieldCount=$_nx_record->fieldCount();
			$_nx_record_key=$_nx_record->getUrlKey(); // record.url_key
			$_nx_record_prop="";
			';
		}
		
		return $code.'
			$form = $this->getForm($_nx_record, $_nx_record_prop, $_nx_records_prop);'.
(($opts===''|| ($opts!='no_header' && $opts=='true'))?'
				echo $form["header"];':'').'
		// </form>
		?>';
	}	
	
	function mapFormF()
	{
		return '<?php 
	// <record-end>
		
	// form footer
		if ($form)
			echo $form["hiddenFields"].\'</form>\';
		
		$_nx_recIndex++;			
		} // for
		
	// </record-end>	
		?>';
	}	
	
	function mapFieldsH($opts)
	{
		$s='<?php
//<fields>
		$_nx_iterField = $_nx_record->getFieldIterator();
		$_nx_fieldIndex=0;
';
		// compute all fields count in case we want all of them
		if (strtolower(trim($opts))=='all')
			$s .= '$_nx_fieldCount=$_nx_record->fieldCount(true);';
		
		$s.='
		for ($_nx_iterField->reset(); $_nx_iterField->isValid(); $_nx_iterField->next())
		{
			$_nx_field = $_nx_iterField->getCurrent();
';
		
		if (strtolower(trim($opts))!='all')
			$s .= 'if (!$_nx_field->isHidden())';
		
		return $s.'
			{
//</fields>
		?>';
	}

	function mapFieldsF()
	{
		return '<?php
//<fields-end>
				 
			$_nx_fieldIndex++; //index of non hidden fields
			} /* if */
		} /* iter field */
//</fields-end>
		?>';		
	}	
	
	function mapGetFieldH($key,$bykey,$attVal)
	{
		$key=trim($key);
		$bykey=trim($bykey);
		$attVal = trim($attVal);
		
		// get field by name
		if ($bykey=='' || $bykey=='name')
		{
		return '<?php
//<get_field>
		$_nx_field = $_nx_record->getField("'.$key.'");
		if(isset($_nx_field)) 
		{
//</get_field>
		?>';
		}

		// get field by property
		if ($bykey=='property' || $bykey=='alias')
		{
		return '<?php
//<get_field>
		$_nx_field = $_nx_record->getFieldBy("'.$key.'","alias"); 
		if(isset($_nx_field)) 
		{
//</get_field>
		?>';
		}

		// other kind of attribute
		return '<?php
//<get_field>
		$_nx_field = $_nx_record->getFieldByAttribute("'.$bykey.'","'.$key.'","'.$attVal.'");
		if(isset($_nx_field)) 
		{
//</get_field>
		?>';
		
	}

	function mapGetFieldF()
	{
		// other kind of attribute
		return '<?php
		} //<get_field_end>
		?>';
	}
	
	function mapRecordAttr($s,$prop,$dft)
	{
		if ($s=='url_key')
			return '<?php echo $_nx_record->getUrlKey(); ?>';
		else if ($s=='property')
			return '<?php echo $_nx_record->getProperty("'.$prop.'","'.addslashes($dft).'"); ?>';
		else if ($s=='index')
			return '<?php echo $_nx_recIndex; ?>';
		else 
			return '<?php echo '.$this->mapRecordExpr("record.{$s}").'; ?>';
	}
		
	function mapFormAttr($s,$prop,$dft)
	{
		if ($s=='url_key')
			return '<?php echo $_nx_record->getUrlKey(); ?>';
		else if ($s=='property')
			return '<?php echo $_nx_record->getProperty("'.$prop.'","'.addslashes($dft).'"); ?>';
		else if ($s=='index')
			return '<?php echo $_nx_recIndex; ?>';
		else 
			return '<?php echo '.$this->mapRecordExpr("record.{$s}").'; ?>';
	}

	function mapField($m,$n,$opts,$inPHP=false)
	{
		nxLog("map field expression: {field.$m:$n|$opts)",'compile');

			// get field variable
		if ($n)
			$fvar='$_nx_field_tmp';
		else
			$fvar='$_nx_field';
			
		$opts=trim($opts,'"');
		
		// get method name
		if ($m!='')
			$fcall=$this->mapFieldMeth('field.'.$m,$fvar,$opts);
		else
			$fcall=$fvar.'->$fmethod('."'{$opts}')";
			
		// get field 
		if ($n)
			$exist="{$fvar}=\$_nx_record->getField('{$n}')";
		else
			$exist='$_nx_field';
			
		// get field value
		$code="(($exist)?($fcall):''/*unknown field*/)";
		
		// in PHP ?
		if ($inPHP==true || $inPHP=="@")
			return $code;
		else
			return '<?php echo '.$code.';?>';
	}

	// ========= conditional macro/expression ===========
	function mapIf($c,$do,$else)
	{
		//nxLog('if condition:: '.$c, "TPL");
		
		$cond=trim($this->mapRecordExpr($this->mapFieldMeth($this->mapExprData(str_replace("\'","'",trim($c)),''))),'#');
		
		$code= <<<EOH
<?php
		if ($cond)
EOH;
		if (trim($do)=='')
			return $code.'{?>';
		else
			$code .=' echo "'.str_replace(array('"',"\'"),array('\"',"'"),$do).'";';

		if (trim($else)=='')
			return $code.'		?>';
		else
			return $code.'else	echo "'.str_replace(array('"',"\'"),array('\"',"'"),$else).'";?>';
	}

	// private
	function & mapFieldMeth($s,$f='$_nx_field',$opts='')
	{
		if ($s!='')
		{
			$findex='$_nx_fieldIndex';
			$patterns = array(
				'/field[.]is_first/i',
				'/field[.]is_last/i',
				'/field[.]even_odd/i',
				'/field[.]is_even_index/i',
				'/field[.]is_odd_index/i',
				'/field[.]index/i',
				'/field[.]error/i',
				'/field[.]ok/i',
				'/field[.]name/i',
				'/field[.]label/i',
				'/field[.]required/i',
				'/field[.]hidden/i',
				'/field[.]format/i',
				'/field[.]html/i',
				'/field[.]status/i',
				'/field[.]js/i',
				'/field[.]list/i',
				'/field[.]default/i',
				'/field[.]string/i',
				'/field[.]db/i',
				'/field[.]object/i',
				'/field[.]url_param/i',
				'/field[.]alias/i',
				'/field[.]form/i',
				'/field[.]prompt/i',
				'/field[.]is_help/i',
				'/field[.]class/i',
				'/field[.]property/i',
				'/field[.]is_pseudo/i',
				'/field[.]type/i',
				'/field[.]help/i',
				'/field[.]has_attribute/i', // ex. field.has_attribute("is_link")
				'/field[.]([a-z0-9_]+)$/i', // ex. field.has_attribute("is_link")
				'/\$_FLD_/','/\$_OPTS/'
				);
				
			$replace = array(
				'($_nx_fieldIndex===0)',
				'($_nx_fieldIndex+1==$_nx_fieldCount)',
				'($_nx_fieldIndex%2===0?"even":"odd")',
				'($_nx_fieldIndex%2===0)',
				'($_nx_fieldIndex%2===1)',
				'$_nx_fieldIndex',
				'$_FLD_->error()',
				'$_FLD_->isOk()',
				'$_FLD_->desc->name',
				'$_FLD_->desc->getLabel()',
				'$_FLD_->desc->isRequired()',
				'$_FLD_->desc->isHidden()',
				'$_FLD_->toFormat($_OPTS)',
				'$_FLD_->toHTML($_OPTS)',
				'$_FLD_->toStatus()',
				'$_FLD_->toJS($_OPTS)',
				'$_FLD_->toList($_OPTS)',
				'$_FLD_->getDefaultValue($_OPTS)',
				'$_FLD_->toString($_OPTS)',
				'$_FLD_->toDB()',
				'$_FLD_->toObject()',
				'$_FLD_->toUrlParam()',
				'$_FLD_->getAlias()',
				'$_FLD_->toForm()',
				'$_FLD_->getPrompt()',
				'$_FLD_->desc->getProperty("help",null,false)!=null',
				'$_FLD_->desc->getProperty("class",$_OPTS)',
				'$_FLD_->desc->getProperty($_OPTS)',
				'$_FLD_->is_pseudo()',
				'$_FLD_->desc->getType()',
				'$this->getString($_FLD_->getProperty("help","",false))',
				'null!=$_FLD_->getProperty',
				'$_FLD_->desc->getProperty("$1",$_OPTS)',
				$f,"'$opts'"
				);
			$expr=preg_replace($patterns,$replace,$s);
			nxLog("map field method: [$s] to [$expr]",'compile');

		}
		else
		{
			nxLog("map field method: [default]",'compile');
			$expr='$_nx_field->$fmethod('."'$opts'".')';
		}
		return $expr;
	}

	function mapRecordExpr($s)
	{
		$patterns = array(
			'is_form',
			'is_help',
			'form.is_help',
			'form.is_required',
			' and ',
			' or ',
			'not ',
			'record.url_key',
			'record.index',
			'record.id',
			'record.is_first',
			'record.is_last',
			'record.even_odd',
			'record.is_even_index',
			'record.is_odd_index',
			'record.nb_fields',
			'records.count',
			
			// paged list of records
			'pages.limit',
			'pages.recordsPerPage',
			
			'pages.several_pages',
			'pages.nb_records',
			'pages.first_record',
			'pages.last_record',
			'pages.nb_pages',
			'page.is_first',
			'page.is_last',
			'page.is_current',
			'page.is_previous',
			'page.is_next',
			'page.index',
			'page.url',	
			'pages.current',
			'page.current'	
			);
			
		$replace = array(
			'isset($form)',
			'isset($form["isHelp"]) && $form["isHelp"]',
			'isset($form["isHelp"]) && $form["isHelp"]',
			'isset($form["isRequired"]) && $form["isRequired"]',
			'&&',
			'||',
			' !',
			'$_nx_record_key',
			'$_nx_recIndex',
			'$_nx_record->recordPrefix',
			'($_nx_recIndex==0)',
			'($_nx_recIndex+1==$_nxvar_records->count())',
			'($_nx_recIndex%2===0?"even":"odd")',
			'($_nx_recIndex%2===0)',
			'($_nx_recIndex%2===1)',
			'$_nx_fieldCount',
			'$_nxvar_records->count()',
			
			
			'$_nxvar_context["recordsPerPage"]',
			'$_nxvar_context["recordsPerPage"]',
			'($_nxvar_nbpages>1)',
			'$_nxvar_context["totalRecords"]',
			'$_nxvar_context["firstRecord"]',
			'$_nxvar_context["lastRecord"]',
			'$_nxvar_nbpages',
			'($_nxvar_page==0)',
			'($_nxvar_page==$_nxvar_nbpages-1)',
			'($_nxvar_page_index==$_nxvar_page)',
			'(($_nxvar_page_index-1)==$_nxvar_page)',
			'(($_nxvar_page_index+1)==$_nxvar_page)',
			'($_nxvar_page+1)',
			'($_nxvar_page_url)',
			'($_nxvar_page_index)',
			'($_nxvar_page_index)'
			);
		$expr=str_replace($patterns,$replace,$s);
		
		nxLog("map record method: [$s] to [$expr]",'compile');
		return $expr;
	}
	
	// @desc: map an expression data to their values
	// 2syntaxs: #selector:name:option# or %selector:name:option%
	function mapExprData($e,$static)
	{
		//nxLog("expression data:[$e]","TPL");
		$patterns = array(
			'/%(([^"#:]+):([^#]*))%/', // alternative syntax: %field:field%
			'/#(@?)expr:([^#]*)#/e',
			// record data
			'/#field(?:[.]([^=:#]+))?(?:[:]([^=:|#]*)(?:[:|=]([^=:}]+))?)?#/e',

			// http
			'/#(get|post|request|cookie|param|session):([^#|:]+)([|:]([^#]+))?#/e',
			'/#@(get|post|request|cookie|param|session):([^#|:]+)([|:]([^#]+))?#/e',
		
			// remap access to variables to simplify other regular expression
			'/#@(?:@|const(?:ant)?:)(?=[^#]+#)/',
			'/#@(?:=|prop(?:erty)?:)(?=[^#]+#)/',
			'/#@(?:\$|var(?:iable)?:)(?=[^#]+#)/',
			'/#@(?:\$\$|global:)(?=[^#]+#)/',
			'/#(?:@|const(?:ant)?:)(?=[^#]+#)/',
			
			'/#(=|prop(erty)?:)(?=[^#]+#)/',
			'/#(?:\$|var(?:iable)?:)(?=[^#]+#)/',
			'/#(?:\$\$|global:)(?=[^#]+#)/',
			
			// run time variables
			'/#P:([^#|:=]+)([:]([^#|=]+))?(([|=])([^#]*))?#/e',
			'/#K:([^#|:=]+)(([|])([^#]*))?#/e',
			'/#G:([^#|:=]+)(([:=|])([^#]*))?#/e',
			'/#V:([^#|:=]+)(([:=|])([^#]*))?#/e',
			
			// build time variables
			'/#(_P:)([^#|:=]+)([:]([^#|=]+))?(([|=])([^#]*))?#/e',
			'/#(_K:)([^#|:=]+)(([|])([^#]*))?#/e',
			'/#(_G:)([^#|:=]+)(([:=|])([^#]*))?#/e',
			'/#_V:([^#|:=]+)(([|])([^#]*))?#/e',
			
			// system properties #@user:user.companyName|not loggged#
			'/#(@)?(user|page|site):([^#|:=]+)(?:[:]([^#|=]+))?(?:[|=]([^#]+))?#/e',
			);
			
		$replace = array(
			'#$1#',
			'$this->expression("$2","$1",false)',
			// record data
			'$this->mapField("$1","$2","$3",true)',			
			
			'$this->mapQProp("$2","$4","'.$static.'",true)',
			'$this->mapQProp("$2","$4","",true)',
			
			'#_K:',
			'#_P:',
			'#_V:',
			'#_G:',
			'#K:',
			'#P:',
			'#V:',
			'#G:',
			
			// runtime variables
			'$this->mapProp("$1","$5","$6","$3","'.$static.'",true)',
			'$this->mapK("$1","$3","$4","'.$static.'",true)',
			'$this->mapGlobal("$1","$3","$4","'.$static.'",true)',
			'$this->mapLocal("$1","$3","$4","'.$static.'",true)',
			
			// build-time variables
			'$this->mapProp("$2","$6","$7","$4",true,true)',
			'$this->mapK("$2","$4","$5",true,true)',
			'$this->mapGlobal("$2","$4","$5",true,true)',
			'$this->mapLocal("$1","$3","$4",true,true)',	

			'$this->mapSystemProp("$2","$3","$4","$1",true)',				
			);
			
		$res = preg_replaceX($patterns, $replace,$e,$this);
		//nxLog("result of expression data:[$res]");
		return $res;
	}	
	
	function mapString($str,$pack,$static,$inPHP)
	{
		if ($static=="@")
		{
			return $this->props->getString($str,null,$pack);
		}
		else
		{
			$res='$this->getString("'.$str.'",null,"'.$pack.'")';
			if ($inPHP)
				return $res;
			else
				return '<?php echo '.$res.';?>';
		}
	}
	
	// ================== PARSER ===================
	
	// parse a string in tpl format and returns a php view
	function & parse(&$str,$ext="tpl")
	{
		$this->buildlocals=array();
		$patterns = array(
			// escape sequence /% {=prop} %/ used to trick Dreamweaver in url rewriting
			// otherwise DM try to remap the url {=prop} to something else...
			'@/%|%/@',  

			// NB. keep this sequence: heads,require,css,js
			
			// adds up headers from other page components
			'/{(@)?heads([:=]([^=}|]+)([|]([^}]+))?)?}/e',
			
			// parse resource requirements
			'/{(@)?require:([^=:.}]+)[.]([^ :}{=.]+)(:([^:{}=]*)(:([^{};=]+))?)?}/e',
			
			// get list of resources as html link/script tags
			'/{(@)?css([:=]([^=}|]+)([|]([^}]+))?)?}/e',
			'/{(@)?js([:=]([^=}|]+)([|]([^}]+))?)?}/e',

			'/[%](([a-zA-Z]+):([^#]*))[%]/', // alternative syntax: %field:field%
			
			// remap access to variables to simplify other regular expression
			'/{@(@|const(ant)?:)/',
			'/{@(=|property:)/',
			'/{@(\$\$|global:)/',
			'/{@(\$|var(iable)?:)/',
			
			'/{(const(ant)?:)/',
			'/{(=|prop(erty)?:)/',
			'/{(\$\$|global:)/',
			'/{(\$|var(iable)?:)/',
			
			"'<script[^>]*?>.*?</script>'si",  
			'/<?php([^?]+)\?\>/si',
			'/{(get|post|request|cookie|session):([^}|:]+)([|:]([^}]+))?}/e',

			// run time variables
			'/{(P:)([^}|:=]+)([:]([^}|=]+))?(([|=])([^}]*))?}/e',
			'/{(K:)([^}{|:=]+)(([|])([^}{]*))?}/e',
			'/{(G:)([^}|:=]+)(([:=|])([^}{]*))?}/e',
			'/{(V:)([^}|:=]+)(([:=|])([^{}]*))?}/e',
			
			// build time variables
			'/{(_P:)([^}|:=]+)([:]([^}|=]+))?(([|=])([^}]*))?}/e',
			'/{(_K:)([^}{|:=]+)(([|])([^}{]*))?}/e',
			'/{(_G:)([^}|:=]+)(([:=|])([^}{]*))?}/e',
			'/{_V:([^}|:=]+)(([=|])([^{}]*))?}/e',
			
			// system properties {@user:user.companyName|not loggged}
			'/{(@)?(user|page|site):([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?}/e',

			// expression: syntax include any valid operator, numbers, and
			// + data values: #(prop|var|const|global):name# ,
			// + field values: #field[.](is_first|index|name|label)# 	(see mapFieldMeth() for others),
			// + #record[.](is_first|etc...)# 	see mapRecordExpr() for others
			'/{(@)?expr(?:ession)?:([^}]+)}/e',
			
			'/{records(:([^=:}]+))?}/e',
			'|{/records}|e',
			'|{records.count}|',
			'|{no_record}|',
			'|{/no_record}|',
			'/{record(:([^=:}]+)(:([^=:}]+))?)?}/e', // record iterator block
			'|{/record}|e', // record iterator block end
			'/{record(?:[.](url_key|even_odd|property|index|name|id)(?:[:]([^=:|}]+))?(?:[|]([^}]*))?)?}/e', // record attribute, ex. record.url_key
			'/{form(:([^=:}]+)(:([^=:}]+))?)?}/e', // record iterator block
			'|{/form}|e', // record iterator block end
			'/{form(?:[.](url_key|even_odd|property|index|name|id)(?:[:]([^=:|}]+))?(?:[|]([^}]*))?)?}/e', // record attribute, ex. record.url_key

			'/{fields(:([^=:}]+))?}/e',
			'|{/fields}|e',
			'/{get_field:([^=:}]+)(?:[:]([^=:}]+))?(?:[:]([^=:}]+))?}/e',
			'|{/get_field}|e',
//			'/{field(?:[.]([^=|:}]+))?(?:[:]([^=:|}]*)?(?:[:|=]([^:=}]+))?)}/e',
			'/{(@?)field(?:[.]([^=:}|]+))?(?:(?:[:]([^=:|}]*))?(?:[:|=]([^:=}]+))?)?}/e',

			// html utilities
			'/{(@)?spacer[:]([^,:}]+)([:,]([^,:}]+))?}/e',
			
			// paged records
			'/{page[s]?:previous}/',
			'/{page[s]?:next}/',
			'/{pages(:all)?}/',
			'|{/pages}|',
			'/{page[.:]url}/',
			'/{page[.:]index}/',
			'/{page[s]?[.:]recordsPerPage}/',
			'/{page[s]?[.:]limit}/',
			'/{page[s]?[.:]first_record}/',
			'/{page[s]?[.:]last_record}/',
			'/{page[s]?[.:]nb_records}/',
			'/{page[s]?[.:]nb_pages}/',
			'/{page[s]?[.:]current}/',
			'/{no_page}/',
			'|{/no_page}|',

//			'/{if[:]\s*([^}]+)}/e',
			'/{if[:]\s*([^}?]+)(?:\?([^:}]+):([^}]+))?}/e',
			'/{elseif[:]\s*([^}]+)}/e',
			'|{else}|',
			'|{/if}|',
			'/{(@)?button:([^=:}]+)(?:[:]([^}:]+))?}/e',			
			'/{submit(:([^=:}]+))?}/e',
			'/{(@)?(?:string:|_)[%"\']([^\'"]*)["\'%](:"?([^":]+)"?)?}/e',
			'/{--(.*)--}/',
			'/{!--(.*)--}/',
//			'/{!--(.|-(?!-})*)--}/s',
			);
			
		$replace = array(
			'',
			'$this->mapHeads("$3","$5","$1")',
			'$this->mapReq("$2","$3","$5","$7","$1")',
			'$this->mapCSS("$3","$5","$1")',
			'$this->mapJS("$3","$5","$1")',

			'#$1#',
			'{_K:',
			'{_P:',
			'{_G:',
			'{_V:',
			'{K:',
			'{P:',
			'{G:',
			'{V:',
			
			'$0',
			'$0', // '$this->mapPHP("$1")',
			'$this->mapQProp("$1","$2","$4",false,false)',

			// runtime variables
			'$this->mapProp("$2","$6","$7","$4",false,false)',
			'$this->mapK("$2","$4","$5",false,false)',
			'$this->mapGlobal("$2","$4","$5",false,false)',
			'$this->mapLocal("$2","$4","$5",false,false)',
			
			// build-time variables
			'$this->mapProp("$2","$6","$7","$4",true,false)',
			'$this->mapK("$2","$4","$5",true,false)',
			'$this->mapGlobal("$2","$4","$5",true,false)',
//			'$this->mapLocal("$3","$5","$6",true)',
			'$this->mapLocal("$1","$3","$4",true,false)',	

			'$this->mapSystemProp("$2","$3","$4","$1",false)',	

			'$this->expression("$2","$1",false)',
			
			
			'$this->mapRecordsH("$2")',
			'$this->mapRecordsF()',
			'<?php echo $_nxvar_records->count();?>',
			'<?php //<no_record>
			if (!$_nx_recIndex)
			//else
			{
			//</no_record>
			?>',
			'<?php
			//<no_record-end>
			}
			//</no_record-end>
			?>',
			'$this->mapRecordH("$2","$3")',
			'$this->mapRecordF()',
			'$this->mapRecordAttr("$1","$2","$3")',

			'$this->mapFormH("$2","$3")',
			'$this->mapFormF()',
			'$this->mapFormAttr("$1","$2","$3")',

			
			'$this->mapFieldsH("$2")',
			'$this->mapFieldsF()',
			'$this->mapGetFieldH("$1","$2","$3")',
			'$this->mapGetFieldF()',
			'$this->mapField("$2","$3","$4","$1")',
			
			// html utilities
			'$this->mapSpacer("$2","$3","$1")',
			
			// paged records
			// pages.previous
			'<?php if ($_nxvar_nbpages>1 && $_nxvar_page_index>0)
				{
					$_nxvar_page=$_nxvar_page_index-1;
					$_nxvar_page_url = $_nxvar_page_url_.$_nxvar_page;
			?>',
			// pages.next
			'<?php if ($_nxvar_nbpages>1 && $_nxvar_page_index<$_nxvar_nbpages-1)
				{ 
					$_nxvar_page=$_nxvar_page_index+1;
					$_nxvar_page_url = $_nxvar_page_url_.$_nxvar_page;
						?>',
			// pages.all
			'<?php
				if ($_nxvar_nbpages>1) for ($_nxvar_page=0;$_nxvar_page<$_nxvar_nbpages;$_nxvar_page++)
				{
					if($_nxvar_page > $_nxvar_page_index+4 && $_nxvar_page<$_nxvar_nbpages+1)
						continue;

					$_nxvar_page_url = $_nxvar_page_url_.$_nxvar_page; ?>
			',
			// /pages
			'<?php } ?>',
			// page.url
			'<?php echo $_nxvar_page_url; ?>',
			// page.index
			'<?php echo $_nxvar_page+1; ?>',
			// page.limit
			'<?php echo isset($_nxvar_context["recordsPerPage"])?$_nxvar_context["recordsPerPage"]:100; ?>',
			'<?php echo isset($_nxvar_context["recordsPerPage"])?$_nxvar_context["recordsPerPage"]:100; ?>',
			// page.first_record
			'<?php echo $_nxvar_context["firstRecord"]; ?>',
			// page.last_record
			'<?php echo $_nxvar_context["lastRecord"]; ?>',
			// page.nb_records
			'<?php echo $_nxvar_context["totalRecords"]; ?>',
			
			// page.nb_pages
			'<?php echo ceil($_nxvar_nbpages); ?>',
			// page.current
			'<?php echo ($_nxvar_page_index+1); ?>',						
			
			// no_pages
			'<?php else { ?>',
			// /no_pages
			'<?php } ?>',

			// conditional
			'$this->mapIf("$1","$2","$3")',
			'"<?php }else ?>".$this->mapIf("$1","","")',
			'<?php 
		} /*if*/
		else
		{ 
		?>',
			'<?php 
		} /*if*/ 
		?>',			
			'$this->getButton("$2","$1","$3")',
			'$this->mapSubmit("$2")',
			'$this->mapString("$2","$4","$1",false)',
			'',
			'<!--\\1-->'
			);
			
		return preg_replaceX($patterns, $replace,$str,$this);
	}

	function mapCSS($s,$dft,$build)
	{
		return '';
	}

	function mapHeads($s,$dft,$build)
	{
		return '';
	}

	function mapJS($s,$dft,$build)
	{
		return '';
	}


	function mapSubmit($name)
	{
		return '';
	}

	function getButton($name,$build,$urlParams)
	{
		return '';
	}
	
	function mapSpacer($w,$h,$static)
	{
		return '';
	}

}	
	
?>
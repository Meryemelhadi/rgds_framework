<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// singleton
$GLOBALS['NxChoiceList2_docRecs']=array();

class NxChoiceListBase
{
	var $desc;
	var $props;
	var $lang;
	var $package;
	var $dir;

	function __construct(&$props,$name,$dir='lists',$lang='',$prompt='',$desc=null) {

		$this->props =& $props;
		$this->desc = $desc;

		// manage package
		$_name = explode('@',$name);
		if(count($_name)>1)
		{
			$this->package=$_name[1];
			$this->name=$_name[0];
		}
		else
		{
			$this->package=$this->getProperty('nx.package','');
			$this->name=$name;
		}

		$this->dir=ltrim($dir,'/\\');

		global $NX_LANG;
		// if default lang, get it from properties
		if ($lang == null)
			$lang = $NX_LANG;

		$this->lang=$lang;
		$this->prompt=$prompt;
	}

	function &getListAsArray()
	{
		return $this->desc;
	}
	
	function getListAsProperties()
	{
		return new NxProperties($this->desc,$this->props);
	}
		
	function &getChoice($v)
	{
		if (isset($this->desc[$v]))
			return $this->desc[$v];
		elseif (isset($this->desc["?"]))
			return $this->desc["?"];
		else
			return '';
	}
	
	function addChoices($values) {
		
	}	

	/** returns a localised string.
	 * @param v key 
	 * @param deft default value
	 * @param alt_k alternate key to search if k has no value (useful for fields name/labels) */
	function getString($v,$deft=null,$alt_k=null)
	{
		if (isset($this->desc[$v]))
			return $this->desc[$v];
		elseif ($alt_k && isset($this->desc[$alt_k]))
			return $this->desc[$alt_k];
		else
			return $deft===null?$v:$deft;
	}
	
	function & getProperty($name,$deft=null,$cascade=true)
	{
		return $this->props->getProperty($name,$deft,$cascade);
	}

}


class NxChoiceList2 extends NxChoiceListBase
{
	var $desc;
	var $props;
	
	// public interface
	
	function __construct(&$props,$name,$dir='lists',$ds='db:ds_category.list',$lang=null,$prompt=null,$tpl_label='')
	{
		parent::__construct($props,$name,$dir,$lang,$prompt);
		
		//$this->exts=array('html','props','values','words','inc');
		$this->exts=array('inc','html','props','values','words');
			
		$this->ds=$ds;
		$this->tpl_label=$tpl_label;

		// get array
		$this->error = new NxErrorCB();
		
		$this->loadList();
	}
	
	function reload($name=null,$dir='lists') {
		$GLOBALS['NxChoiceList2_docRecs']=array();
	}

	// protected interface

	function loadList()
	{	
		global $NX_LANG;

		// load document
		// get property path
		$path = $this->dir.'/'.strtr($this->name,'.','/');
      
		// if already loaded, returns it
		if (!isset($GLOBALS['NxChoiceList2_docRecs'][$path][$NX_LANG]))
		{		
			// get or build executable
			$exe =& $this->buildExe($path);
			if ($exe==null)
				return $exe;
			
			// execute it and get back an array of doc properties
			$GLOBALS['NxChoiceList2_docRecs'][$path][$NX_LANG] =& $this->execute($exe);
		}
		
		$this->desc =& $GLOBALS['NxChoiceList2_docRecs'][$path][$NX_LANG];

		// wrap it as record
		if ($this->desc == null)
		{
			$this->desc = array();
		}
	}

	//! remove list cache from gen dir
	function refreshList()
	{
		// get dir for the list		
		$fs=$this->props->getPlugIn('FileSys');
		
		// check if output directory exist
		$fs->rmDir($GLOBALS['NX_CONTENT_COMP'].$this->dir.'/'.strtr($this->name,'.','/'));
	}	
	
	function &buildExe($path)
	{
		if (!($exePath = $this->getExePath($path)))
		{
			$this->error->addError("cant find content destination for category: $path");
			return null;
		}
		
		// if production mode (once) and output file exists => returns it
		if (NX_CHECK_CONTENT=='once' && is_file($exePath))
			// deployed once
			return $exePath;
				
		// if DS specified then try it first, else try static doc first
		$src=null;
		if ($this->ds!='')
		{
			//try to load from Data source
			$src=$this->loadFromDS($path);
		}

		if ($src==null)
			// is static source document ?
			if ($srcPath = $this->getSrcPath($path))
			{
				// static input file => check if destination file is up to date
	
				// build refresh ?
				if (NX_CHECK_CONTENT!='compile' && // build
						(is_file($srcPath) &&
						 is_file($exePath) &&
						($ftimeSrc = @filemtime($srcPath)) && 
						($ftimeExe = @filemtime($exePath)) && 
						($ftimeExe-$ftimeSrc)>0 && 
						!isset($_GET["nocache"])))
					return $exePath;
							
				// no => build it as an array of properties (php object)
				
				// create array as a string and save it
				$src = $this->getArrayStr($srcPath);
			}
			else 
			{
				// no static input, check if can be loaded from DB
				$src=$this->loadFromDS($path);
			}

		if ($src)
		{
			// parse properties
			$self=$this;
			$src=preg_replace_callback_array(
				array(
					'/{(?:=|property:)([^}]+)}/'=>
						function($m) { return "'.\$this->getProperty('".$m[1]."').'";},
					'/{(?:\$\$|global:)([^}]+)}/'=>
						function($m) { return "'.\$GLOBALS['".$m[1]."'].'";},
					'/{(?:const(?:ant):)([^}]+)}/'=>
						function($m) { return "'.".$m[1].".'";},
					'/{(?:string:|_)"([^"]*)"(?:[:]"?([^"}]+)"?)?}/'=>
						function($m) use($self) { return $self->mapString("$1","$2");},
				),
				$src);

			/*
			$src=preg_replaceX(
				array(
					'/{(?:=|property:)([^}]+)}/',
					'/{(?:\$\$|global:)([^}]+)}/',
					'/{(?:const(?:ant):)([^}]+)}/',
					'/{(?:string:|_)"([^"]*)"(?:[:]"?([^"}]+)"?)?}/e',
				),
				array(
					"'.\$this->getProperty('$1').'",
					"'.\$GLOBALS['$1'].'",
					"'.$1.'",
					'$this->mapString("$1","$2")',
				),
				$src,
				$this);
			*/

			// save it as a php file
			$this->saveExe($src,$exePath);
		}
		else
		{
			// not found => error..
			$this->error->addError("cant find content source: $path");
			return null;
		}
					
		return $exePath;
	}
	
	function getSrcPath($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;

		if ($this->package)
		{
			foreach ($this->exts as $ext) 
			{
				$basePath=GENERIS_ROOT."packages/{$this->package}/conf/content/{$path}";
				foreach ($this->exts as $ext) 
				{
					/*
					if (defined('PORTAL_PROFILE'))
					{
						$profile = PORTAL_PROFILE;
						if (
							is_file($f="{$basePath}/{$profile}_{$NX_LANG}.{$ext}") ||
							is_file($f="{$basePath}/{$profile}_{$NX_DEFT_LANG}.{$ext}") ||
							is_file($f="{$basePath}/{$profile}.{$ext}")
							)
						{
							nxLog("found content file $f","DS:XDOCS",NX_COMPILE_EVENT);
							$this->ext=$ext;
							return $f;
						}
					}
					*/

					if (
						is_file($f="{$basePath}_{$NX_LANG}.{$ext}") ||
						is_file($f="{$basePath}_{$NX_DEFT_LANG}.{$ext}") ||
						is_file($f="{$basePath}.{$ext}")
						)
					{
						nxLog("found content file $f","DS:XDOCS",NX_COMPILE_EVENT);
						$this->ext=$ext;
						return $f;
					}
				}

				$basePath=NX_CONF."packages/{$this->package}/content/{$path}";
				if (
					is_file($f="{$basePath}_{$NX_LANG}.{$ext}") ||
					is_file($f="{$basePath}_{$NX_DEFT_LANG}.{$ext}") ||
					is_file($f="{$basePath}.{$ext}")
					)
				{
					nxLog("found content file $f","DS:XDOCS",NX_COMPILE_EVENT);
					$this->ext=$ext;
					return $f;
				}
			}

		}
		
		$basePath=NX_CONF."content/{$path}";
		foreach ($this->exts as $ext) 
		{
			if (
				is_file($f="{$basePath}_{$NX_LANG}.{$ext}") ||
				is_file($f="{$basePath}_{$NX_DEFT_LANG}.{$ext}") ||
				is_file($f="{$basePath}.{$ext}") ||
				is_file($f=NX_DIS_CONTENT."{$path}_{$NX_LANG}.{$ext}") ||
				is_file($f=NX_DIS_CONTENT."{$path}_{$NX_DEFT_LANG}.{$ext}") ||
				is_file($f=NX_DIS_CONTENT."{$path}.{$ext}")
				)
			{
				nxLog("found content file $f","DS:XDOCS",NX_COMPILE_EVENT);
				$this->ext=$ext;
				return $f;
			}
		}
			
		nxError("Strings not found/list $path in package {$this->package}","DS:XDOCS");

		return null;
	}
		
	function getExePath($path)
	{
		if (defined('PORTAL_PROFILE'))
			$profile = '.'.PORTAL_PROFILE;
		else
			$profile = '';

		global $NX_LANG;
		//global $NX_DEFT_LANG;
		if ($this->package)
			return $GLOBALS['NX_CONTENT_COMP']."{$path}{$profile}_{$NX_LANG}@{$this->package}.inc";
		else
			return $GLOBALS['NX_CONTENT_COMP']."{$path}{$profile}_{$NX_LANG}.inc";
	}

	function & execute($exe)
	{
		// execute PHP code (that initialises $list)
		if (isset($exe) && $exe != '')
		{
			include($exe);
			return $list;
		}
		return array();
	}

	// =========  parse for static content ===========
	function getArrayStr($srcPath)
	{
		$parser='parse_'.$this->ext;
		$list = $this->$parser($srcPath);

		// serialise the doc
		if (count($list)<=0)
		{
			$this->error->addError('empty document:'.$srcPath);
		}

		return '<?php $list='.$this->var_export($list).';?>';
	}
	
	// parser for .inc files
	function parse_inc($srcPath) 
	{
		// PHP code creating $list as array
		include($srcPath);
		return $list;
	}
	
	// parser for .inc files
	function parse_values($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once('NxLists_values.inc');
		$w=new NxLists_values($this->props);
		return $w->parse($srcPath);
	}
	
	// parser for .inc files
	function parse_props($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once('NxLists_values.inc');
		$w=new NxLists_values($this->props);
		return $w->parse($srcPath);
	}
	
	// parser for .html files
	function parse_html($srcPath) 
	{
		// read document as html "select"
		include_once('NxLists_select.inc');
		$w=new NxLists_select($this->props);
		return $w->parse($srcPath);
	}
	
	// parser for .words files
	function parse_words($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once('NxLists_words.inc');
		$w=new NxLists_words($this->props);
		return $w->parse($srcPath);
	}

	// export of data structure as PHP
	function var_export(&$doc)
	{
		$s = 'array(';
		foreach ($doc as $v=>$text)
		{
            $text=str_replace("'","\\'",$text);
            $v=str_replace("'","\\'",$v);
			$s .= "'{$v}'=>'$text',";
		}
		return $s.')';	
	}
	
	function saveExe($src,$exePath)
	{
		$fs=$this->props->getPlugIn('FileSys');
		
		// check if output directory exist
		if (!is_dir($d=dirname($exePath)))
			if ($fs->createTree($d)==null)
			{
				$this->error->addError('cant create directory :'.$d);
				return false;
			}
			
		// transform it and write to file
		if ($fp = fopen($exePath,'w'))
		{
			// parse and copy to file
			fputs($fp,$src);
			fclose($fp);
			
			nxLog("compiled content to $exePath","DS:XDOCS",NX_COMPILE_EVENT);
			return true;
		}
		else 
		{
			$this->error->addError('cant save content executable :'.$exePath);
			return false;
		}
	}
	
	function loadFromDS($path)
	{
		// get ds settings
		if ($this->ds!='')
		{
			$ads=explode(':',$this->ds);
			switch(count($ads))
			{
				case 2:
					list($media,$view)=$ads;
					break;
				case 1:
					list($view)=$ads;
					$media='db';
					break;
				default:
					$view='ds_category.list';
					$media='db';
					break;				
			}
		}
		else
		{
			$view='ds_category.list';
			$media='db';
		}

		// split media.instance
		$plug=explode('.',$media);
		if (count($plug)>1)
			$inst=$plug[1];
		else
			$inst=false;

		// get db data source plugin		
		$ds =& $this->props->getPlugIn(strtolower($plug[0]),'NxDS_','ds',true,$inst);

		
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","DS_XDOCS");
			return null;
		}
		
		$ds->setProperty('cat.id',preg_replace('|/|','.',$this->name));
		
		// get records

		$records = & $ds->getRecords($view, $this->error);

		// ok we're in, we have the records, lets build the source now..
		$s='$list=array(';
		if ($records != null && $records->count()>0) 
			for ($iter = $records->getRecordIterator(); $iter->isValid(); $iter->next())
			{
				$this->record =& $iter->getCurrent();
				
				// get value
				$this->f_v=$record->getField('cat.value');
				if (!$this->f_v)
					continue;
				$v=$this->f_v->toObject('format');
							
				$self=$this;

				// map template to label
				if ($this->tpl_label=='')
					// simple label from field cat.label
					$label=$this->mapField($this->record,'cat.label',$f_v);
				else
					// complex label from properties, strings or fields
					// ex: "{string:user}:{field:first_name} {field:last_name}" 
					/*
					$label=preg_replaceX(	
		
		array(
			'/{field([:?])([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?}/e',
			), 
		array(
			'$this->mapField($this->record,"$2",$this->f_v,"$1","$3","$4")',
			),
						$this->tpl_label,$this);
					*/
					$label=preg_replace_callback(	
						'/{field([:?])([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?}/',
						function($m) use ($self) {
							$self->mapField($self->record,$m[2],$this->f_v,$m[1],$m[3],$m[4]);
						},
						$this->tpl_label);
												
				$s.= "'$v'=>'$label',\n";
			}
		$s.=');';
		
		return '<?php '.$s.'?>';
	}

	function mapString($str,$pack)
	{
		return $this->getString($str,null,$pack);
	}
	
	function mapProp($p,$isDeft,$dft,$static,$inPHP)
	{
		return $this->getProperty(trim($p),$dft);
	}
	
	// return label for field (or value if field not found)
	function mapField($record,$name,$fval=null,$op,$pref,$suf){
		$f=$record->getField($name);
		if (!$f)
			$f=$fval;
		
		if (!$f)
			return '';
		$r=str_replace("'","\\'",$f->toHTML());

		if($op=='?' && $r)
		{
			$r = $pref.$r.$suf;
		}
			
		return $r;
	}

}

?>
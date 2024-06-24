<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'ds/NxDS_xdocs.inc');

class NxData_EnumSource extends NxProperties
{
	function __construct(&$props)
	{
		$empty=array();
		parent::__construct($empty,$props);

		$this->exts=array('html','props','values','words','inc');
	}
	
	// extension allowing to retreive raw data instead of record
    function & getData($view,&$errorCB)
	{
		$this->error =& $errorCB;
		
		// get actual view
		$view = $this->getProperty($view.'.ds',$view);
		
		// get property path
		$path = $this->path.strtr($view,'.','/');

		// load document
		$doc =& $this->loadDoc($path);

		// wrap it as record
		return $doc;
	}
	
	// load a collection of docs and returns it as a record collection with 
	// doc.href, doc.key, doc.title, doc.text fields.
	function &loadDoc($path)
	{
		// if already loaded, returns it
		if (!isset($this->docRecs[$path]))
		{		
			// get or build executable
			$exe =& $this->buildExe($path);
			if ($exe==null)
				return $exe;
			
			// execute it and get back an array of doc properties
			$this->docRecs[$path] =& $this->execute($exe);
		}
		
		return $this->docRecs[$path];
	}	

	function &buildExe($path)
	{
		if (!($exePath = $this->getExePath($path)))
		{
			$this->error->addError("cant find content destination for category: $path");
			return null;
		}
		
		// if production mode (once) and output file exists => returns it
		if (is_file($exePath))
			// deployed once
			if (NX_CHECK_CONTENT=='once')
				return $exePath;
				
		
		//try to load from DB
		$src=$this->loadFromDB($path);

		if ($src==null)
			// is static source document ?
			if ($srcPath = $this->getSrcPath($path))
			{
				// static input file => check if destination file is up to date
	
				// build refresh ?
				if (NX_CHECK_CONTENT!='compile' && // build
						(is_file($srcPath) &&
						($ftimeSrc = @filemtime($srcPath)) && 
						($ftimeExe = @filemtime($exePath)) && 
						($ftimeExe-$ftimeSrc)>0 && 
						!isset($_GET["nocache"])))
					return $exePath;
							
				// no => build it as an array of properties (php object)
				
				// create array as a string and save it
				$src = & $this->getArrayStr($srcPath);
			}
			/* else
			{
				// no static input, check if can be loaded from DB
				$src=$this->loadFromDB($path);
			}*/

		if ($src)
		{
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

		foreach ($this->exts as $ext) 
		{
			if (is_file($f=NX_CONTENT."{$path}_{$NX_LANG}.{$ext}") ||
				is_file($f=NX_CONTENT."{$path}_{$NX_DEFT_LANG}.{$ext}") ||
				is_file($f=NX_CONTENT."{$path}.{$ext}") ||
				is_file($f=NX_DIS_CONTENT."{$path}_{$NX_LANG}.{$ext}") ||
				is_file($f=NX_DIS_CONTENT."{$path}_{$NX_DEFT_LANG}.{$ext}") ||
				is_file($f=NX_DIS_CONTENT."{$path}.{$ext}"))
			{
				$this->ext=$ext;
				return $f;
			}
		}
			
		return null;
	}
		
	function getExePath($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;	
		return $GLOBALS['NX_CONTENT_COMP']."{$path}/{$NX_LANG}.inc";
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
	function & getArrayStr($srcPath)
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
		include_once('lists/NxLists_values.inc');
		$w=new NxLists_values($this);
		return $w->parse($srcPath);
	}
	
	// parser for .inc files
	function parse_props($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once('lists/NxLists_values.inc');
		$w=new NxLists_values($this);
		return $w->parse($srcPath);
	}
	
	// parser for .html files
	function parse_html($srcPath) 
	{
		// read document as html "select"
		return $this->parse($srcPath);
	}
	
	// parser for .words files
	function parse_words($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once('lists/NxLists_words.inc');
		$w=new NxLists_words($this);
		return $w->parse($srcPath);
	}

	// export of data structure as PHP
	function var_export(&$doc)
	{
		$s = 'array(';
		foreach ($doc as $v=>$text)
		{
			$text=str_replace("'","\\'",$text);
			$s .= "'{$v}'=>'$text',";
		}
		return $s.')';	
	}
	
	function saveExe($src,$exePath)
	{
		$fs=$this->getPlugIn('FileSys');
		
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
	
	function loadFromDB($path)
	{
		// get db data source plugin
		$media='db';
		$view='ds_category.list';
		
		$ds =& $this->getPlugIn(strtolower($media),'NxDS_','ds');
		
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","DS_XDOCS");
			return null;
		}
		
		$ds->setProperty('tree.id',preg_replace('|/|','.',$path));
		
		// get records
		$records = & $ds->getRecords($view, $this->error);

		// check results
		if ($records == null || $records->count()==0) 
			return null;
			
		// ok we're in, we have the records, lets build the source now..
		$s='$list=array(';
		for ($iter = $records->getRecordIterator(); $iter->isValid(); $iter->next())
		{
			$record =& $iter->getCurrent();
			
			// get value and label
			$f_v=$record->getField('cat.value');
			if (!$f_v)
				continue;
								
			$f_l=$record->getField('cat.label');
			if (!$f_l)
				continue;
				
			$label=$f_l->toHTML();
			$v=$f_v->toObject();
							
			$s.= "'$v'=>'$label',\n";
		}
		$s.=');';
		
		return '<?php '.$s.'?>';
	}
}

?>
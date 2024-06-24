<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'ds/NxDS_xdocs.inc');

class NxDS_props extends NxDS_xdocs
{
	function __construct(&$props,$dir='props')
	{
		parent::__construct($props,'');
		$this->exts=array('props','inc');
	}

	function getRecordCount($view, &$errorCB)
	{		
		$options =& $this->getRecords($view, $errorCB);
		return $options->count();
	}
	
	function & getArrayStr($srcPath)
	{
		$parser='parse_'.$this->ext;
		$list = $this->$parser($srcPath);

		// serialise the doc
		if (count($list)>0)
		{
			return '<?php $properties='.$this->var_export($list).';?>';
		}
		else
		{
			$this->error->addError('empty document:'.$srcPath);
			return null;
		}
	}
	
	// parser for .inc files
	function parse_inc($srcPath) 
	{
		// PHP code creating $list as array
		include($srcPath);
		
		if (isset($properties))
			return $properties;
			
		return $list;
	}
	
	// parser for .inc files
	function parse_props($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once('lists/NxLists_values.inc');
		$w=new NxLists_values($this);
		return $w->parse($srcPath);
	}
	
	function getSrcPath($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;

		foreach ($this->exts as $ext) 
		{
			if (is_file($f=NX_CONF."{$path}_{$NX_LANG}.{$ext}") ||
				is_file($f=NX_CONF."{$path}_{$NX_DEFT_LANG}.{$ext}") ||
				is_file($f=NX_CONF."{$path}.{$ext}") ||
				is_file($f=NX_DIS_CONF."{$path}_{$NX_LANG}.{$ext}") ||
				is_file($f=NX_DIS_CONF."{$path}_{$NX_DEFT_LANG}.{$ext}") ||
				is_file($f=NX_DIS_CONF."{$path}.{$ext}"))
			{
				$this->ext=$ext;
				return $f;
			}
		}
			
		$this->error->addError("cant find content source: $path");
		return null;
	}
		
	function getExePath($path)
	{
		global $NX_LANG;
		return NX_GEN."{$path}_{$NX_LANG}.inc";
	}

	function & execute($exe)
	{
		// execute PHP code (that initialises $list)
		include($exe);
		return $properties;
	}

	// export of data structure as PHP
	function var_export(&$doc)
	{
		$s = 'array(';
		foreach ($doc as $n=>$v)
		{
			if (is_array($v))
			{
				$v=$this->var_export($v);
			}
			else
			{
				$v=str_replace("'","\\'",$v);
				if ($v!="false" && $v!="true")
					$v="'$v'";
			}
								
			$s .= "'{$n}'=>$v,";
		}
		return $s.')';	
	}
}

?>
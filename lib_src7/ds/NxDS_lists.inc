<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'ds/NxDS_xdocs.inc');

class NxDS_lists extends NxDS_xdocs
{
	function NxDS_lists(&$props,$dir='lists')
	{
		parent::__construct($props,"$dir/");
		$this->exts=array('html','props','values','words','inc');
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
		include_once(NX_LIB.'data/fields/lists/NxLists_values.inc');
		$w=new NxLists_values($this);
		return $w->parse($srcPath);
	}
	
	// parser for .inc files
	function parse_props($srcPath) 
	{
		// read document as a bag of words enclosed by <words></words>
		include_once(NX_LIB.'data/fields/lists/NxLists_values.inc');
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
		include_once(NX_LIB.'data/fields/lists/NxLists_words.inc');
		$w=new NxLists_words($this);
		return $w->parse($srcPath);
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
	
	// parse html links from file and return a record collection
	function &parseDoc($src)
	{
		nxLog("list:[$src]",'DS:LISTS');
		
		// extract options
		preg_replaceX('%<select[^>]*>(.*)</select>%sie','$this->select[]="$1";',$src,$this);
		
		if (count($this->select) == 0)
			// not html => try it as php
			return null;
			
		// get first select of the doc
		$src=$this->select[0];
		
		// tags to lowercase
		$src=preg_replace('#</option>#i','</option>',$src);
		
		// split up options
		$optionsStr = explode('</option>',$src);

		// pattern/replace arrays
		$patterns = array(
			'%value="([^"]+)"%sie',
			'%>(.*)$%esi',
			);
			
		$replace = array(
			'$this->setOptProp("node.value","$1")',
			'$this->setOptProp("node.text","$1")',
			);
		
		// get links properties: link.href, link.key, link.title, link.text
		$this->options=array();
		
		// parse each option
		foreach ($optionsStr as $optionstr)
		{
			$this->option=array();
					
			// extract option properties
			preg_replace($patterns, $replace,$optionstr);
			
			// add option to collection as value=>text
			if (isset($this->option['node.text']))
			{
				$text=$this->option['node.text'];
				
				if (isset($this->option['node.value']))					
					$key=$this->option['node.value'];
				else
					// no value=>reformat text as key
					$key='?-'.preg_replace('#[ -]#','_',$text);

				$this->options[$key]=$text;
			}
		}

		return $this->options;
	}	
	
	function setOptProp($p,$v)
	{
		nxLog("HTML content , node[$p]=$v","DS::LIST");
		$v=str_replace('#$#','$',$v);		
		$this->option[$p]=$v;
		return '';
	}	
}

?>
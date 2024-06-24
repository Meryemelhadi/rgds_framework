<?php

// handler for building select lists from list of words.
// read words and create codes by using a dictionary.
// returns an array of the form code => word
class NxLists_words 
{
	var $dict,$prefix;
		
	function __construct(&$props)
	{
		$this->props=&$props;
	}
	
	function parse($path) 
	{
		// read file
		$src=NxDS_Source::file_get_contents($path);
			
		$bags=explode("</words>",$src);
		
		foreach ($bags as $bagSrc)
		{
			// process <words></words> sections
			$patt=array(
				'/<!--(.*)-->/s',
				'#<words(?:\s+(\w+)="([^"]+)"(?:\s+(\w+)="([^"]+)")?)?\s*>(.*)$#ise', // attrs
			);
			
			$rep=array(
				'',
				'$this->processWords("$5","$1","$2","$3","$4");',
			);
			preg_replaceX($patt,$rep,$bagSrc,$this);
		}
		
		// for each found bag, add codes=>words to map
		$catLen=count($this->codes);
		$map=array();
		for ($b=0;$b<$catLen;$b++)
		{
			// get bag of codes,words
			$codes = $this->codes[$b];
			$words = $this->words[$b];
			
			// build list from codes and words
			$len=count($codes);
			for ($i=0;$i<$len;$i++)
				$map[$codes[$i]]=$words[$i];			
		}
		
		
		return $map;
	}
	
	function processWords($words,$a1,$v1,$a2,$v2) 
	{
		$dictID='';
		$prefix='';
		
		if ($a1=='dict')
		{
			$dictID=$v1;
			
			if ($a2=='prefix')
				$prefix=$v2;
		}
		elseif ($a1=='prefix')
		{
			$prefix=$v1;
			
			if ($a2=='dict')
				$dictID=$v2;
		}
		
		$words = trim($words);
		$this->words[]=explode("\n",$words);
		$this->codes[]=$this->processCodes($words,$prefix,$dictID);
		return '';
	}
	
	function processCodes($words,$prefix,$dictID) 
	{
		// extract words, put in lower case, replace sep by '_'
		$patt=array(
			'/\n/',
			"/(?:[ \t']|-)+/",	// seps
			'#\|#',
			'#^#',
		);
		
		$rep=array(
			'|',
			'_',
			'|'.$prefix,
			$prefix,
		);
		
		$codes=trim(strtolower(preg_replace($patt,$rep,$words)),'|');
		
		// get and apply dictionary
		if ($dictID)
		{
			if($dict=$this->loadDict($dictID))
			{
				// replace words by their code (dict is array code=>word)
				$codes=str_replace(array_values($dict),array_keys($dict),$codes);
			}
		}
		
		// transform string to array of word
		$codes=explode('|',$codes);
		
		return $codes;
	}
	
	function loadDict($dictName) 
	{
		// load dictionary as a regular list
		// get data source plugin
		$ds =& $this->props->getPlugIn('lists','NxDS_','ds');
				
		// get dictionary as an array
		$error=new NxErrorCB();
		$desc = & $ds->getData($dictName, $error);
			
		if ($desc)
			return $desc;
		else
			return null;
	}
}

?>

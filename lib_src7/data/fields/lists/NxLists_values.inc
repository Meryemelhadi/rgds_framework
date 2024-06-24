<?php

// handler for building select lists from list of words.
// read words and create codes by using a dictionary.
// returns an array of the form code => word
class NxLists_values 
{		
	var $index;
	var $seq;
	var $prefix;
	var $list;
	
	function __construct(&$props)
	{
		$this->props=&$props;
	}
	
	function parse($path) 
	{
		// read file
		$src=NxDS_Source::file_get_contents($path);
		$src=implode(';',explode("\n",$src));
			
		$this->index=0;
		$this->seq=false;
		$this->prefix='';
		$this->list=array();
		
		$patt=array(
			'/&([^;]+);/',
			'/[{<]!--(.*)--[}>]/sU',
			'/$/',
			'/prefix:([^;]*);+/ie',
			'/prompt:([^;]*);+/ie',
			'/seq:([^;]*);+/ie',
			'/sep:([^;]*);+/ie',
			'/\s+;+/',
			'/(?:([^=; \t]+)=)?([^;]+);+/e',
		);
		
		$rep=array(
			'#$1##',
			'',
			';',
			'$this->setPrefix("$1");',
			'$this->setPrompt("$1");',
			'$this->setSeq("$1");',
			'$this->setSep("$1");',
			';',
			'$this->processValues("$1","$2");',
		);
		preg_replaceX($patt,$rep,$src,$this);
		
		return $this->list;
	}
	
	function setPrefix($p)
	{
		$this->prefix=$p;
		return '';
	}
	
	function setPrompt($p)
	{
		$p=preg_replace('/#([^#]+)##/','&$1;',$p);
		$this->list['?']=$p;
		return '';
	}
	
	function setSeq($start)
	{
		$this->index=$start;
		$this->seq=true;
		return '';
	}
	function setSep($start)
	{
		$this->sep=$start;
		return '';
	}
	
	function processValues($k,$v) 
	{
		$k=trim($k);
		$v=preg_replace('/#([^#]+)##/','&$1;',$v);
		if ($v)
		{
			// get key, if sequence assign an index, add prefix
			if ($k==='')
				if ($this->seq)
					$k=($this->index++);
				else
					$k=strtolower($v);
			
			$this->list[$this->prefix.$k]=$v;
		}
		return '';
	}
}

?>

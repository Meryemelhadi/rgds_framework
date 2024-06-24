<?php

include_once('NX_DOM.inc');

class NxSax_dump
{
	var $parser;
	var $createTree=false;
	var $srcPath='';
	
	function __construct($type="php",$display=true) {
		$parserClass="NxSax_{$type}";
		include_once("{$parserClass}.inc");
		$this->parser=&new $parserClass($this);
	    
		if ($display)
	    {
	    	$this->display=true;
		    $this->gt='&gt;';
		    $this->lt='&lt;';
		    $this->res= "<pre>";
	    }
	    else
	    {
	    	$this->display=false;
		    $this->gt='>';
		    $this->lt='<';
		    $this->res= '';
	    }		
	}
	
	function parseFile($srcPath) {	
		$this->srcPath=$srcPath;
		if(($src=$this->file_get_contents($srcPath))===null)
		{
			$this->_error(-1,"cant find XML doc: [$srcPath]");
			return false;
		}
		
		$this->parser->parse($src);
		return $this->parser->cb->res;
	}
	
	function & processAttributes($attributes)
	{
		$atts=array();
		foreach ($attributes as $n=>$v)
		{
			$n=strtolower($n);
			$atts[$n]=$v;
		}
			
		return $atts;
	}
	
	function close() {		
		// cleanup
		$this->parser->close();
	}
		
	function cdata($parser, $cdata)
	{
		if ($this->display)
			$cdata=htmlentities($cdata);
			
		$this->res.=$cdata;
	}
		
	function _text($cdata) {
		$this->res.=$cdata;
	}
	
	function otherXML($parser, $cdata)
	{
		if ($this->display)
			$cdata=htmlentities($cdata);
		$this->res.=$cdata;
	}
					
	function tag_open($parser, $tagName, $attributes)
	{ 
		// process as text
		$attributes=$this->processAttributes($attributes);
		$tagName=strtolower($tagName);
		
        $text="{$this->lt}{$tagName}";
        $attrValues = $attributes;
        $attrNames = array_keys($attrValues);
        $size = count($attrNames);
        for ($i = 0; $i < $size; $i++) {
                $name = $attrNames[$i];
                $text.=" {$attrNames[$i]}=\"{$attrValues[$name]}\"";
        }
        $text.=$this->gt;
		$this->res.= $text;
	}
	
	function tag_close($parser, $tagName)
	{
		$tagName=strtolower($tagName);

		// process as text
        $text="{$this->lt}/{$tagName}";
        $text.=$this->gt;
		$this->res.= $text;
	}
	
	// ========== managing comments ====
	function _comment_open() {
		$this->res.="{$this->lt}!==";	
		return true;
	}
	
	function _comment_close() {
		$this->res.="--{$this->gt}";
		return true;
	}	
	
	function _error($line,$err,$tag='') 
	{
		if ($tag)
			echo "error: $err at line $line, tag=$tag in file [{$this->srcPath}]";
		else
			echo "error: $err at line $line in file [{$this->srcPath}]";
	}
	
	// ========= Utility ===========
	function &file_get_contents($filename) 
	{
		$fp = @fopen($filename, "r");
		if (!($fp))
			return null;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		return $temp;
	}	
	
}
?>
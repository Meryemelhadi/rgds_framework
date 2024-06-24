<?php

include_once('NX_DOM.inc');    
//include_once('NX_DOM'.NX_PHP_VER.'.inc');    
/* 
	1) parse document with SAX parser, 
	2) build DOM tree with DOM builder 
*/
class NX_parserXML
{
	var $parser;
	var $createTree=false;
	var $srcPath='';
	
	function __construct($createTree=false,$type="html") {
		$parserClass="NxSax_{$type}";
		include_once("{$parserClass}.inc");
		$this->parser=new $parserClass($this);
	
		$this->createTree=$createTree; // automatically create the tree ?
		$this->dom=new NxDOM_DomBuilder();
	}
	
	function parseFile($srcPath) {	
		$this->srcPath=$srcPath;
		if(($src=$this->file_get_contents($srcPath))===null)
		{
			$this->_error(-1,"cant find XML doc: [$srcPath]");
			return false;
		}
		
		$this->dom->doc->documentURI=$srcPath;
		
		return $this->parser->parse($src);
	}
	
	function parse($src,$uri='(inline)') {	
		$this->dom->doc->documentURI=$uri;
		
		return $this->parser->parse($src);
	}
	
	function close() {		
		// cleanup
		$this->parser->close();
	}
	
	function &getDOM() {
		return $this->dom;
	}

	function &getDOMDoc() {
		return $this->dom->doc;
	}
	
	function cdata($parser, $cdata)
	{
		// $textNode=$this->createTextNode($cdata);
		$this->dom->appendText($cdata);		
		$this->_text($cdata);
	}
		
	function _text($cdata) {
	}
	
	function otherXML($parser, $cdata)
	{
		// echo("[[$cdata]]");
	}
			
	function & processAttributes($attributes)
	{
		$atts=array();
		foreach ($attributes as $n=>$v)
		{
			$n=strtolower($n);
			if (preg_match('/^xmlns(?:[:](.+))?$/i',$n,$matches))
			{
				if (isset($matches[1])&&$matches[1])
					$atts['_nsdef'][]=array('prefix'=>$matches[1],'uri'=>$v);
				else
					$atts['_nsdef'][]=array('prefix'=>'','uri'=>$v);
			}
			else					
				$atts[$n]=$v;
		}
			
		return $atts;
	}
		
	function tag_open($parser, $tag, $attributes)
	{ 
		$atts=&$this->processAttributes($attributes);
		$node=&$this->dom->createNode($tag,$atts);
			
		if (method_exists($this, $f="{$tag}_open"))
		{
			if (($err=$this->$f($node))!==true)
			{
				$errorline=$this->parser->getCurrentLine();
				$this->_error($errorline,$err,$tag);				
			}
		}
		elseif (($err=$this->_open($tag,$node))!==true)
		{
			$errorline=$this->parser->getCurrentLine();
			$this->_error($errorline,$err,$tag);				
		}
		
		if ($err!==true)
			$this->dom->invalidateNode();
	}
	
	function tag_close($parser, $tag)
	{
		if (method_exists($this, $f="{$tag}_close"))
		{
			if (($err=$this->$f())!==true)
			{
				$errorline=$this->parser->getCurrentLine();
				$this->_error($errorline,$err,$tag);				
			}
		}
		elseif (($err=$this->_close($tag))!==true)
		{
			$errorline=$this->parser->getCurrentLine();
			$this->_error($errorline,$err,$tag);				
		}
		
		if ($err!==true)
			$this->dom->invalidateNode();
		
		$this->dom->closeNode();
	}
	
	// default tag handlers
	function _open($tag,$node) {
		return true; 
	}
	function _close($tag) {
		return true; 
	}
	
	// ========== managing comments ====
	function _comment_open() {
		$this->dom->setTextMode(true);	
		return true;
	}
	
	function _comment_close() {
		$this->dom->setTextMode(false);
		return true;
	}	
	
	// ========== managing doc types ====
	function _doc_type_open() {
		$this->dom->setTextMode(true);	
		return true;
	}
	
	function _doc_type_close() {
		$this->dom->setTextMode(false);
		return true;
	}	
	
	// ========== managing PI <?xxx  ====
	function pi__open() {
		$this->dom->setTextMode(true);	
		return true;
	}
	
	function pi__close() {
		$this->dom->setTextMode(false);
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
		$fp = @fopen($filename, "r",'b');
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
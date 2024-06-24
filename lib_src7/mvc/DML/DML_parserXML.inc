<?php

class DML_ParserXML
{
	var $parser;
	
	function __construct() {
		$this->parser = xml_parser_create('UTF-8');
	}
	
	function close() {
		// cleanup
		xml_parser_free($this->parser);
	}

	function parseFile($srcPath) {	
		if($src=$this->file_get_contents($srcPath))
			$this->parseXML($src);
		else
			echo "cant find XML doc";
	}
	
	function parseXML($src) {	
		// prepare parser
		xml_parser_set_option($this->parser,XML_OPTION_TARGET_ENCODING,'UTF-8');
		xml_set_object($this->parser, $this);
		xml_set_element_handler($this->parser, "tag_open", "tag_close");
		xml_set_character_data_handler($this->parser,"cdata");
		xml_set_default_handler($this->parser,"otherXML");		
		xml_parse($this->parser, $src);
	}
	
	function tag_open($parser, $tag, $attributes)
	{ 
		if (method_exists($this, $f="{$tag}_open"))
			if (($err=$this->$f($attributes))!==true)
			{
				$errorline=xml_get_current_line_number($parser);
				echo "error(line:$errorline): tag $tag, calling $f, returned[$err]";
			}
	}
	
	function cdata($parser, $cdata)
	{
	}
	
	function otherXML($parser, $cdata)
	{
	}
	
	function tag_close($parser, $tag)
	{
		if (method_exists($this, $f="{$tag}_close"))
			$this->$f();
	}
	
	// --------- utilities -------
	function &file_get_contents($filename) 
	{
		$fp = @fopen($filename, "r");
		if (!($fp))
			return 0;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		return $temp;
	}

}
?>
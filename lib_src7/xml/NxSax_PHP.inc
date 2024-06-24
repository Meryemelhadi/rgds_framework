<?php
class NxSax_PHP
{
	var $parser;
	
	function __construct(&$cb) {
		$this->cb=&$cb;
		$this->parser =& xml_parser_create();
	}
	
	function close() {
		// cleanup
		xml_parser_free($this->parser);
	}
	
	function parse($src) {	
		// prepare parser
		xml_parser_set_option($this->parser,XML_OPTION_TARGET_ENCODING,'UTF-8');
		xml_set_object($this->parser, $this->cb);
		xml_set_element_handler($this->parser, "tag_open", "tag_close");
		xml_set_character_data_handler($this->parser,"cdata");
		xml_set_default_handler($this->parser,"otherXML");		
				
		$src=$this->preprocess($src);
		if (!xml_parse($this->parser, $src))
		{
			$this->cb->_error(xml_get_current_line_number($this->parser),
				 xml_error_string(xml_get_error_code($this->parser)));
			return false;
		}
		
		return true;
	}
	
	function getCurrentLine()
	{
		return xml_get_current_line_number($this->parser);
	}
		
	// --------- utilities -------
	// transform comments into element and add entity references found in doc.
	function preprocess($src)
	{
		$ents='';
		if (preg_match_all('/&([^;&]+);/',$src,$entities)>1)
		{
			foreach (array_unique($entities[1]) as $e)
			{
				$ents.="<!ENTITY $e \"&{$e};\">";				
			}

			return preg_replace(
					array(
						'/<nxl([^:])/',
						'/<!--/s','/-->/'
						),
					array(
	//					'<!DOCTYPE nxl [<!ENTITY nbsp "&nbsp;">]>
						'<!DOCTYPE nxl ['.$ents.']>
	<nxl$1',
						'<_comment><![CDATA[',']]></_comment>'),
					$src);
		}
		else
			return preg_replace(
					array(
						'/<!--/s','/-->/'
						),
					array(
						'<_comment><![CDATA[',']]></_comment>'),
					$src);
	}
}
?>
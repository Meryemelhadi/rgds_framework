<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_format
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}
	
	// replace utf8 buggy html_entity_decode
    function html_entity_decode($string, $quote_style = ENT_COMPAT, $charset = null)
    {
        if (!is_int($quote_style)) {
            nxError('html_entity_decode() expects parameter 2 to be long, ' . gettype($quote_style) . ' given', 'PLUG_FORMAT');
            return '';
        }

        $trans_tbl = get_html_translation_table(HTML_ENTITIES);
        $trans_tbl = array_flip($trans_tbl);

        // Add single quote to translation table;
        $trans_tbl['&#039;'] = '\'';

        // Not translating double quotes
        if ($quote_style & ENT_NOQUOTES) {
            // Remove double quote from translation table
            unset($trans_tbl['&quot;']);
        }

        return strtr($string, $trans_tbl);
    }
    
    // transforms HTML fragment into a set of words (for indexing)
    function stripTags($s,$encod='UTF-8') {
    	return preg_replace(
				array('/[\'"\n\r\t]+/','/[ ]+/'),
				array(' ',' '),
				NxPlug_format::html_entity_decode(
					strip_tags(
						preg_replace(
							array('/</','/>/',),
							array(' <','> ',),
							$s)),
					ENT_QUOTES,$encod));
    }
    
    // transforms HTML fragment into a set of words (for indexing)
    function parseTPL($s) {
    	return preg_replaceX(
				array('/{@(?:=|property:)([^}]+)}/ie',),
				array('$this->props->getProperty("$1")'),
				$s,$this);
    }
}

?>

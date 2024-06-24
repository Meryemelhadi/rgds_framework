<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
	
/* manage template files with html extension. Manage head and body tags in an html page. */
class NxPlug_TplMap_doc
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}

	function translate($src,&$tpl)
	{
		// extract head and returns body	
		$patterns = array(
			'/<!--!((((.)(?!-->))*)(.)?)-->/s',
			'%<\?xml(.*)\?>%eis',
			'%<\?mso-application((?:.*))\?>%ise',
			'%___([a-z_0-9]+)%ise',
			'%_S?_([a-z_0-9]+)%ise',
			);
			
		$replace = array(
			'$1',
			'',
			'',
			'{field:$1}',
			'{field.string:$1}'
			);

		// extract head, title and body code
		$res = '{records:page.records}{record}'.preg_replaceX($patterns, $replace,$src,$this).'{/record}{/records}';
		
		return $res;
	}

	function transformNXL($src)
	{
 		return $src;
	}

}

?>
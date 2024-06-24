<?
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
	
/* manage template files with html extension. Manage head and body tags in an html page. */
class NxPlug_TplMap_xsl
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
			'%<\?xml(.*)\?>%ise',
			'%<\?mso-application((?:.*))\?>%ise',
			);
			
		$replace = array(
			'$1',
			'',
			''
			);

		// extract head, title and body code
		$res = preg_replaceX($patterns, $replace,$src,$this);
		
		return $res;
	}

	function transformNXL($src)
	{
 		return $src;
	}
}

?>
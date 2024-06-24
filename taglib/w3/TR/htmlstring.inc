<?PHP

include_once(NX_TAG_LIB.'w3/1999/xhtml.inc');

// library for the creation of html or xml views
// URI: http://www.w3.org/TR/REC-html40
class taglib_w3_TR_htmlstring extends taglib_w3_1999_xhtml
{		
	
	function __construct(&$context)
	{
		parent::__construct($context);
	}

	function html($node,$content,$level,$pad)
	{
		return $this->passThrough($node,$content,$pad,true);
	}
		
	// overload parseVal without wrapping attributes with <?php 
	function parseVal($v,$quotes=false)
	{
		if (preg_match('/#[^#]+#/',$v))
		{
			$v = parent::parseVal($v,true,false);
			if (preg_match('/\$[_a-z]+(->|\[)/i',$v))
				return "'.{$v}.'";
			else
				return trim($v,"'");
			
		}
		
		return $v;
	}
	function comment_($node,$content,$level,$pad)
	{	
		return '';		
	}
}

?>
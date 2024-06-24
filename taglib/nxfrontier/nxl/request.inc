<?PHP

// core library for nxl language
class taglib_nxfrontier_nxl_request extends NxTagLib
{
	function header()
	{
		return "// import library session";
	}
	
	function get($node,$content,$level,$pad)
	{	
		/* media */
		$media=$node->getAttribute('media','request');
		$var='_'.strtoupper($media);
		
		/* name */
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('error in session:get : no "name" attribute');
		
		/* value */
		if (($value=$node->getAttribute('value'))!==null)
		{
			$value = $this->parseVal($value);
		}
		else
		{
			$value=$content;
		}
			
		return "\n{$pad}(isset(\${$var}['$name'])?\${$var}['$name']:$value)";
	}

	function set($node,$content,$level,$pad)
	{	
		/* media */
		if (($media=$node->getAttribute('media'))===null)
			$media='request';
		$var='_'.strtoupper($media);
		
		/* name */
		if (($name=$node->getAttribute('name'))===null)
			$this->fatal('error in session:get : no "name" attribute');
		
		/* value */
		if (($value=$node->getAttribute('value'))!==null)
		{
			$value = $this->parseVal($value);
		}
		else
		{
			$value=$content;
		}
			
		return "{$pad}\${$var}['$name']=$value;";
	}
}

?>

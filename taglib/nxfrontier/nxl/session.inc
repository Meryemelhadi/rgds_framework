<?PHP

// core library for nxl language
class taglib_nxfrontier_nxl_session extends NxTagLib
{	
	function header()
	{
		return "// import library session";
	}
	
	function get($node,$content,$level,$pad)
	{	
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
		if ($value==='')
			$value='null';
			
		return "\n{$pad}(isset(\$_SESSION['$name'])?\$_SESSION['$name']:$value)";
	}

	function set($node,$content,$level,$pad)
	{	
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
			
		return "{$pad}\$_SESSION['$name']=$value;";
	}

	function destroy($node,$content,$level,$pad)
	{				
		return "\n{$pad}session_destroy();";
	}
	
	function cleanup($node,$content,$level,$pad)
	{				
		return "\n{$pad}\$_SESSION=array();";
	}

}

?>

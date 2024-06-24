<?PHP

// core library for nxl language
class taglib_nxfrontier_nxl_php extends NxTagLib
{	
	function header()
	{
		return "// import library PHP";
	}
	
	function script_($node,$content,$level,$pad)
	{
		return "\n{$content}\n";
	}
}

?>
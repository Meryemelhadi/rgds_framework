<?PHP

// core library for nxl language
class taglib_nxfrontier_nxl_db extends NxTagLib
{	
	function header()
	{
		return "// import library DB";
	}
	
	function listDB($instr,$content,$level,$pad)
	{	
		return "
{$pad}// list db
{$pad}listDB();";
	}
}

?>
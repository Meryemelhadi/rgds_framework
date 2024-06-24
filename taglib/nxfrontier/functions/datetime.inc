<?PHP

// all data format operations
class taglib_nxfrontier_functions_datetime extends NxTagLib
{		
	function gettime($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);

		if (trim($v)=='')
		{
			return "time()";
		}
		else
		{
			return "strtotime('$v')";
		}
	}
	
	function toString($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		$this->getAttribute($fmt,$node,'format','F j, Y',true,true);

		if (trim($v)=='')
		{
			return "date($fmt)";
		}
		else
		{
			return "date($fmt,$v)";
		}
	}
}

?>
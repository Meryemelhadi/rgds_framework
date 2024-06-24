<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_custom
{
	function __construct()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm(&$value,&$desc)
	{
		// $F_ = $desc->name;
		$fname = $this->getAlias();
	
		// $inputVal = eregi_replace("\"", "&quot;", $this->value);
		$inputVal = $value;
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
			
		$control = $desc->getProperty("control","rich_text");
			
		ob_start();
			include(NX_CONF."controls/$control/edit.inc");
		$res=ob_get_contents();
		ob_end_clean();
		
		return $res;
	}

	function readForm(&$recData,&$desc)
	{
		//$fname = $desc->name;
		$fname = $desc->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=="")
			return null;

		return  
//				ereg_replace("\"","&quot;",
					ereg_replace("'","&#39;",
						ereg_replace( "[<]\?","&lt;&#3f;", 
							ereg_replace( "<script","&lt;script",
								$recData[$fname]
							)
						)
//					)
				);
	}
}

?>
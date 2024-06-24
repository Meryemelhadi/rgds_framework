<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxControl_fck_editor
{
	function __construct()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm(&$value,&$desc)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		
		// $F_ = $desc->name;
		$fname = $desc->getAlias();
	
		// $inputVal = eregi_replace("\"", "&quot;", $this->value);
		$inputVal = $value;
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $value.'<input type="hidden" name="'.$fname."\" value='".$inputVal."' />";
			
		include_once(NX_DOC_ROOT.'nx/FCKeditor/fckeditor.php');
		$oFCKeditor = new FCKeditor($fname) ;
		$oFCKeditor->BasePath = '/nx/FCKeditor/';
		$oFCKeditor->Config['nx_skin_url']=$NX_SKIN_URL;
		$oFCKeditor->Config['nx_skin_default_url']=$NX_DEFT_SKIN_URL;
		$oFCKeditor->Value = $value;
		
		return $oFCKeditor->CreateHtml();
	}

	function readForm(&$recData,&$desc)
	{
		//$fname = $desc->name;
		$fname = $desc->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=="")
			return null;						

		return
			preg_replace(
				array(
					//'/<([\w]+) (?:style|class)=([^>]*)([^>]*)/i',
					'/<\\?\??xml[^>]*>/i',
					'/<script[^>]*>[^<]*<\/script>/i',
					'/<\/?\w+:[^>]*>/i',
					'/<\/?font[^>]*>/i',
					
					"/'/",
					"/[<]\?/",
					"/<script/i",
					'/(&nbsp;)+\n?/i',
					'/(&nbsp;)+\n?/i'
					),
				array(
					//'<$1$3',
					'',
					'',
					'',
					'',
					
					"&#39;",
					"&lt;&#3f;",
					"&lt;script",
					'',''
					),
					$recData[$fname]
				);							
	}
}

?>
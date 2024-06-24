<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxControl_fck_editor
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if (!is_file(NX_DOC_ROOT.'nx/FCKeditor/fckeditor.php'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/FCKeditor/',NX_DOC_ROOT.'nx/FCKeditor/');
		}
		return true;
	}
	
	function toForm(&$value,&$f)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();
	
		// $inputVal = eregi_replace("\"", "&quot;", $this->value);
		$inputVal = $value;
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $value.'<input type="hidden" name="'.$fname."\" value='".$inputVal."' />";
			
		$this->init($desc);
		
		include_once(NX_DOC_ROOT.'nx/FCKeditor/fckeditor.php');
		$oFCKeditor = new FCKeditor($fname) ;
		$oFCKeditor->BasePath = '/nx/FCKeditor/';
		$oFCKeditor->Config['nx_skin_url']=$NX_SKIN_URL;
		$oFCKeditor->Config['nx_skin_default_url']=$NX_DEFT_SKIN_URL;
		$oFCKeditor->Value = $value;
		
		return $oFCKeditor->CreateHtml();
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
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
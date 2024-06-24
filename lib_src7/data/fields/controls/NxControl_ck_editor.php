<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
if (!isset($GLOBALS['form_ckeditor']))
	$GLOBALS['form_ckeditor']=false;

class NxControl_ck_editor
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_ckeditor'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/ckeditor/ckeditor.php'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/ckeditor/',NX_DOC_ROOT.'nx/controls/ckeditor/');
		}
		$GLOBALS['form_ckeditor']=true;
		return false;
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
			
		$s='';

		// add resources once
		if (!$this->init($desc))
		{
			include_once(NX_DOC_ROOT.'nx/controls/ckeditor/ckeditor.php');

			$s.='
	<script>
		if (typeof ajax_require != "undefined")
		{
			if(typeof window.CKEDITOR_BASEPATH == "undefined")
				window.CKEDITOR_BASEPATH = "/nx/controls/ckeditor/";

			if(typeof CKEDITOR == "undefined")
				ajax_require("/nx/controls/ckeditor/ckeditor.js","ckeditor");
		}
	</script>
';
		}

		$CKEditor = new CKEditor('/nx/controls/ckeditor/');
		$config['language'] = 'fr';
		$config['enterMode'] = 'CKEDITOR.ENTER_P';
		$config['toolbar'] = array(
			  array('Cut','Copy','Paste','PasteText','PasteFromWord'),
			 // array('Bold','Italic','Underline','-','NumberedList','BulletedList','-','Outdent','Indent','-','-','Link','Unlink','JustifyLeft','JustifyCenter','JustifyRight','JustifyFull','FitWindow' ),			  
			array('Bold','Italic','Underline','-','NumberedList','BulletedList','-','Outdent','Indent','-','-','Link','Unlink'),
			array('JustifyLeft','JustifyCenter','JustifyRight','JustifyBlock')
		  );

		$jscode = $this->ck_replace($CKEditor,$fname,$config);

		$s.=<<<EOF
		<textarea id="_{$fname}_" name="{$fname}">{$value}</textarea>

		<script type="text/javascript">
		if (typeof ajax_require != "undefined")
		{
			ajax_onload(function(){
				try {
					{$jscode}
				}
				catch(e) {
					var editor = CKEDITOR.instances['_{$fname}_'];
					CKEDITOR.remove(editor);
					{$jscode}
				}
			},"ckeditor");
		}
		</script>
EOF;



		return $s; 
	}

	// based on CKEDITOR::replace function
	function ck_replace($ck, $id, $config)
	{
		$js = '';
		

//			$js .= "CKEDITOR.replace('".$id."', ".$ck->jsEncode($config).");";
		$js .= "CKEDITOR.replace('_".$id."_', ".json_encode($config).");";
		return $js;
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
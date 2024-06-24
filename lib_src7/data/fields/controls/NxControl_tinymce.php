<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
if (!isset($GLOBALS['form_tinymce']))
	$GLOBALS['form_tinymce']=false;

class NxControl_tinymce
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_tinymce'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/tinymce/js/tiny_mce/tiny_mce.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/tinymce/',NX_DOC_ROOT.'nx/controls/tinymce/');
		}
		$GLOBALS['form_tinymce']=true;
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
			$s.='
	<script type="text/javascript">
		// ajax_require("/nx/controls/tinymce/js/tiny_mce/tiny_mce.js","tinymcexx");
	</script>
';
		}
		
		$value=htmlentities($value);
		$skin_url=NX_BASE_SKINS_URL.$desc->getProperty('page.skin','default').'/';

		$s.=<<<EOF
		<textarea id="{$fname}" name="{$fname}" rows="15" cols="80" style="width: 80%">
		{$value}
		</textarea>

<script type="text/javascript">
		window.tinyMCEControlInit("{$skin_url}","{$fname}");
/*			ajax_onload(
				function(){

					window.tinyMCE.init(
						{
							mode : "exact",
							elements : "{$fname}",
							theme : "advanced"							
						});
				},"tinymcexx"); 
*/

/*
window.LazyLoad.loadOnce([
  "/nx/controls/tinymce/js/tiny_mce/tiny_mce.js"
], function(){

		);
	},null,true);
*/

</script></div>

<script type="text/javascript">
/*window.istinyloaded=true;
ajax_onload(
	function(){
		if (window.istinyloaded)
			return;
		window.istinyloaded=true;
alert("ok");
		tinyMCE.init(
			{
				mode : "textareas",
				elements : "{$fname}"
			}
		);
	},"tinymce");
*/
</script>
EOF;

		return $s;
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
					//'/(&nbsp;)+\n?/i',
					//'/(&nbsp;)+\n?/i'
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
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!isset($GLOBALS['form_colorpicker']))
	$GLOBALS['form_colorpicker']=false;

class NxControl_colorpicker
{
	function __construct()
	{
	}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_colorpicker'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/colorpicker/datepicker.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/colorpicker/',NX_DOC_ROOT.'nx/controls/colorpicker/');
		}
		$GLOBALS['form_colorpicker']=true;

		return false;
	}


	function toForm(&$value,&$f)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;

		$fname = $desc->getAlias();
		// add resources once
		if (!$this->init($desc))
		{
			$s.='
	<link rel="stylesheet" href="/nx/controls/colorpicker/colorpicker.css" /> 
	<script>
		ajax_require("/nx/controls/colorpicker/colorpicker.js","colorpicker");
	</script>
';
		}


		$s.=<<<EOF

		<input id="{$fname}" name="{$fname}" type="text form-control" name="{field.name}" value="{$value}" class="procolor" style="width:100px;background:{$value};"/>
		
		<script type="text/javascript">
			ajax_onload(function(){
				// if (!'{$fname}'.search(/[$]/))
					ProColor.prototype.attachButton(document.getElementById('{$fname}'),ProColor.prototype.options);
			},"colorpicker");
		</script>
EOF;

		return $s;
	}

	function readForm(&$recData,&$f)
	{
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=='')
			return null;
		else
			return nl2br(htmlspecialchars($value,ENT_QUOTES));
	}
}

?>
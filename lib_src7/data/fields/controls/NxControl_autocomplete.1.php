<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!isset($GLOBALS['form_autocomplete']))
	$GLOBALS['form_autocomplete']=false;

class NxControl_autocomplete
{
	function __construct()	{}
	function toHTML() {}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_autocomplete'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/autocomplete/autocomplete.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/autocomplete/',NX_DOC_ROOT.'nx/controls/autocomplete/');
		}		$GLOBALS['form_autocomplete']=true;

		return false;
	}
	
	function toForm($value,&$f)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();

		// get choice list array (as "value" => "label")
		if (trim($value) != '')
		{
			$list =& $desc->getList();
			$listArray =& $list->getListAsArray();
			if (isset($listArray[$value]))
				$inputVal = $listArray[$value];
			else
				$inputVal = '';
		}
		else
			$inputVal = '';

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $f->toHTML($opts).'<input type="hidden" name="'.$fname."\" value='".$inputVal."' />";

		$limit = $desc->getProperty('limit','10');
		$package=$desc->getProperty('nx.package','root');
		$ds = $desc->getProperty('ds');
		$dspack = explode('@',$ds);
		if (count($dspack)==1)
			$dspack[1]=$package;
		$ds=implode('@',$dspack);

		$spec = $ds . '|' . $desc->getProperty('ds_field_value','oid').'|'.$desc->getProperty('ds_format','').'|'.$desc->getProperty('ds_format_info','').'|'.$package;
		$spec=base64_encode($spec);
		$url = $desc->getProperty('url','/nx/controls/autocomplete/autocomplete.php?').'&amp;ds='.$spec.'&amp;json=true&amp;limit='.$limit.'&amp;';

		$s='';

		// add resources once
		if (!$this->init($desc))
		{
			$s.='
	<link rel="stylesheet" href="/nx/controls/autocomplete/autocomplete.css" />
	<script>
		if (typeof ajax_require != "undefined")
		{
			ajax_require("/nx/controls/autocomplete/autocomplete.js","autocomplete");
		}
	</script>
';
		}
		
		$s.=<<<EOH
<span class="ac_holder">
<input type="hidden" name="{$fname}" id="{$fname}" value="{$value}" /><input styleX="width: 200px" type="text" id="{$fname}_text" autocomplete="off" onFocus="javascript:
var options = {
script:'{$url}', 
varname:'input',
timeout:5000,
minchars:1,
json:true, 
shownoresults:true,
maxresults:16, 
callback: function (obj) { $('{$fname}').value=obj.id; $('{$fname}_json_info').update('you have selected: '+obj.id + ' ' + obj.value + ' (' + obj.info + ')'); }
};
var json;
if (typeof ajax_require != 'undefined') { ajax_onload(function(){json=new AutoComplete('{$fname}_text',options);},'autocomplete');}; return true;"  value="{$inputVal}" />

</span><span id='{$fname}_json_info'></span>
EOH;

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
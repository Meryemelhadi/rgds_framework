<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_select
{
	function __construct()
	{
	}
	
	function toHTML(&$f,$opts)
	{
		if ($opts)
			return $f->toFormat($opts);

		$desc=&$f->desc;
			
		if (isset($f->value) && $f->value!=='')
		{
			$list = $desc->getList();
			
			$vals = explode($desc->sep,$f->value);
			if (count($vals)>1)
			{
				$sep = $desc->getProperty("sep",'<br>',false);
				$vals = explode("|",$f->value);
				foreach($vals as $v)
					$res[]= $list->getChoice($v);
					
				return implode($sep,$res);
			}
			else
				return $list->getChoice($f->value);
		}
		else
			return '';
	}
	
	function toForm($value,&$f)
	{
		$desc=&$f->desc;
		// $fname = $desc->name;
		$fname = $f->getFormName();

		if ($desc->isRequired())
			$att = ' required="required" ';
		else
			$att = '';

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$value."\" />";

		if (!$desc->isEdit())
			return $this->toHTML($f,'')."<input type=\"hidden\" name=\"".$fname."\" value=\"".$value."\" />";			

		if($sources = $desc->getProperty("onchange_sources",null,false))
		{
			$recPrefix = $f->record->getPrefix();
			$asources = explode(",",$sources);
			$record = $f->record;
			foreach($asources as $source)
			{
				if($fs=$record->$source)
				{
					$_REQUEST[$source]=$fs->object;
				}
				else {
					$source2 = $recPrefix.$source;
					if($fs=$record->$source2)
						$_REQUEST[$source]=$fs->object;
				}
			}

			// get choice list array (as "value" => "label")
			$list = $desc->getList($fname,true);
		}
		else
			// get choice list array (as "value" => "label")
			$list = $desc->getList();
        
		$listArray =& $list->getListAsArray();
		$prompt=$desc->getPrompt();
		$prompt_value=$desc->getProperty('prompt_value','?',false);

		$style = $desc->getStyle("select","choice");
		$isMultiple=$desc->getProperty("multiple",false,false);
		
		$sel = array();
		if (isset($value))
		{
			if(!$desc->sep)
			    $desc->sep='|';

			$selected_options = explode($desc->sep,$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}
		
		if ($isMultiple)
		{
			$isMultiple = " multiple=\"true\"";
			$fname.='[]';
		}
		else
			$isMultiple = "";

		if($targets = $desc->getProperty("onchange_target"))
		{
			$atarget = explode(",",$targets);
			$onchange="window.onchange_$fname(this);";

			$script = <<<EOH
<script>
window.onchange_$fname = function (e) {
	var value = e.value;
	var form = e.form;
	var parameters={};
	parameters[e.name]=value;
EOH;


			foreach($atarget as $target) 
			{
				// get other field and prepare descriptor for ajax calls
				$ftarget = $f->record->$target;
				$tdesc = $ftarget->desc;
				$limit = $tdesc->getProperty('limit','10');
				$package=$tdesc->getProperty('nx.package','root');
				$ds = $tdesc->getProperty('ds');
				$dspack = explode('@',$ds);
				if (count($dspack)==1)
					$dspack[1]=$package;
				$ds=implode('@',$dspack);

				$tprompt=$tdesc->getPrompt();
				$tprompt_value=$tdesc->getProperty('prompt_value','?',false);

				$targetFname = $ftarget->getAlias();
				$suid=isset($_GET['suid'])?$_GET['suid']:'';

				$spec = $ds . '|' . $tdesc->getProperty('ds_field_value','oid').'|'.$tdesc->getProperty('ds_format','').'|'.$tdesc->getProperty('ds_format_info','').'|'.$package."|$tprompt|$tprompt_value";
				$spec=base64_encode($spec);
				$url = $desc->getProperty('url','/nx/controls/pm/autocomplete.php?').'&amp;ds='.$spec.'&amp;json=true&amp;limit='.$limit.'&amp;suid='.$suid;
				
				$script .= <<<EOH

	var url_{$targetFname} = '$url';
	// send request to the server
	new Ajax.Request(url_{$targetFname}.replace(/&amp;/g,"&"),
	{
		method: 'post',
		parameters: parameters,
		onSuccess:function(response, param) 
		{
			// update field
			try
			{
				var res = response.responseText;
				var targetFormField = '$targetFname';
				if(typeof form.elements[targetFormField] != 'undefined')
					form.elements[targetFormField].innerHTML = res;
			}
			catch (ex) {
				alert(ex);
			}
		}
	});
EOH;

			}

			$script .= <<<EOH
}
</script>
EOH;


		}
		else {
			$onchange = str_replace("'","\\'",$desc->getProperty("onchange"));
			$script='';
		}

		if($onchange)
			$onchange=' onchange="'.$onchange.'" ';

		$res = "{$script}<select $att class=\"select form-control\" ".$isMultiple." name=\"".$fname."\" ".$onchange.$style.">";
		if ($prompt	&& !isset($listArray[$prompt_value]))
		{
			$res .= "<option value=\"$prompt_value\">".$prompt."</option>";
		}
	
		// search empty prompt
		if($prompt=$desc->getPrompt_searchEmpty())
		{
			$prompt_value=$desc->getProperty('prompt_search_empty_value','?!',false);
			$res .= "<option value=\"$prompt_value\">".$prompt."</option>";
		}

		foreach (array_keys($listArray) as $k) 
		{
			$lab = &$listArray[$k];
			$res .= "<option value=\"$k\" ". (isset($sel[$k]) ? "selected=\"selected\"" : "").">".$lab."</option>";
		}
		return $res."</select>";
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]))
			return null;
			
		$values = $recData[$fname];

		// is it a JSON from front / APÏ ?
		if(is_object($values) && isset($values->value))
			$values = (string)$values->value;

		if (is_array($values))
			$value = implode($values,$desc->sep);
		else
			$value = $values;
			
		return $value;
	}
}

?>

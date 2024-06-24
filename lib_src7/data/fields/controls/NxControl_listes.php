<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_listes
{
	function __construct()
	{
	}
	
	function toHTML(&$f,$opts)
	{
		if ($opts)
			return $f->toFormat($opts);

		$desc=&$f->desc;
		if (isset($f->value))
		{
			$list =& $desc->getList();
			
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
		return '';
	}
	
	function toForm($value,&$f)
	{
		global $NX_CHARSET;
		$desc=&$f->desc;		

		// get choice list array (as "value" => "label")
		$list =& $desc->getList();
		$listArray =& $list->getListAsArray();
		$prompt=$desc->getPrompt();
		$prompt_value=$desc->getProperty('prompt_value','?',false);

		$style = $desc->getStyle("select","choice");
		$isMultiple=$desc->getProperty("multiple",false,false);
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		$sel = array();
		if (isset($value))
		{
			$value=html_entity_decode($value,ENT_QUOTES,$NX_CHARSET);
			$selected_options = explode($desc->sep,$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
				$sel[preg_replace('/'.chr(146).'/','&rsquo;',htmlspecialchars($option,ENT_QUOTES,$NX_CHARSET))]=true;
				$sel[html_entity_decode($option,ENT_QUOTES,$NX_CHARSET)]=true;
			}
		}
		
		$msg=$desc->getString('Please select an option',null,'form_fields');
		$selection=$desc->getString('Selected values',null,'form_fields');
		$others=$desc->getString('Values you can add',null,'form_fields');
		$fun="_nxOption_$fname";
		$script = <<<EOH
<script type="text/javascript">
	window.add$fun = function(fname,event){
		var objS=document.getElementById(fname+'_2'), indexS=objS.options.selectedIndex;
		if(indexS<0) return alert("Please select an option");
		var objD=document.getElementById(fname+'_1');
		objD.options[objD.options.length]=new Option(objS.options[indexS].text,objS.options[indexS].value);

		var objRef=document.getElementById(fname);
		var len=objRef.options.length;
		objRef.options[len]=new Option(objS.options[indexS].text,objS.options[indexS].value);
		objRef.options[len].selected=true;

		objS.options[indexS]=null;

		event.cancelBubble=true;
		event.returnValue=false;
		return false;
		}
	window.remove$fun = function (fname,event){
		var objS=document.getElementById(fname+'_1'), indexS=objS.options.selectedIndex;
		if(indexS<0) return alert("Please select an option");
		var objD=document.getElementById(fname+'_2');
		objD.options[objD.options.length]=new Option(objS.options[indexS].text,objS.options[indexS].value);

		var objRef=document.getElementById(fname);
		var len=objRef.options.length;
		objRef.options[indexS]=null;
		
		objS.options[indexS]=null;

		event.cancelBubble=true;
		event.returnValue=false;
		return false;
		}

</script>
EOH;

		if ($isMultiple)
			$isMultiple = " multiple=\"true\"";
		else
			$isMultiple = "";

		$stylelayout = $desc->getProperty("style","height:150px;width:200px;");
		
		$list=$list1=$list2='';
		foreach (array_keys($listArray) as $k) 
		{
			// for compatibility with DB			
//			$option="<option value=\"$k\">".htmlentities($listArray[$k],ENT_COMPAT,$NX_CHARSET)."</option>";
			$option="<option value=\"$k\">".preg_replace('/&rsquo;/','\'',$listArray[$k])."</option>";
			if (isset($sel[$k]))
			{
				$list1 .= $option;
				$list .="<option value=\"$k\" selected=\"true\">".$listArray[$k]."</option>";
			}	
			else
			{
				$list2 .= $option;
			}
		}
		
		return <<<EOH
{$script}<table class="control_lists"><tr>
<td style="vertical-align:top">
$selection<br/>
<select class="select select_list" {$isMultiple} 
	name="{$fname}_1[]" id="{$fname}_1" style="$stylelayout;" ondoubleclick="return remove$fun('{$fname}',event)">
$list1</select>
</td>
<td style="vertical-align:middle;padding-top:40px;">
<input type="button" class="button_select_list" onclick="return remove{$fun}('{$fname}',event)" value="-->"><br/>
<input type="button" class="button_select_list" onclick="return add{$fun}('{$fname}',event)" value="<--">
</td>
<td style="vertical-align:top">
$others<br/>
<select class="select select_list" {$isMultiple} 
	name="{$fname}_2[]" id="{$fname}_2" style="$stylelayout" ondoubleclick="return add$fun('{$fname}',event)">
$list2
</select><select class="select" {$isMultiple} 
	name="{$fname}[]" id="{$fname}" style="visibility:hidden;display:none;">$list</select>
</td></tr></table>
EOH;
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]))
			return null;
			
		$values = $recData[$fname];
		if (is_array($values))
			$value = implode($values,$desc->sep);
		else
			$value = $values;
			
		return $value;
	}
}

?>
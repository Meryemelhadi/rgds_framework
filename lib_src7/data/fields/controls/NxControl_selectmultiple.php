<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 

source: http://livepipe.net/control/selectmultiple
*/

if (!isset($GLOBALS['form_selectmultiple']))
	$GLOBALS['form_selectmultiple']=false;

class NxControl_selectmultiple
{
	function __construct()
	{
	}

	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_selectmultiple'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/livepipe/selectmultiple.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/livepipe/',NX_DOC_ROOT.'nx/controls/livepipe');
		}
		$GLOBALS['form_selectmultiple']=true;

		return false;
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
		$desc=&$f->desc;

		// get choice list array (as "value" => "label")
		$list =& $desc->getList();
		$listArray =& $list->getListAsArray();
		$prompt=$desc->getPrompt();
		$prompt_value=$desc->getProperty('prompt_value','?',false);

		$isMultiple=$desc->getProperty("multiple",false,false);
		$fname = $f->getAlias();
		$flabel=$f->getLabel();

		$sel = array();
		$multV=false;
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			$multV=count($selected_options)>1;
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}
		
		$opts='';
		$opts_tr='';
		$even_odd='odd';
		foreach (array_keys($listArray) as $k) 
		{
			$lab = $listArray[$k];
			if ($multV)
			{
				if (isset($sel[$k]))
				{
					$selK.=$k.$desc->sep;
					$selV.=$lab.',';
				}
				$opts .= "<option value=\"$k\">".$lab.'</option>';
			}
			else
				$opts .= "<option value=\"$k\" ". (isset($sel[$k]) ? "selected=\"selected\"" : "").">".$lab."</option>";

			$opts_tr.=<<<EOH
            <tr class="{$even_odd}">  
                <td class="select_multiple_name">$lab</td>  
                <td class="select_multiple_checkbox"><input type="checkbox" value="$k"/></td>  
            </tr>  
EOH;
			if ($even_odd=='odd')
				$even_odd='even';
			else
				$even_odd='odd';
		}

		$res = '<select class="select" id="'.$fname.'" name="'.$fname."\" >";
		if ($prompt	&& !isset($listArray[$prompt_value]))
		{
			$res .= "<option value=\"$prompt_value\">".$prompt."</option>";
		}

		$res.=$opts;

		if ($multV)
		{
			$selK=trim($selK,$desc->sep);
			$selV=trim($selV,',');
			$res .= "<option value=\"$selK\" selected=\"selected\">".$selV."</option>";
			$optValJS="value:'$selK',";
		}
		else
			$optValJS='';

		$res.='</select>';
		
		$s='';
		if (!$this->init($desc))
		{
			$s.='
	<link rel="stylesheet" href="/nx/controls/livepipe/selectmultiple.css" /> 
	<script type="text/javascript" language="javascript" src="/nx/controls/livepipe/livepipe.js"></script>
	<script type="text/javascript" language="javascript" src="/nx/controls/livepipe/selectmultiple.js"></script>
';
		}


		$s.=<<<EOH
<div id="{$fname}_container" style="position:relative">  
	{$res}
    <a href="" id="{$fname}_open" class="select_multiple_label">Select Multiple</a>  
    <div style="display:none;" id="{$fname}_options" class="select_multiple_container">  
        <div class="select_multiple_header">Select multiple $flabel</div>  
        <table cellspacing="0" cellpadding="0" class="select_multiple_table" width="100%">
			$opts_tr
        </table>  
        <div class="select_multiple_submit"><button type="button" value="Done" id="{$fname}_close">Done</button></div>  
    </div>  
</div>  

<script>
	//complex example, note how we need to pass in different CSS selectors because of the complex HTML structure  
var {$fname} = new Control.SelectMultiple('{$fname}','{$fname}_options',{  
    checkboxSelector: 'table.select_multiple_table tr td input[type=checkbox]',  
    nameSelector: 'table.select_multiple_table tr td.select_multiple_name',
	labelSeparator:',',valueSeparator:'{$desc->sep}',$optValJS
    afterChange: function(){  
        if({$fname} && {$fname}.setSelectedRows)  
            {$fname}.setSelectedRows();  
    }  
});  
  
//adds and removes highlighting from table rows  
{$fname}.setSelectedRows = function(){  
    this.checkboxes.each(function(checkbox){  
        var tr = $(checkbox.parentNode.parentNode);  
        tr.removeClassName('selected');  
        if(checkbox.checked)  
            tr.addClassName('selected');  
    });  
}.bind({$fname});  
{$fname}.checkboxes.each(function(checkbox){  
    $(checkbox).observe('click',{$fname}.setSelectedRows);  
});  
{$fname}.setSelectedRows();  
  
//link open and closing  
$('{$fname}_open').observe('click',function(event){  
    $(this.select).style.visibility = 'hidden';
    $(this.container).style.display = '';  
    Event.stop(event); 
    return false;  
}.bindAsEventListener({$fname}));  
$('{$fname}_close').observe('click',function(event){  
    $(this.select).style.visibility = 'visible';  
    $(this.container).style.display = 'none';
    Event.stop(event);  
    return false;  
}.bindAsEventListener({$fname}));  
</script>
EOH;
		return $s;
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
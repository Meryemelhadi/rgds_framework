<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_checkbox_boolean
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
		$desc=&$f->desc;
		$fname = $f->getAlias();
				
		$isChecked='';
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			if (isset($selected_options[0]) && $selected_options[0]==1)
				$isChecked=' checked="true" ';
		}
		else
			$value='';
			
		return '<span><input class="checkbox form-control" type="checkbox" name="'.$fname.'[]" value="'.$value.'"'.$isChecked.'></input></span>';
	}
	
	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]))
			return null;
			
		$values = $recData[$fname];
		if (is_array($values))
			$value = implode($values,$desc->sep);
		else
			$value = $values;
					
		// remap false value
		if (empty($value))
			$value='0'; // false
		else
			$value='1'; // true

		return $value;
	}
}

?>
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
		return '';
	}
	
	function toForm($value,&$f)
	{
		$desc=&$f->desc;

		// get choice list array (as "value" => "label")
		$fname = $f->getAlias();
		$prompt=$desc->getPrompt();
				
		$isChecked='';
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			if (isset($selected_options[0]) && ($selected_options[0]=='1' || $selected_options[0]=='yes') )
				$isChecked=' checked="true" ';
		}
		else
			$value='';
			
		return '<span><input class="checkbox form-control" type="checkbox" name="'.$fname.'" value="1"'.$isChecked.'/>&nbsp;'.$prompt.'</span>';
	}
	
	function readForm(&$recData,&$f)
	{
		// $fname = $desc->name;
		$fname = $f->getAlias();

		if (isset($recData[$fname]))
			$value = $recData[$fname];
		else
			$value = 0;

		if($value==='true' || $value=='1')
			$value=1;
		else
			$value=0;
					
		// remap false value
		if (empty($value))
			$value='0'; // false
		else
			$value='1'; // true

		return $value;
	}
}

?>
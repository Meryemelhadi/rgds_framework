<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_select_array
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
			$selected_options = explode($desc->sep,$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}

		$res=array();
		
		if ($isMultiple)
		{
			$isMultiple = " multiple=\"true\"";
			$fname.='[]';
		}
		else
			$isMultiple = "";

		$onchange = str_replace("'","\\'",$desc->getProperty("onchange"));
		if($onchange)
		{
			$res['onchange']=$onchange;
		}
		$res['name']=$fname;
		$res['type']='select';

		if ($prompt	&& !isset($listArray[$prompt_value]))
		{
			$res['prompt']=$prompt;
		}
		foreach (array_keys($listArray) as $k) 
		{
			$lab = &$listArray[$k];
			$f=array('value'=>$k,'label'=>$lab);
			if (isset($sel[$k]))
				$f['selected']=true;

			$res['options'][]=$f;
		}
		return $res;
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
			$value = implode($values,$this->desc->sep);
		else
			$value = $values;
			
		return $value;
	}
}

?>
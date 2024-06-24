<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_checkbox
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
		$fname = $f->getAlias();       
		$listname = $desc->getProperty('list',$fname.'_list');

		// get choice list array (as "value" => "label")
		$list = $desc->getList($listname);
		$listArray =& $list->getListAsArray();

		$style = $desc->getStyle("select","choice");
		$isMultiple=$desc->getProperty("multiple",false,false);
		if ($isMultiple)
			$itype='checkbox';
		else
			$itype='radio';
			
		$stylelayout = $desc->getProperty("style","");

		// nb columns
		$cols = $desc->getProperty('columns',1);
		$layout = $desc->getProperty('controlLayout',($cols>1)?'table':'inline');

		if(($evh = $desc->getProperty("onclick",''))!='')
		{
			$evh = str_replace("'","\\'",$evh);
			$evh=" onclick=\"$evh\" ";
		}
		
		$sel = array();
		if (isset($value))
		{
			$selected_options = explode($desc->sep,$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}
		
		switch ($layout)
		{
			case "rows":
				$res = "<table ".$style.">";
				$i=0;
				foreach (array_keys($listArray) as $k) 
				{
					$i++;
					$res .= '<tr><td><input class="'.$itype.' form-control" type="'.$itype.'" name="'.$fname.'[]" value="'.$k.'"'.
							(isset($sel[$k]) ? " checked=\"true\"" : "").$evh.'/></td><td class="checkbox_label"><label  for="'.$fname.$i.'"'.$evh.'>'.$listArray[$k].'</label></td></tr>';
				}
				return $res."</table>";
			case "columns":
				$res = "<table ".$style."><tr>";
				$i=0;
				foreach (array_keys($listArray) as $k) 
				{
					$i++;
					$res .= '<td><input class="'.$itype.' form-control" type="'.$itype.'" name="'.$fname.'[]" value="'.$k.'"'.
							(isset($sel[$k]) ? " checked=\"true\"" : "").$evh.'/></td><td class="checkbox_label"><label  for="'.$fname.$i.'">'.$listArray[$k].'</label></td>';
				}
				return $res."</tr></table>";
			case "tree":
				return $this->_toFormTree($fname,$listArray,$sel,$style,$itype);
			case "table":
				return $this->_toFormTable($fname,$listArray,$sel,$style,$itype,$cols);
			// case "inline":
			default:
				$res = "<span ".$style.">";
				$i=0;
				foreach (array_keys($listArray) as $k) 
				{
					$i++;
					$res .= '<input class="'.$itype.' form-control" type="'.$itype.'" id="'.$fname.$i.'" name="'.$fname.'[]" value="'.$k.'"'.
							(isset($sel[$k]) ? " checked=\"true\"" : "").$evh.'/><label  for="'.$fname.$i.'" class="checkbox_label">'.$listArray[$k].'</label>&nbsp;';
				}
				return $res."</span>";
		}			
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
	
	function _toFormTree($fname,&$listArray,&$sel,$style,$itype)
	{
		$count=0; 
		$baseLevel=0; // level of the first entry
		$curlev=0;
		$listLevel=1;
		$res = '<span class="choice_list_input" '.$style.'><ul style="list-style-type:none" class="choice_level_'.$curlev.'">';
		foreach (array_keys($listArray) as $k) 
		{
			$prevLev=$curlev;
			$label=$listArray[$k];
			$curlev=substr_count($k,NX_CATEGORY_SEP);
			$label=ltrim($label," \t+");
			if ($curlev > $prevLev)
			{
				if ($count>0)
				{
					// open list
					$res .='<ul style="list-style-type:none" class="level_'.$curlev.'">';
					$listLevel++;
				}
				else
					$baseLevel=$curlev;
			}
			else if ($curlev < $prevLev)
			{
				// close list
				$res .='</ul>';
				$listLevel--;
			}
			
			$res .= '<li class="choice_level_'.$curlev.'">';
			
			if (substr($k,0,1)!='!')
			{
				$res .= '<input class="'.$itype.' form-control" type="'.$itype.'" name="'.$fname.'[]" value="'.$k.'"'.
					(isset($sel[$k]) ? " checked=\"true\"" : "").'/>&nbsp;';
				$res .= '<span class="checkbox_label">'.$label.'</span></li>';
			}
			else
				$res .= '<span class="checkbox_label_header">'.$label.'</span></li>';
			$count++;
		}
		
		while(($listLevel--) > $baseLevel)
		{
			// close list
			$res .='</ul>';
		}
		
		return $res."</span>";
	}
	
	function _toFormTable($fname,$listArray,&$sel,$style,$itype,$cols=1)
	{
        $n = count($listArray) ;
        $per_column = floor(($n+$cols-1) / $cols) ;
		if($per_column <= 0)
			$listCols=array($listArray);
		else
			$listCols=array_chunk($listArray,$per_column,true);		

		$res='<table><tr>';
		foreach ($listCols as $listArray)
		{
			$i=0;
			$res .= '<td style="vertical-align:top"><table class="choice_list_input" id="'.$fname.'"><tr><td valign="top"><table>';
		
			foreach (array_keys($listArray) as $k) 
			{
				$label=$listArray[$k];
				$curlev=substr_count($k,NX_CATEGORY_SEP);
				$label=ltrim($label," \t+");
						
				if (substr($k,0,1)!='!')
				{					
					$res .= '<tr><td><input class="'.$itype.' form-control" type="'.$itype.'" name="'.$fname.'[]" value="'.$k.'"'.
						(isset($sel[$k]) ? " checked=\"true\"" : "").'/>&nbsp;';
					$res .= '<span class="checkbox_label">'.$label.'</span></td></tr>';
					$i++;
				}
				else
				{
					if ($i>0)
						$res.='</table></td><td valign="top"><table>';
									
					$res .= '<tr><th class="checkbox_label_header">'.$label.'</th></tr>';
					$i=1;
				}
				//$count++;
			}
			
			$res .="</table></td></tr></table></td>";
		}
		return $res.'</tr></table>';
	}
	
	function array_chunk ($a, $s, $p=false) {
		$r = Array();
		$ak = array_keys($a);
		$i = 0;
		$sc = 0;
		for ($x=0;$x<count($ak);$x++) {
		 if ($i == $s){$i = 0;$sc++;}
		 $k = ($p) ? $ak[$x] : $i;
		 $r[$sc][$k] = $a[$ak[$x]];
		 $i++;
		}
		return $r;
	}
}

?>
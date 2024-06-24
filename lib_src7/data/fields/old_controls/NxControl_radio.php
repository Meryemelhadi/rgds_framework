<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_radio
{
	function NxControl_radio()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm($value,&$desc)
	{
		// get choice list array (as "value" => "label")
		$list =& $desc->getList();
		$listArray =& $list->getListAsArray();

		$style = $desc->getStyle("select","choice");
		// $fname = $desc->name;
		$fname = $desc->getAlias();
		
		$layout = $desc->getProperty("controlLayout","inline");
		
		$sel = array();
		if (isset($value))
		{
			$selected_options = explode("|",$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}
		
		switch ($layout)
		{
			case "rows":
				$res = "<table ".$style.">";
				foreach (array_keys($listArray) as $k) 
				{
					$res .= '<tr><td><input class="radio" type="radio" name="'.$fname.'" value="'.$k.'"'.
							(isset($sel[$k]) ? " checked=\"true\"" : "").'/></td><td>'.$listArray[$k].'</td></tr>';
				}
				return $res."</table>";
			case "tree":
				return $this->_toFormTree($fname,$listArray,$sel,$style);
			// case "inline":
			default:
				$res = "<span ".$style.">";
				foreach (array_keys($listArray) as $k) 
				{
					$res .= '<input class="radio" type="radio" name="'.$fname.'" value="'.$k.'"'.
							(isset($sel[$k]) ? " checked=\"true\"" : "").'>'.$listArray[$k].'</input>&nbsp;';
				}
				return $res."</span>";
		}			
			
	}
	
	function readForm(&$recData,&$desc)
	{
//		$fname = $desc->name;
		$fname = $desc->getAlias();
		
		
		if (isset($recData[$fname]))
			return $recData[$fname];
		else 
			return null;
	}
	
	function _toFormTree($fname,&$listArray,&$sel,$style)
	{
		$curlev=0;
		$listLevel=1;
		$res = '<span '.$style.'><ul style="list-style-type:none" class="'.$curlev.'">';
		foreach (array_keys($listArray) as $k) 
		{
			$prevLev=$curlev;
			$label=$listArray[$k];
			$curlev=substr_count($k,NX_CATEGORY_SEP);
			$label=ltrim($label," \t+");
			if ($curlev > $prevLev)
			{
				// open list
				$res .='<ul style="list-style-type:none" class="'.$curlev.'">';
				$listLevel++;
			}
			else if ($curlev < $prevLev)
			{
				// close list
				$res .='</ul>';
				$listLevel--;
			}
			
			$res .= '<li class="'.$curlev.'"><input type="radio" name="'.$fname.'" value="'.$k.'"'.
					(isset($sel[$k]) ? " checked=\"true\"" : "").'/>&nbsp;'.$label.'</li></li>';
		}
		
		while(($listLevel--) > 0)
		{
			// close list
			$res .='</ul>';
		}
		
		return $res."</span>";
	}	
}

?>
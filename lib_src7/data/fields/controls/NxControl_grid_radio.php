<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_grid_radio
{
	function __construct()
	{
	}
	
	function toHTML(&$f,$opts)
	{
		if ($opts)
			return $f->toFormat($opts);

		$desc=&$f->desc;
			
		$res='';
		$values = array();
		if (isset($f->value))
		{
			$vals = explode($desc->sep,$f->value);
			if (count($vals)>1)
			{
				$values = $vals;
			}
			else
				$values[] = $f->value;
		}
		
		$list = $desc->getList();
		$listArray =& $list->getListAsArray();
		foreach (array_keys($listArray) as $k) 
		{
			$className = in_array($k, $values)?'checked-cell':'unchecked-cell';	
			$res .= '<td style="width:10% !important" class="form-cell cell-radio-type cell-radio-ctrl '.$className.'"><div class="simple-wrapper"><div class="field-radio"></div></div></td>';
		}		
		
		return $res;
	}
	
	function toForm($value,&$f)
	{
		$desc=&$f->desc;
		
		// get choice list array (as "value" => "label")
		$list = $desc->getList();
		$listArray =& $list->getListAsArray();

		$style = $desc->getStyle("select","choice");
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		$layout = $desc->getProperty("controlLayout","inline");
		
		$sel = array();
		if (isset($value))
		{
			if (preg_match('/[:][:]/',$value) && count($listArray)>0)
			{
				$keys = array_keys($listArray);
				switch($value)
				{
					case "::first":
						$sel[$keys[0]] = true;
						break;
					case "::last":
						$sel[$keys[count($keys)-1]] = true;
						break;
					default:
						$index = preg_replace('/[:][:]/','',$value);
						if ((int)$index >= 0 && (int)$index < count($listArray))
							$sel[$keys[$index]] = true;
						break;
				}
			}
			else
			{
				$selected_options = explode("|",$value);
				foreach($selected_options as $option)
				{
					$sel[$option]=true;
				}
			}
		}
	
		$res = '';		
		foreach (array_keys($listArray) as $k) 
		{
			$cInput = '<input class="radio" type="radio" name="'.$fname.'" value="'.$k.'"'.(isset($sel[$k]) ? " checked=\"true\"" : "").'/>';
			$res .= '<td style="width:10% !important" class="form-cell cell-radio-type cell-radio-ctrl"><div class="simple-wrapper"><div class="field-radio">'.$cInput.'</div></div></td>';
		}
		return $res;
		
		/*
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
							(isset($sel[$k]) ? " checked=\"true\"" : "").'/>'.$listArray[$k].'&nbsp;';
				}
				return $res."</span>";
		}
		*/			
			
	}
	
	function readForm(&$recData,&$f)
	{
//		$fname = $desc->name;
		$fname = $f->getAlias();
		
		
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
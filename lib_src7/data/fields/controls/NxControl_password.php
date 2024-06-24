<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_password
{	
	function toHTML(&$value,&$f)
	{
		return $value;
	}
	
	function toForm(&$value,&$f)
	{
		$desc=&$f->desc;		
		$fname = $f->getAlias();

		$inputVal = '';

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$inputVal."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value=\"".$inputVal."\" />";

		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$rowsMaxBox= intval($desc->getProperty("rowsMaxBox",5));

		$multiLines = $desc->getProperty("multiLines","auto",false);
		$size=intval($desc->getProperty("size",$colsMaxBox));
		$maxLength = intval($desc->getProperty("maxLength",$size));
		$filterChars = $desc->getProperty("onInput",null);		
		if(($evh = $desc->getProperty("onclick",''))!='')
		{
			$evh=" onclick=\"$evh\" ";
		}

		if ($multiLines!==false && ($size >= (2 * $colsMaxBox)))
		{
			if ($filterChars != null)
				$filterChars = "";
			else
				$filterChars = " onkeypress=\"keypress_textarea(event,".$maxLength.");".$filterChars."\" onpaste=\"keypress_textarea(event,".$maxLength.");".$filterChars."\"";

			if ($desc->getProperty("autocomplete",true,false) != true)
				$filterChars.=' autocomplete="off" ';

			// size larger than 2 lines => textarea with checking of number of chars
			$nbRows =  $size / $colsMaxBox;
			if ($nbRows <= $rowsMaxBox)
				$rows = $nbRows;
			else
				$rows = $rowsMaxBox;

			$style = $desc->getStyle("input","textarea");
			$res = "<textarea class=\"text\" type=\"text\" name=\"".$fname."\"".$style.
				 " cols=\"".$colsMaxBox."\" rows=\"".$rows."\"".
				 $filterChars."$evh>".$inputVal."</textarea>";
		}
		else
		{
			if (!isset($filterChars))
				$filterChars = "";
			else
				$filterChars = " onkeypress=\"".$filterChars."\" onpaste=\"".$filterChars."\"";

			if (!$desc->getProperty("autocomplete",true,false))
				$filterChars.=' autocomplete="off" ';

			if ($size <= $colsMaxBox)
			{
				// small input box
				$style = $desc->getStyle("input","text.short");
				$res = "<input class=\"text\" type=\"text\" name=\"".$fname."\" size=\"$size\" maxlength=\"$maxLength\" ".$style." value=\"".$inputVal."\"".$filterChars."$evh/>";
			}
			else // if($multiLines == "no" || (int)$size < 2 * $colsMaxBox)
			{
				// input box larger than colMaxBox on one line
				$style = $desc->getStyle("input","text.long");
				$res = "<input class=\"text\" type=\"text\" name=\"".$fname."\" size=\"$colsMaxBox\" maxlength=\"$maxLength\" ".$style." value=\"".$inputVal."\"". $filterChars."$evh/>";
			}
		}

		return $res;
	}

	function readForm(&$recData,&$f)
	{
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=='')
			return null;
		else
		{
			return nl2br(htmlspecialchars($recData[$fname],ENT_QUOTES));
		}
	}
}

?>
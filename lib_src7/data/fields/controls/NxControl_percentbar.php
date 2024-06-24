<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_percentbar
{
	function __construct()
	{
	}
	
	function toHTML(&$value,&$f)
	{
		return '<div class="ctrl_percentbar_cont" style="width:100%"><div class="ctrl_percentbar" style="background-color:#DAF0D8;width:'.$value.'%;font-weight:bold;">'.$value.'%</div></div>';
	}
	
	function toForm(&$value,&$f)
	{
		$desc=&$f->desc;		
		$fname = $f->getAlias();

		$inputVal = eregi_replace("[<]br[ /]*[>]", "", $value);

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
			return nl2br(htmlspecialchars($value,ENT_QUOTES));
	}
}

?>
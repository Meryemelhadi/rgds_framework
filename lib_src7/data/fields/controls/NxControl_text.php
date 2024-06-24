<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_text
{
	function __construct()
	{
	}
	
	function toHTML($value,$f)
	{
		// try appling i8n string to value (useful for workflow states)
		if($f->desc->getProperty('i8n','false',false)=='true')
			$value = __($value);

		return $value;
	}
	
	function toForm(&$value,&$f)
	{
		$desc=&$f->desc;		
		$fname = $f->getAlias();

		if ($desc->isRequired())
			$att = ' required="required" ';
		else
			$att = '';

		$inputVal = preg_replace("%[<]br[ /]*[>]%", ' ', $value);
		$inputVal = preg_replace("%&lt;br[ /]*&gt;%", ' ', $inputVal);

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$inputVal."\" />";

		if (!$desc->isEdit())
			return $this->toHTML($inputVal,$f)."<input type=\"hidden\" name=\"".$fname."\" value=\"".$inputVal."\" />";

		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$rowsMaxBox= intval($desc->getProperty("rowsMaxBox",5));

		$multiLines = $desc->getProperty("multiLines","auto",false);
		$size=intval($desc->getProperty("size",100000,false));
		$maxLength = intval($desc->getProperty("maxLength",$size,false));
		$filterChars = $desc->getProperty("onInput",null,false);		
		if(($evh = $desc->getProperty("onclick",'',false))!='')
		{
			$evh=" onclick=\"$evh\" ";
		}

		if ($multiLines!==false && ($size >= (2 * $colsMaxBox)))
		{
			// TEXT AREA

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
			$res = "<textarea $att class=\"text noprint form-control\" type=\"text\" name=\"".$fname."\"".$style.
				 " cols=\"".$colsMaxBox."\" rows=\"".$rows."\"".
				 $filterChars."$evh>".$inputVal."</textarea><div class=\"text print\" style=\"display:none\">$inputVal</div>";
		}
		else
		{
			// INPUT LINE

			if (!isset($filterChars))
				$filterChars = "";
			else
				$filterChars = " onkeypress=\"".$filterChars."\" onpaste=\"".$filterChars."\"";

			$autocomplete = strtolower($desc->getProperty("autocomplete","",false));
			if($autocomplete==='0' || $autocomplete=='off' || $autocomplete=='false')
			{
				$autocomplete=' autocomplete="off"';
			}
			else
				$autocomplete='';

			$prompt=$desc->getPrompt();
			if($prompt)
			{
				$placeholder=" placeholder=\"$prompt\" ";
			}
			else
				$placeholder='';

			/*
			if(!$inputVal && $prompt)
			{
				$inputVal=$prompt;
				$focus="onfocus=\"if(this.value==this.defaultValue) this.value='';\"";
			}
			else
				$focus='';
			*/

			if ($size <= $colsMaxBox)
			{
				// small input box
				$style = $desc->getStyle("input","text.short");
				$res = "<input $att $placeholder class=\"short_input text  form-control\" type=\"text\" name=\"".$fname."\" size=\"$size\" maxlength=\"$maxLength\" ".$style." value=\"".$inputVal."\"".$filterChars."$evh $autocomplete/>";
			}
			else // if($multiLines == "no" || (int)$size < 2 * $colsMaxBox)
			{
				// input box larger than colMaxBox on one line
				$style = $desc->getStyle("input","text.long");
				$res = "<input $att $placeholder class=\"text  form-control\" type=\"text\" name=\"".$fname."\" size=\"$colsMaxBox\" maxlength=\"$maxLength\" ".$style." value=\"".$inputVal."\"". $filterChars."$evh $autocomplete/>";
			}
		}

		return $res;
	}

	function readForm(&$recData,&$f)
	{
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=='' || $value==$f->desc->getPrompt())
			return null;
		else
		{
			$s = nl2br(htmlspecialchars($recData[$fname],ENT_QUOTES));
			if (false && get_magic_quotes_gpc()) 
			{
				$s = str_replace('\\"','"',$s);
			} 
			return $s;
		}
	}
}

?>
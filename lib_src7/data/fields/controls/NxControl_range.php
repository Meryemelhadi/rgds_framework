<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_range
{
	function __construct()
	{
	}
	
	function toHTML(&$value,&$f)
	{
		return $value;
	}
	
	function toForm(&$value,&$f)
	{
		$desc=&$f->desc;		
		$fname = $f->getAlias();

		$inputVal = preg_replace("%[<]br[ /]*[>]%i", "", $value);

        /* split min|max */
		if(!$sep=$desc->sep)
			$sep='-';
	
        $arValues = explode($sep, $inputVal);    
        $minValue = $arValues[0];
        $maxValue = $arValues[1];        
        
        if($fname_min= $desc->getProperty("field_name_min"))
        {
            $f_min = $f->record->$fname_min;
        }
        else
        {
            $f_min = null;
            $fname_min = $fname.'_min';
        }

        if($fname_max= $desc->getProperty("field_name_max"))
        {
            $f_max = $f->record->$fname_max;
        }
        else
        {
            $f_max = null;
            $fname_max = $fname.'_max';
        }


		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$inputVal."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname_min."\" value=\"".$minValue."\" /><input type=\"hidden\" name=\"".$fname_max."\" value=\"".$maxValue."\" />";

		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		//$rowsMaxBox= intval($desc->getProperty("rowsMaxBox",5));

		$multiLines = $desc->getProperty("multiLines","auto",false);
		$size=intval($desc->getProperty("size",$colsMaxBox));
		$maxLength = intval($desc->getProperty("maxLength",$size));
		$filterChars = $desc->getProperty("onInput",null);		
		if(($evh = $desc->getProperty("onclick",''))!='')
		{
			$evh=" onclick=\"$evh\" ";
		}
				
		$prompt=$desc->getPrompt();
		$prompt2='Min '.$prompt;

		if (!isset($filterChars))
			$filterChars = "";
		else
			$filterChars = " onkeypress=\"".$filterChars."\" onpaste=\"".$filterChars."\"";
		
		$res = '<div class="nx_control_range clear-both">';
				
		$res .= '<div class="nx_control_range_min"><div class="ncr-wrapper"><div>';
        if($f_min)
		{
            $res .= $f_min->form;
		}
        else
        	$res .= "<input title=\"$prompt2\" placeholder=\"$prompt2\" class=\"text\" type=\"text\" name=\"".$fname_min."\" size=\"$size\" maxlength=\"$maxLength\" value=\"".$minValue."\"".$filterChars."$evh/>";
            
		$res .= '</div></div></div>';
		
		$res .= '<div class="nx_control_range_max"><div class="ncr-wrapper"><div>';
        if($f_max)
		{
			$res .= $f_max->form;

		}
        else
		    $res .= "<input  placeholder=\"Max\" class=\"text\" type=\"text\" name=\"".$fname_max."\" size=\"$size\" maxlength=\"$maxLength\" value=\"".$maxValue."\"".$filterChars."$evh/>";

		$res .= '</div></div></div>';
		
		$res .= '</div>';

		return $res;
	}

	function readForm(&$recData,&$f)
	{
		// $fname = $desc->name;
		$desc=&$f->desc;		
		$fname = $f->getAlias();
		$fname_min= $desc->getProperty("field_name_min",$fname.'_min');
		$fname_max= $desc->getProperty("field_name_max",$fname.'_max');

		$minValue = $f->getFieldData($recData,$fname_min,TC_NO_SELECTION);
		$maxValue = $f->getFieldData($recData,$fname_max,TC_NO_SELECTION);

		if(!$sep=$desc->sep)
			$sep='-';
		
		if( ( isset($minValue) && trim($minValue) != '' ) || ( isset($maxValue) && trim($maxValue) != '' ) ) {
			return nl2br(htmlspecialchars($minValue.$sep.$maxValue,ENT_QUOTES));
		}
		
		if( isset($recData[$fname]) && $recData[$fname] != ''  ) {
			return nl2br(htmlspecialchars($recData[$fname],ENT_QUOTES));
		}
			
		return null;
	}
}

?>
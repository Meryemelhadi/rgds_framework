<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'data/fields/NxDataField_datetime.php');

/** Data field: date */
class NxDataField_time extends NxDataField_datetime
{
	var $day;
	var $month;
	var $year;
	var $h,$m,$s;

	function toHTML($opts=null)
	{
		// NB. locale is set up at the page level
		if (checkdate((int)$this->month->value,(int)$this->day->value, (int)$this->year->value))
		{
			$t=mktime ($this->h->value,$this->mn->value,$this->s->value,$this->month->value,$this->day->value,$this->year->value);	
			
			if ($opts!=null)
				$dateFormat=$opts;
			else
				$dateFormat = $this->desc->getProperty("format","short");
				
			$df=$this->desc->getString("format_datetime_$dateFormat",null,'form_fields',$dateFormat);
			
			switch($df)
			{								
				case 'unix_s':
				case "timestamp":
					return $t;
				case 'unix_ms':
					return ''.$t.'000';
				/*case "long":
					$fmt = $this->desc->getProperty("dateTimeFormatLong","%A %d %B %Y, %Hh%Mm%S");
					return $this->strftime($fmt,$t);
				case "short":
					$fmt = $this->desc->getProperty("dateTimeFormatShort","%x %X");
					return $this->strftime($fmt,$t);*/
				default: // '%d %B %Y'
					return $this->strftime($df,$t);
			}										
		}
		else
		{
			return 'n/a';
		}
	}


	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return "";

		$falias=$this->getAlias();
			
		if (!$desc->isEdit() || $desc->isHidden())
		{
			$res = 
			"<input type=\"hidden\" name=\"".$falias."_day"."\" value=\"".$this->day->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_month"."\" value=\"".$this->month->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_year"."\" value=\"".$this->year->value."\"/>";
			"<input type=\"hidden\" name=\"".$falias."_h"."\" value=\"".$this->h->value."\"/>";
			"<input type=\"hidden\" name=\"".$falias."_mn"."\" value=\"".$this->m->value."\"/>";
			"<input type=\"hidden\" name=\"".$falias."_s"."\" value=\"".$this->s->value."\"/>";

			if (!$desc->isEdit() && !$desc->isHidden())
				$res .= $this->toHTML();
				
			return $res;
		}
			
		$style = $desc->getStyle("input","datetime");
	
		$res = "<div class=\"input_datetime\" nowrap=\"true\" ".$style.">";
		// $list =& $desc->getChoiceList("day-of-month");
		// $res = $list->getFormList($this->desc->name."_day",$this->day);
		$res .= $this->day->toForm();
		
		// $list =& $desc->getChoiceList("months");
		// $res .= $list->getFormList($this->desc->name."_month",$this->month);
		$res .= $this->month->toForm();

		$res .= 
		'<input class="text input_year form-control" id="input_year" type="text" name="'.$falias.'_year" size="4" maxlength="4" value="'.$this->year.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/> '.

		'<input class="text form-control" id="input_h" type="text input_year" name="'.$falias.'_h" size="2" maxlength="2" value="'.$this->h.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/>:'.
		'<input class="text form-control" id="input_m" type="text" name="'.$falias.'_m" size="2" maxlength="2" value="'.$this->m.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/>:'.
		'<input class="text form-control" id="input_s" type="text" name="'.$falias.'_s" size="2" maxlength="2" value="'.$this->s.'" onkeypress="keypress_number(event)" onpaste="keypress_number(event)"/></div>';
		return $res;
	}

}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxControl_datetime
{
	function __construct()	{}
	function toHTML() {}
		
	function toForm(&$f,$controlName)
	{
		$desc=&$f->desc;
		$falias=$fname = $f->getAlias();
	
		$falias=$f->getAlias();
			
		if (!$desc->isEdit() || $desc->isHidden())
		{
			$res = 
			"<input type=\"hidden\" name=\"".$falias."_day"."\" value=\"".$f->day->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_month"."\" value=\"".$f->month->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_year"."\" value=\"".$f->year->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_h"."\" value=\"".$f->h->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_mn"."\" value=\"".$f->mn->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_s"."\" value=\"".$f->s->value."\"/>";

			if (!$desc->isEdit() && !$desc->isHidden())
				$res .= $f->toHTML();
				
			return $res;
		}
					
		$style = $desc->getStyle("input","datetime");
	
		$res = "<span class=\"input_datetime\" nowrap=\"true\" ".$style.">";
		
		if ($controlName=='time')
		{
			$res .= 
			"<input type=\"hidden\" name=\"".$falias."_day"."\" value=\"".$f->day->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_month"."\" value=\"".$f->month->value."\"/>".
			"<input type=\"hidden\" name=\"".$falias."_year"."\" value=\"".$f->year->value."\"/>";			
		}
		else
		{
			$res .= $f->day->toForm();
			$res .= $f->month->toForm();
			$res .= $f->year->toForm();
		}
		
		$res .= $f->h->toForm();
		$res .= ' : '.$f->mn->toForm();

		if (strtolower($desc->getProperty("withSeconds","false",false))=="true")
			$res .= ' : '.$f->s->toForm();
		else
			$res .= "<input type=\"hidden\" name=\"".$falias."_seconds"."\" value=\"".$f->s->value."\"/>";

		return $res.'</span>';
	}

	function readForm(&$recData,&$f,$controlName)
	{
		$desc=&$f->desc;
		$falias=$fname = $f->getAlias();
		
		$f->day->value = $f->getFieldData($recData,$falias."_day",TC_NO_SELECTION);
		$f->month->value = $f->getFieldData($recData,$falias."_month",TC_NO_SELECTION);
		$f->year->value = $f->getFieldData($recData,$falias."_year",TC_NO_SELECTION);

		$f->h->value = $f->getFieldData($recData,$falias."_h",TC_NO_SELECTION);
		$f->mn->value = $f->getFieldData($recData,$falias."_mn",TC_NO_SELECTION);
		$f->s->value = $f->getFieldData($recData,$falias."_s",0);

		// check date existence
		if ($controlName!='time' && !checkdate((int)$f->month->value,(int)$f->day->value, (int)$f->year->value))
		{
			$errorCB=new NxErrorCB();
			$f->addError($errorCB, "invalid date", $fields);
			return false;
		}
		if ((int)$f->h->value>23 || ((int)$f->mn->value>60)||((int)$f->s->value>60))
		{
			$errorCB=new NxErrorCB();
			$f->addError($errorCB, "invalid time", $fields);
			return false;
		}

		return true;
	}
}

?>
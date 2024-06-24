<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_int_phone
{
	function __construct()
	{
	}
	
	function toHTML(&$value,&$f)
	{
        if(defined('NX_INT_PHONES') && !NX_INT_PHONES)
        {
		    $parts = $this->parseNumber($value);
            $value = $parts[1].$parts[2];
			$value = preg_replace('/\d{2}/', '$0 ', $parts[1].$parts[2]);
        }
		else
		{
			$value = preg_replace('/\d{2}/', '$0 ', $value);
		}

		return $value;
	}

	function toForm($value,&$f)
	{
		$desc=&$f->desc;
		
		//$fname = $desc->name;
		$fname = $f->getAlias();

		if ($desc->isRequired())
			$att = ' required="required" ';
		else
			$att = '';
		
		// $layout = $desc->getProperty("controlLayout","full");
		
		// split the value into: <int code>|<area code>|<local number>
		$parts = explode("|",$value);
		if (count($parts) != 3)
		{
			$parts = $this->parseNumber($value);
		}
						
		// display 3 entry fields as <fname>_int <fname>_area <fname>_num
		$res = '<span class="input_phone">';
		
		// international code
		$res .= $this->_toFormIntCode($parts[0],$f);

		// area
		$res .='&nbsp;(<input onkeypress="keypress_phonenumber(event)" class="text" id="phone_area" type="text" name="'.$desc->name."_area".'" value="'.$parts[1].'" size="3" />)';
		// local number
		$res .='&nbsp;<input onkeypress="keypress_phonenumber(event)"  class="other_text" id="phone_local_number" type="text" name="'.$desc->name."_num".'" value="'.$parts[2].'" '.$att.'/>';

		return $res."</span>";
	}

	function _toFormIntCode($value,&$f)
	{
		$desc=&$f->desc;
		
		// get choice list array (as "value" => "label")
		$listName = $desc->getProperty("list","int_phone",false);		
		$list = $desc->getChoiceList($listName);
		$listArray = $list->getListAsArray();

		$style = $desc->getStyle("select","choice");
		// $fname = $desc->name . "_intCode";
		$fname = $f->getAlias(). "_intCode";

		if ($value=="")
		{
			$value=$desc->getProperty("defaultPhoneCode","44");
		}
		$sel = array();
		$sel[$value]=true;
		
		$isMultiple = "";

		$res = "<select class=\"select\" id=\"phone_int_code\" ".$isMultiple." name=\"".$fname."\" ".$style.">";
		foreach (array_keys($listArray) as $k) 
		{
			$lab = &$listArray[$k];
			$res .= "<option value=\"$k\" ". (isset($sel[$k]) ? "selected=\"true\"" : "").">".$lab."</option>";
		}
		return $res."</select>";
	}
	
	function readForm(&$recData,&$f,&$errorCB)
	{
		$desc=&$f->desc;

		$errFields = null;
		$name = $f->getAlias();

		// read international code
		// $fname = $desc->name."_intCode";
		$fname = $name."_intCode";
		
		if (isset($recData[$fname]))
		{
			$code = "+".$recData[$fname];
		}
		else
		{
			$code = "";
			
			if ($desc->isRequired())
				$errFields = $desc->getString("international code");		
		}

		// read area code
		$fname = $name."_area";
		if (isset($recData[$fname]) && $recData[$fname]!="")
		{
			$value = " (".trim($recData[$fname]).")";
		}
		else
		{
			$value = "";
		}
		
		$fname = $name."_num";
		if (isset($recData[$fname]) && $recData[$fname]!="")
		{
			$value .= " ".str_replace(array('-',' ','_','.'),"",$recData[$fname]);
		}
		else
		{
			if ($desc->isRequired())
				$errFields = $desc->getString("local number");		
		}
				
		if ($value!="")
			$value = $code.$value;

		// check fields have been entered if requested
		if (isset($errFields))
		{
			$desc->addError($errorCB, "incomplete phone number", $errFields);
		}	
		else if (!$this->checkPhoneNumber($value))
		{
			$desc->addError($errorCB, "invalid phone number", $errFields);
		}
			
		return $value;
	}
		
	function checkPhoneNumber($num)
	{
		if (preg_match('|^([+]([0-9]+))?[ _\-]?([\(]?([0-9]*)[)/])?([ \-_\.0-9]*)$|',$num,$matches)>0)
			return true;
		else
			return false;
	}
	
	function parseNumber($num)
	{
		$parts=array();
		
		// try to recognise the form: +44 (208) 987 000
		if (preg_match('|^([+]([0-9]+))?[ _\-]?([\(]?([0-9]*)[)/])?([ \-_.0-9]*)$|',$num,$matches) > 0)
		{
			// match ok
			$parts[0]=$matches[2];
			$parts[1]=$matches[4];
			$parts[2]=$matches[5];
			
		}
		else if (preg_match('|^([+]([0-9]+))?[ _\-]?([\(]?([0-9]*)[)/])|',$num,$matches) > 0)
		{
			// match ok
			$parts[0]=$matches[2];
			$parts[1]=$matches[4];
			$parts[2]=substr($num,strlen($matches[0]));
			
		}
		else if (preg_match('|^([+]([0-9]+))|',$num,$matches) > 0)
		{
			// match ok
			$parts[0]=$matches[2];
			$parts[1]="";
			$parts[2]=substr($num,strlen($matches));
			
		}		
		else
		{
			// no match (tbd. other phone syntax ?)
			$parts[0]="";
			$parts[1]="";
			$parts[2]=$num;				
		}
		
		$parts[2]=str_replace(array('-',' ','_','.'),"",$parts[2]);
		return $parts;
	}
}

?>
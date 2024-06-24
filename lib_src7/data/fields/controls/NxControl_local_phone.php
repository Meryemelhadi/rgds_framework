<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_local_phone
{
	function __construct()
	{
	}
	
	function toHTML(&$value,&$f)
	{
		$value = preg_replace('/\d{2}/', '$0 ', $value);
		return $value;
	}

	function toForm($value,&$f)
	{
		$desc=&$f->desc;
		
		//$fname = $desc->name;
		$fname = $f->getAlias();
							
		// display 3 entry fields as <fname>_int <fname>_area <fname>_num
		$res = '<span class="input_phone">';
		
		// local number
		$res .='<input onkeypress="keypress_phonenumber(event)"  class="other_text" id="phone_local_number" type="text" name="'.$desc->name.'" value="'.$value.'"/>';

		return $res."</span>";
	}

	function readForm(&$recData,&$f,&$errorCB)
	{
		$desc=&$f->desc;
		$errFields = null;
		$name = $f->getAlias();
	
		$fname = $name;
		if (isset($recData[$fname]) && $recData[$fname]!="")
		{
			$value = str_replace(array('-',' ','_','.'),"",$recData[$fname]);
		}
		else
		{
			if ($desc->isRequired())
				$errFields = $desc->getString("local number");		
			$value='';
		}
				
		// check fields have been entered if requested
		if (isset($errFields))
		{
			$desc->addError($errorCB, "incomplete phone number", $errFields);
		}	
		else if (!$this->checkPhoneNumber($value))
		{
			$desc->addError($errorCB, "invalid phone number", $errFields);
			return $value;
		}
			
		return $value;
	}
		
	function checkPhoneNumber($num)
	{
		if (preg_match('|^[0-9\-. +()]*$|',$num,$matches)>0)
			return true;
		else
			return false;
	}
}

?>
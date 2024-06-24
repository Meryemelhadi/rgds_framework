<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'data/fields/NxDataField_text_choice.php');

/** Data field : integer list */
class NxDataField_integerList extends NxDataField_text_choice
{
	var $min;
	var $max;
	var $value;

	function __construct($desc,$withPadding=true)
	{
		$this->desc = $desc;
		$this->min = intval($desc->getProperty("min",0,false));
		$this->max = intval($desc->getProperty("max",10,false));
		$this->withPadding = $withPadding;
	}

	function setValue($deft)
	{
		if ($deft === null)
		{
			$this->value = TC_NO_SELECTION;
			return false; // not found
		}
		else
			$this->value = $deft;

		// if it's an int and in the range, then ok
		$v=intval($deft);
		if("$deft"=="$v" && !($v > $this->max || $v < $this->min))
			$this->value = $deft;
		else
			$this->value = TC_NO_SELECTION;
			
		return true;
	}

	function toHTML($opts=null)
	{
		if (isset($this->value))
			return $this->value;
		else
			return "";
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;
		$fname = $this->getAlias();

		if (!$desc->isInput())
			return "";
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$this->value."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value=\"".$this->value."\" />";			

		if (isset($desc->props["multipleChoices"]) && $desc->props["multipleChoices"]==true)
			$isMultiple = true;
		else
			$isMultiple = false;

		return $this->getFormList($fname,$desc->getStyle("select","choice"),$isMultiple,$desc->getProperty("prompt",TC_NO_SELECTION,true),$desc->getProperty("increment",1,false));
	}

	function getFormList($name,$style,$isMultiple,$prompt,$incr=1)
	{       
		$sel = array();
		if (isset($this->value))
		{
			// TODO : check the cast
			if(trim((string)$this->value)!=='')
				$selected_options = explode("|",(string)$this->value);
				if($selected_options) foreach($selected_options as $option)
				{
					$sel[$option]=true;
				}
		}

		if ($isMultiple)
			$isMultiple = " multiple=\"true\"";
		else
			$isMultiple = "";

		$sel2 = array();
		foreach($sel as $k=>$v)
		{
			$k2=ceil(($k/$incr))*$incr;

			if($k2<$this->min)
				$k2=$this->min;
			elseif($k2>$this->max)
				$k2=$this->max;

			$sel2[$k2]=true;
		}

		$res ="<select class=\"select form-control\" $isMultiple name=\"$name\" $style>";
		$res .= "<option value=\"?\">".$prompt."</option>";
		for($i = $this->min; $i <= $this->max ; $i+=$incr) 
		{
//			$label = $this->withPadding ? str_pad($i, 2, '0', STR_PAD_LEFT) : (string)$i;
			$label = str_pad($i, 2, '0', STR_PAD_LEFT);
			$res .= "<option value=\"$i\" ". (isset($sel2[$i]) ? "selected=\"selected\"" : "").">".$label."</option>";
		}
		return $res."</select>";
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		// TODO: check this in multi selection mode
		$this->value = $this->getFieldData($recData,$this->desc->name,"");
		if ($this->value === TC_NO_SELECTION) 
		{
			if ($this->desc->isRequired())
			{
				$this->addError($errorCB, "not selected");
				return false;
			}
			else
			{
				return true;
			}
		}

		// check if its an int
		$v=intval($this->value);
		if("$this->value"!="$v")
		{
			$this->addError($errorCB, "invalid number");
			return false;
		}
		else
		{
			if ($v > $this->max || $v < $this->min)
			{
				$this->addError($errorCB, "incorrect");
				return false;
			}
		}
		
		return true;
	}
}

?>
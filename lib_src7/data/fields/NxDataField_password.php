<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
/** Data field : text */
class NxDataField_password extends NxDataField_text
{
	var $value;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$this->value = '';
	}

	function toHTML($opts=null,$default='')
	{
		if ($this->desc->getProperty("masked",true,false) == true)
		{
			// $l = mb_strlen($this->value);
			$l = strlen($this->value);
			return str_pad("", $l,"*");
		}
		else
			return $this->value;
	}
	
	function toForm($opts=null) 
	{
		$desc = & $this->desc;
		$falias=$this->getAlias(); 

		if (!$desc->isInput())
			return "";
				
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$falias."\" value=\"".$this->value."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$falias."\" value=\"".$this->value."\" />";			

		if (!$this->desc->isInput())
			return "";
		
		return $this->_toForm(preg_replace("%[<]br[ /]*[>]%", '', $this->value));
	}
	
	function _toForm($inputVal)
	{
		$desc = & $this->desc;
		$falias=$this->getAlias(); 

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$falias."\" value=\"".$inputVal."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$falias."\" value=\"".$inputVal."\" />";

		$colsMaxBox= intval($this->desc->getProperty("colsMaxBox",30));
		$size=intval($this->desc->getProperty("size",$colsMaxBox));
		$maxLength = intval($this->desc->getProperty("maxLength",$size));
		$filterChars = $this->desc->getProperty("onInput",null);

		if (!isset($filterChars))
			$filterChars = '';
		else
			$filterChars = " onkeypress=\"".$filterChars."\" onpaste=\"".$filterChars."\"";

		// mask pwd?
		if ($this->desc->getProperty("masked",true,false) == true)
			$type_input='password';
		else
			$type_input='text';

		if (!$this->desc->getProperty("autocomplete",true,false))
        {
			$filterChars.=' autocomplete="off" ';
        }

		if ($size <= $colsMaxBox)
		{
			// small input box
			$style = $this->desc->getStyle("input","text.short");
			$res = "<input class=\"password form-control\" type=\"{$type_input}\" name=\"".$falias."\" size=\"$size\" maxlength=\"$maxLength\" ".$style." value=\"".$inputVal."\"".$filterChars."/>";
		}
		else
		{
			// input box larger than colMaxBox on one line
			$style = $this->desc->getStyle("input","text.long");
			$res = "<input class=\"password form-control\" type=\"{$type_input}\" name=\"".$falias."\" size=\"$colsMaxBox\" maxlength=\"$maxLength\" ".$style." value=\"".$inputVal."\"". $filterChars."/>";
		}

		return $res;
	}
}

?>
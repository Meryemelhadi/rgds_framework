<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
/** Data field : text */
class NxDataField_float extends NxDataField_text
{
	function readFromDB($recData)
	{
		$v = $this->getFieldData($recData,$this->getFName(),'');
		$v = (float)$v;
		$v = (string)$v;
		$this->value = $v;
	}

	function toDB($opts=null) 
	{
		return "'".str_replace(',', '.',''.$this->value)."'";			
	}
}

?>
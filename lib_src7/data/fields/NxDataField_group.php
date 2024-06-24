<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : choice list */
class NxDataField_group extends NxData_Field
{
	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}

	function clear()
	{
	}

	function setValue($deft)
	{
		return true;
	}
	
	function toUrlParam()		{
		return '';
	}

	function & toObject($opts=null)
	{ 
		return ''; 
	}
	
	function readFromStore(&$v){
	}
	
	function readFromDB($recData)
	{
	}

	function toDB($opts=null)
	{
		return '';
	}

	function toString($opts=null)
	{
		return '';
	}
	
	/* apply a format on the choice list. Format is list of the form:
			array('empty'=>'%s:empty','selected'=>'%s:ok') : display all items
		or
			array('first'=>'[%s','last'=>'%s]','others'=>',%s','unique'=>'[%s]') */
	function toFormat($opts='')
	{
		return '';
	}
	
	function toHTML($opts=null)
	{
		return '';
	}

	function toForm($opts=null) 
	{
		return '';
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		return true;
	}
	
	function is_transient() {
		return true;
	}
	
	function is_pseudo() {
		return true;
	}
}

?>
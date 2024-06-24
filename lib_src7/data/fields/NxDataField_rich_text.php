<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'data/fields/NxDataField_html.php');

/** Data field : rich edit text */
class NxDataField_rich_text extends NxDataField_html
{
	function __construct(&$desc)
	{
		parent::__construct($desc);
	}
	
	function onSubmitCheck($frm)
	{
		$controlName=$this->desc->getProperty('control','');
		if ($controlName=='fck_editor')
			return "";
			
		return NxData_Field::onSubmitCheck($frm);
	}
	
	function getControl() 
	{
		$controlName=$this->desc->getProperty('control');
		if (!$controlName || $controlName=='rich_text')
		{
			if (defined('NX_EDITOR'))
				$controlName=NX_EDITOR;
			else
				$controlName='fck_editor';			
		}
		
		switch($controlName) {
			case 'rte':
				if (NX_DEVICE.NX_DEVICE_VER=='IE55')
					$control = $this->desc->getControl('rich_text');
				else
					$control = $this->desc->getControl('fck_editor');
				break;
			case 'fck_editor':
					$control = $this->desc->getControl('fck_editor');
				break;
			case 'rich_text_default':
					$control = $this->desc->getControl('rich_text_default');
				break;
			default:
				$control = $this->desc->getControl($controlName);
				break;
		}		
		return $control;
	}
	
	function toForm($opts=null) 
	{
		$desc = $this->desc;

		if (!$desc->isInput())
			return "";
			
		// which editor
		$control=$this->getControl($this->desc);
		return $control->toForm($this->value,$this);
	}	

	function _readFromForm(&$recData,&$errorCB)
	{
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = $this->desc;

		// which editor
		$control=$this->getControl($this->desc);
			
		$this->value = $control->readForm($recData,$this);

		if (($this->value == null) && $desc->isRequired())
		{
			$this->addError($errorCB, "empty");
			return false;
		}
		
		return true;
	}
}

?>
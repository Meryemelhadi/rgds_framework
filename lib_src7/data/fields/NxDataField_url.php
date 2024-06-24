<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : html code */
class NxDataField_url extends NxDataField_text
{
	function __construct(&$desc)
	{
		parent::__construct($desc);
	}
	
	function toHTML($opts=null,$default='&nbsp;')
	{
		if (!isset($this->value))
		{
			// nxError('undefined value for field '.$this->desc->getName().' type:'.get_class($this));
			return $default;
		}

		$target='target="'.$this->desc->getProperty("target","_top").'"';
		
		if ($this->value !== '' || $this->value === 0)
		{
			if (preg_match('/(https?|ftp)/',$this->value))
				return "<a $target href=\"{$this->value}\">{$this->value}</a>";
			else
				return "<a $target href=\"http://{$this->value}\">{$this->value}</a>";
		}
		else
			return $default;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : html code */
class NxDataField_params extends NxDataField_text
{
	function __construct(&$desc)
	{
		parent::__construct($desc);
	}
	
	function readFromDB($recData)
	{
		// get value
		$this->value=$this->getFieldData($recData,$this->getFName(),null);
		
		// parse value a set of parameters
		$this->info = $this->unserialise(preg_replace('/&amp;/','&',$this->value));
	}
	
	function toHTML($opts=null,$box=null)
	{
		if ($opts=='all')
		{
			$s='';
			foreach ($this->info as $p=>$v)
			{
				$s .= "<dl><dt>$p</dt><dd>$v</dd></dl>\n";
			}
			return $s;
		}
		
		if (isset($this->info[$opts]))
		{
			return $this->info[$opts];
		}
		else
			return '';
	}

	function toString($opts=null,$box=null)
	{
		if ($opts)
		{
			if (isset($this->info[$opts]))
			{
				return $this->info[$opts];
			}
		}
		
		$s='';
		foreach ($this->info as $p=>$v)
		{
			$s .= "- $p :$v\n";
		}
		return $s;
	}
}

?>
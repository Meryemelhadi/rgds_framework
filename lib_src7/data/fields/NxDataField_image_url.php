<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : image url */
class NxDataField_image_url extends NxDataField_text
{
	function __construct(&$desc)
	{
		parent::__construct($desc);
	}
	
	function toHTML($opts=null)
	{
		switch($opts)
		{
			case 'url':
				return $this->value;
			case 'js':
				return 'url:"'.$this->value.'"';
			default:
				// optimisation: store url in db ?
				if ($this->value!='')
					return "<img border=\"0\" src=\"".$this->value."\">";
				else
					return "&nbsp;";
		}
	}	
}

?>
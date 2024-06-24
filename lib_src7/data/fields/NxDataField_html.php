<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : html code */
class NxDataField_html extends NxDataField_text
{
	function __construct(&$desc)
	{
		parent::__construct($desc);
	}
	
	function toForm($opts=null) 
	{
		if (!$this->desc->isInput())
			return "";
		
		return $this->_toForm(str_replace("\"", "&quot;", $this->value));
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$this->value = 
//				ereg_replace("\"","&quot;",
					str_replace("'","&#39;",
						preg_replace("%[<]\?%","&lt;&#3f;", 
							str_replace( "<script","&lt;script",
								$this->getFieldData($recData,$this->desc->name,"")
							)
						)
//					)
				);

		if (($this->value == "") && $this->desc->isRequired())
		{
			$this->addError($errorCB, "empty");
			return false;
		}
		
		return true;
	}
}

?>
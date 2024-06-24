<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB."NxData_Output.inc");

class NxData_OutputForm extends NxData_Output
{
	function __construct(&$recDesc, $method)
	{
		parent::__construct($recDesc,"form");
		$this->$method = $method;
	}

	function writeHeader()
	{
		echo "<form action=\"". $HTTP_SERVER_VARS['PHP_SELF'] ." method=\"".$this->method."\">";

		$fileSize = 0;
		$fdescs = & $this->recDesc->fdescs;
		foreach(array_keys($fdescs) as $fname)
		{
			$fdesc = & $fdescs[$fname];
			if (isset($fdesc->props["type"]) && $fdesc->props["type"] == "file")
			{
				$fileSize += (int)$fdesc->getProperty("maxFileUploadSize", 1000000);
			}
		}
		if ($fileSize > 0)
		{
			echo "<input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"".$fileSize."\" />";
		}
	}
	
	function writeFooter()
	{
		// todo: get the actual button text or image from recDesc
		// connect to javascript form checker
		echo "<input type=\"submit\" value=\"submit form\"/>";
		echo "</form>";
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
/** default error callback */
class NxApp_ErrorCB
{
	function __construct() {}
	function error($mod,$msg)
	{
		echo("Error in module [".$mod."]:".$msg);
	}
}
?>

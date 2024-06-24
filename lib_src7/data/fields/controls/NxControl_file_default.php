<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_file_default
{
	function toHTML($field,$value,$opts)
	{
		$desc=&$field->desc;
		$fname = $field->getAlias();
		
		// optimisation: store url in db ?
		if (isset($field->info['n']))
		{
			$s='<a target="_blank" href="'.$field->_getUrl().'">'.$field->toTitle().'</a>';
			$size=$field->toSize();
			if ($size!=='')
			{		
				$s.=" <span id=\"file_size\">($size)</span>";
			}
			return $s;
		}
		else
			return "&nbsp;";
	}
}

?>
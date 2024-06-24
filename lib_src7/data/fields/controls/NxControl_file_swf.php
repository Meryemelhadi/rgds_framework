<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_file_swf
{
	function toHTML($f,$value,$opts)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
		// optimisation: store url in db ?
		if (isset($f->info['n']))
		{
			$s="<a target=\"_blank\" href=\"".$f->_getUrl($box)."\">{$f->info['n']}</a>";
			$size=$f->toSize();
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
<?php
/* tag lib only used for passing parameters to xo or xsl objects */
class taglib_nxfrontier_xo_config extends NxTagLib
{
	// do not execute template at parsing time
	function _tag_default()
	{
		return NX_TL_IGNORE_CONTENT;
	}
	
	function tag_default(&$node,$content,$level,$pad,$ns,$op)
	{
		// parse attributes
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			$av=$this->parseVal($av,false);
			$node->setAttribute($an,$av);
		}

		return $content;
	}
}

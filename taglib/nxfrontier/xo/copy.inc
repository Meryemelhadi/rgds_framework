<?php
/* tag lib only used for passing parameters to xo or xsl objects */
class taglib_nxfrontier_xo_copy extends NxTagLib
{
	function tag_default(&$node,$content,$level,$pad,$ns,$op)
	{	
		if (!$this->getAttribute($name,$node,'element',null))
			// $name=$op;
			$name=$node->tagName;
				
		if ($this->getAttribute($uri,$node,'uri',null))
			$uri=" xmlns=\"{$uri}\"";
		else
			$uri='';	
			
		$atts='';
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($an=='uri'|$an=='_pmethod')
				continue;
				
			$av=$this->parseVal($av,false);
				
			$atts .=" $an=\"$av\"";
		}
		
		if ($content!='')
			return "<{$name}{$atts}{$uri}>$content</$name>";
		else
			return "<{$name}{$atts}{$uri}/>";
	}
}

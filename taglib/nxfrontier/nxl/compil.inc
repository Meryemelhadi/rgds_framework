<?php

// core library for nxl language
class taglib_nxfrontier_nxl_compil extends NxTagLib
{	
	function output($node,$content,$level,$pad)
	{
		switch ($node->getAttribute('method','xml2text'))
		{
			case 'xml2text':
				$src=nl2br(htmlspecialchars($content));
				break;
			case 'php':
				$src='<?php '.$content.'?>';
				break;
			default:
				$src=$content;
				break;
		}
		
		if ($this->getAttribute($ident,$node,'indent') && $this->getBool($ident)==false)
		{
			$src=preg_replace('/^\s+/m','',$src);
		}
		
		if ($this->getAttribute($file,$node,'file',null,true,true))
		{
			eval("\$file=$file;");
			$this->context->saveSrc($file,$src);
		}
		else
			$this->context->addResource($src,'output');
			
		if($this->getAttribute($trace,$node,'trace') && $trace=="yes")
			nxltrace("save file=$file".'<ul><li>tag result: '.nl2br(htmlspecialchars($src,ENT_NOQUOTES)).'</ul><li>','frag_source');

		return '';
	}
	
	function return_($node,$content,$level,$pad)
	{
		/* value */
		if (($value=$node->getAttribute('value'))!==null)
		{
			$value = $this->parseVal($value,true);
		}
		else
		{
			$value=$content;
		}
		
		switch ($node->getAttribute('method',''))
		{
			case 'xml2text':
				$src=nl2br(htmlspecialchars($value));
				break;
			case 'php':
				$src='<?php'."\n$value ".'?>';
				break;
			default:
				$src=$value;
				break;
		}
		
		if ($this->getAttribute($ident,$node,'indent') && $this->getBool($ident)==false)
		{
			$src=preg_replace('/^\s+/m','',$src);
		}
		
		return $src;
	}
}
?>
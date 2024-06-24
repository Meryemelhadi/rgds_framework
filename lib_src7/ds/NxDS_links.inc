<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'ds/NxDS_xdocs.inc');

class NxDS_links extends NxDS_xdocs
{
	function __construct(&$props)
	{
		parent::__construct($props,'links/');
	}

	function getRecordCount($view, &$errorCB)
	{		
		$links =& $this->getRecords($view, $errorCB);
		return $links->count();
	}
	
	// export of data structure as PHP
	function var_export(&$doc)
	{
		$s = 'array(';
		foreach ($doc as $link)
		{
			$s.="\narray(";
			foreach ($link as $p=>$v)
			{
				$s .= "'{$p}'=>'$v',";
			}
			$s .= '),';
		}
		return $s.')';	
	}
	
	// parse html links from file and return a record collection
	function &parseDoc($src)
	{
		nxLog("links:[$src]",'DS:LINKS');
		
		// get links properties: link.href, link.key, link.title, link.text
		$this->links=array();
		
		$linksStr = explode('</a>',$src);

		// pattern/replace arrays
		$patterns = array(
			'%id="([^"]+)"%sie',
			'%href="(http://[^"]+)"%sie',
			//'%href="([^."]+)[.]htm[l]?"%esi',
			'%href="([^"]+)"%sie',
			'%target="([^"]+)"%sie',
			'%>(.*)$%esi',
			);
			
		$replace = array(
			'$this->setLinkProp("link.id","$1")',
			'$this->setLinkProp("link.href","$1")',
			//'$this->setLinkProp("link.href","?docID=$1")',
			'$this->setLinkProp("link.href","$1")',
			'$this->setLinkProp("link.target","$1")',
			'$this->setLinkProp("link.text","$1")',
			);
		
		foreach ($linksStr as $linkStr)
		{
			$this->link=array();
					
			// extract link properties
			preg_replace($patterns, $replace,$linkStr);
			
			// add link to collection
			if (isset($this->link['link.href']) && isset($this->link['link.text']))
				$this->links[]=$this->link;
		}

		return $this->links;
	}	
	
	function setLinkProp($p,$v)
	{
		nxLog("HTML content , link[$p]=$v","STATIC_HTML");
		$v=str_replace('#$#','$',$v);		
		$this->link[$p]=$v;
		return '';
	}	
}

?>
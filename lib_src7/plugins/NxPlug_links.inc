<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// link data source: read a collection of links from a static content html file and
// returns it as record collection with fields:
// "link.href", "link.key", "link.title" and "link.text"
class NxPlug_links
{
	function __construct(&$props)
	{
		$this->props=&$props;
		$this->linksRecs=array();
	}
	
	// load links from html (<a> tags) and returns it a record collection.
    function & getRecords($view, &$errorCB)
	{
		// get property path
		$path = 'links/'.strtr($view,".","/");
		
		if (!isset($this->linksRecs[$path]))
			$this->linksRecs[$path] =& $this->loadLinks($path);
			
		return $this->linksRecs[$path];
	}

	function & getRecordCount($view, &$errorCB)
	{		
		$links =& $this->getRecords($view, $errorCB);
		return $links->count();
	}
	
	// add/modify records with view name
	function putRecords($view, &$errorCB, $records,$dbOp=null)
	{	
		return false;
	}
	
	// load a collection of links and returns it as a record collection with 
	// link.href, link.key, link.title, link.text fields.
	function &loadLinks($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;

		if (is_file($f=NX_CONTENT.$path.'_'.$NX_LANG.'.html'))
			return  $this->parseLinks($f); 
		else if (is_file($f=NX_CONTENT.$path.'_'.$NX_DEFT_LANG.'.html'))
			return $this->parseLinks($f); 
		else if (is_file($f=NX_CONTENT.$path.'.html'))
			return $this->parseLinks($f); 
		else if (is_file($f=NX_DIS_CONTENT.$path.'_'.$NX_DEFT_LANG.'.html'))
			return $this->parseLinks($f); 
		else if (is_file($f=NX_DIS_CONTENT.$path.'_'.$NX_DEFT_LANG.'.html'))
			return $this->parseLinks($f); 
		else if (is_file($f=NX_DIS_CONTENT."{$path}.html"))
			return $this->parseLinks($f); 

		$res=false;
		return $res;
	}	

	// parse html links from file and returns a record coleciton
	function &parseLinks($path)
	{
		$src = $this->file_get_contents($path);	
		if ($src===0)
			return false;
			
		// get links properties: link.href, link.key, link.title, link.text
		$this->links=array();
		
		$linksStr = explode('</a>',$src);
		foreach ($linksStr as $linkStr)
		{
			$this->link=array();
				
			// extract head and returns body	
			$patterns = array(
				'%href="(http://[^"]+)"%sie',
				//'%href="([^."]+)[.]htm[l]?"%esi',
				'%href="([^"]+)"%sie',
				'%target="([^"]+)"%sie',
				'%>(.*)$%sie',
				);
				
			$replace = array(
				'$this->setLinkProp("link.href","$1")',
				//'$this->setLinkProp("link.href","?docID=$1")',
				'$this->setLinkProp("link.href","$1")',
				'$this->setLinkProp("link.target","$1")',
				'$this->setLinkProp("link.text","$1")',
				);
	
			// extract link properties
			preg_replaceX($patterns, $replace,$linkStr,$this);
			
			// add link to collection
			if (isset($this->link['link.href']) && isset($this->link['link.text']))
				$this->links[]=$this->link;
		}
		
		// return links as a record collection
		if (count($this->links)>0)
		{
			require_once(NX_LIB."NxData_ArrayRecords.inc");
			$res =& new NxData_ArrayRecords($this->links);
		}
		else
			$res=null;
		
		return $res;
	}	
	
	function setLinkProp($p,$v)
	{
		nxLog("HTML content , link[$p]=$v","STATIC_HTML");
		$v=str_replace("\'","'",$v);
		$this->link[$p]=$v;
		return '';
	}	
	
	function &file_get_contents($filename) 
	{
		$fp = @fopen($filename, "r");
		if (!($fp))
			return 0;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		return $temp;
	}

}

?>
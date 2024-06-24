<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// doc data source: read a collection of docs from a static content html file and
// returns it as record collection with fields:
// "doc.href", "doc.key", "doc.title" and "doc.text"

require_once(NX_LIB.'ds/NxDS_xdocs.inc');

// This data source reads an html document and returns it as a record of doc property
// including a text version of the doc, converted with html2text()
class NxDS_OC_doc extends NxDS_xdocs
{
	function __construct(&$props)
	{
		parent::__construct($props,'');
	}

	function getSrcPath($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;
		
//		$basePath=NX_CONTENT.'OC_pages/'.$path;
		$basePath=NX_CONF.'view/default/www.openclinical.co.uk/'.$path;

		if (is_file($f=$basePath.'_'.$NX_LANG.'.html') ||
			is_file($f=$basePath.'_'.$NX_DEFT_LANG.'.html') ||
			is_file($f=$basePath.'.html'))
			return $f;
			
		$this->error->addError("cant find content source: $path");
		return null;
	}
	
	function &parse($path)
	{
		$src = $this->file_get_contents($path);	
		if ($src===0)
			return false;
					
		// parse doc
		return $this->parseDoc($src);
	}
	
	function &parseDoc(&$src)
	{
		$this->doc=array();
		// extract head and returns body	
		$patterns = array(
			'%[ ]?tppabs="[^"]+"[ ]?%s',
			'%<title>(.*)</title>%sie',
			'%<meta name="([^"]+)" content="([^"]+)">%sie',
			'%pathSupplier\("([^"]+)"\)%e',
			'%href="([^."]+)[.]htm[l]?"%sie',
			'%src="[.][.]/images/([^"]+)"%sie',
			// '%<TD class="textBody">(.*)<!-- END TEXT BODY -->%esi',
			'%<!-- TEXT BODY -->%i',
			'%<TD class="textBody">(.*)</td>[\s\n]*<!-- END TEXT BODY -->%sie',
			);
			
		$replace = array(
			' ',
			'$this->getTitle("$1")',
			'$this->getMeta("$1","$2")',
			'$this->getMeta("SubSection","Supplier");$this->getMeta("Supplier","$1");',
			'$this->getHref("$1")',
			'$this->getImage("$1")',
			'',
			'$this->getBody("","$1")',
			);

		// extract head, title and body code
		$res = preg_replaceX($patterns, $replace,$src,$this);
		
		return $this->doc;
	}
}

?>
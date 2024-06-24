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
class NxDS_xdocs2 extends NxDS_xdocs
{
	function __construct(&$props)
	{
		parent::__construct($props);
	}

	function getExePath($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;	
		return $GLOBALS['NX_CONTENT_COMP']."{$path}_{$NX_LANG}.html+text.php";
	}
	
	function getBody($attr,$s)
	{
		nxLog("HTML content , got body:[$s]","STATIC_HTML");

		// register the html version
		$s=str_replace("\'","'",$s);
		$this->setDocProp('content',$s);

		// translate the html -> text
		require_once(NX_LIB.'tools/class.html2text.inc');
		$h2t =& new html2text($s);
		$text = $h2t->get_text();
		$this->setDocProp('text',$text);
		// $this->setDocProp('content',"<pre>$text</pre>");
		
		return '';
	}
}

?>
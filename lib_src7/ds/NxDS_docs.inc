<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// doc data source: read a collection of docs from a static content html file and
// returns it as record collection with fields:
// "doc.href", "doc.key", "doc.title" and "doc.text"

require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_docs extends NxDS_Source
{
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->docRecs=array();
		$this->fs=$this->getPlugIn('FileSys');
	}
	
	// load docs from html (<a> tags) and returns it as a record.
    function getRecords($view, &$errorCB,$data=null)
	{
		// get actual view
		$view = $this->getProperty($view.'.db',$view);
		
		// get property path
		$path = 'docs/'.strtr($view,".","/");
		
		if (!isset($this->docRecs[$path]))
			$this->docRecs[$path] =& $this->loadDoc($path);
			
		return $this->docRecs[$path];
	}

	function getRecordCount($view, &$errorCB)
	{		
		return 1;
	}
	
	// add/modify records with view name
	function putRecords($records,$view, &$errorCB,$dbOp=null)
	{	
		return false;
	}
	
	// add/modify records with view name
	function putRecord($record,&$errorCB,$dbOp=null)
	{	
		return false;
	}
	
	// load a collection of docs and returns it as a record collection with 
	// doc.href, doc.key, doc.title, doc.text fields.
	function &loadDoc($path)
	{
		global $NX_LANG;
		global $NX_DEFT_LANG;

		if (is_file($f=NX_CONTENT.$path.'_'.$NX_LANG.'.html'))
			return  $this->parseDoc($f); 
		else if (is_file($f=NX_CONTENT.$path.'_'.$NX_DEFT_LANG.'.html'))
			return $this->parseDoc($f); 
		else if (is_file($f=NX_CONTENT.$path.'.html'))
			return $this->parseDoc($f); 
		else if (is_file($f=NX_DIS_CONTENT.$path.'_'.$NX_DEFT_LANG.'.html'))
			return $this->parseDoc($f); 
		else if (is_file($f=NX_DIS_CONTENT.$path.'_'.$NX_DEFT_LANG.'.html'))
			return $this->parseDoc($f); 
		else if (is_file($f=NX_DIS_CONTENT."{$path}.html"))
			return $this->parseDoc($f); 

		$res=false;
		return $res;
	}	

	function &parseDoc($path)
	{
		$src = $this->file_get_contents($path);	
		if ($src===0)
			return false;

		$this->doc=array();

		// extract head and returns body	
		$patterns = array(
			'%<title>(.*)</title>%sie',
			'%<meta name="([^"]+)" content="([^"]+)">%sie',
			'%href="([^."]+)[.]htm[l]?"%sie',
			'%src="[.][.]/images/([^"]+)"%sie',
			'%<body([^>]*)>(.*)<.body>%sie',
			);
			
		$replace = array(
			'$this->getTitle("$1")',
			'$this->getMeta("$1","$2")',
			'$this->getHref("$1")',
			'$this->getImage("$1")',
			'$this->getBody("$1","$2")',
			);

		// extract head, title and body code
		$res = preg_replaceX($patterns, $replace,$src,$this);
		
		// return doc as an array record
		if (count($this->doc)>0)
		{
			require_once(NX_LIB."NxData_ArrayRecords.inc");
			$res =& new NxData_ArrayRecords($this->doc);
		}
		else
			$res=null;
		
		return $res;
	}

	function getTitle($s)
	{
		nxLog("HTML content , got title:[$s]","STATIC_HTML");
		$s=str_replace("\'","'",$s);
		$this->setDocProp('title',$s);
		return '';
	}
	
	function getHref($id)
	{
		nxLog("HTML content , link to doc id :[$id]","STATIC_HTML");
		return 'href="?docID='.$id.'"';
	}

	function getMeta($n,$v)
	{
		nxLog("HTML content , got meta:[$n]=[$v]","STATIC_HTML");
		$v=str_replace("\'","'",$v);
		$this->setDocProp('meta.'.$n,$v);
		return '';
	}
	
	function getBody($attr,$s)
	{
		nxLog("HTML content , got body:[$s]","STATIC_HTML");
		$s=str_replace("\'","'",$s);
		$this->setDocProp('content',$s);
		return '';
	}

	function getImage($name)
	{
		nxLog("HTML content , deploying image:[$name]","STATIC_HTML");
		$src=NX_CONTENT."images/{$name}";
		$dest=NX_CONTENT_IMAGES.$name;
		$dest_url=NX_CONTENT_IMAGES_URL.$name;
		$this->copyRes($src,$dest);
		return 'src="'.$dest_url.'"';
	}
	
	// search for an image in deployemt directory
	function copyRes($src,$dest)
	{
		// prefix for resources in skin
		// $res="res/";

		if (is_file($dest))
		{
			if (NX_CHECK_TPL!='compile')
				return $dest;
		}
		
		nxLog("Trying content image file: {$src}",'STATIC_CONTENT');
		if (is_file($src))
		{
			// optimistic: assume dir existence
			if ($this->fs->copy($src, $dest))
			{
				nxLog("deployed resource from $src to $dest","NxPlug_Template",NX_COMPILE_EVENT);
				return $dest;
			}
				
			// failed: check directory
			if (!is_dir($d=dirname($dest)))
			{
				// create dir
				if ($this->fs->createTree($d))
				{
					// try again to copy the file
					if ($this->fs->copy($src,$dest))
					{
						nxLog("imported content image from $src to $dest","STATIC_CONTENT",NX_COMPILE_EVENT);
						return $dest;
					}
				}
			}

			// can't copy file => log and run view from source
			nxError("can't copy file:$src to $dest.","STATIC_CONTENT");
			return $src;
		}
		
		return false;
	}
		
	function setDocProp($p,$v)
	{
		nxLog("HTML content , doc[$p]=$v","STATIC_HTML");
		$this->doc["page.$p"]=$v;
		return '';
	}	
	/*
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
	*/

}

?>
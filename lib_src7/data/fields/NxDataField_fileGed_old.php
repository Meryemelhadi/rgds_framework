<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxDataField_fileGed extends NxData_Field
{
	var $gFile;
	
	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}

	function clear()
	{
		$this->gFile = null;	
	}

	function & toObject($opts=null)
	{ 
		return $this->gFile; 
	}

	function readFromStore(&$v){
		$this->gFile = $v;
	}

	function onDelete(&$errorCB)
	{
		try {
			$gFile->delete($this->gFile,$errorCB,true);
		}
		catch (Exception $e)
		{
			$errorCB->addError($e);
		}
	}
	
	function clone_it(&$record,&$errorCB,$deepCopy=true)
	{
		// create a new field with resource info
		$f = new NxDataField_file($this->desc);
		$f->record=&$record;

		if (isset($this->gFile))
		{
			$f->gFile = $this->gFile->clone_it();
		}
		
		return $f;
	}

	function readFromDB($recData)
	{
		$data = $this->unserialise($this->getFieldData($recData,$this->getFName(),null));
		$this->gFile = NxGED_Factory::getFile($this->desc,$data);
	}

	function toDB($opts=null) 
	{
		if (is_array($this->info))
		{
//			return "'".htmlspecialchars($this->serialise($this->info),ENT_QUOTES)."'";
			return "'".$this->serialise($this->gFile->info)."'";
		}
		else
		{
			return "NULL";
		}
	}

	function toString($opts=null)
	{
		$type='text';
		$box=null;
		if ($opts!=null) {
			$opts2 = preg_replace_callback('/(path|size|type|js|html|url|string)/i',
				function($m) use(&$type) {$type=$m[1];},$opts);

			$opts2 = preg_replace_callback('/(original|[0-9]+x[0-9]+)/i',
				function($m) use(&$box) {$box=$m[1];},$opts);
		}
				
		switch($type)
		{
			default:
			case 'url':
				return $this->_getUrl(true);
			case 'base_url':
				return $this->_getUrl(false);
			case 'js':
				return $this->toJS('\'');
			case 'name':
				return $this->gFile->name;
			case 'size':
				return $this->toSize();
			case 'type':
				return $this->toType();
			case 'dir':
				return $this->gFile->dir;
			case 'path':
				// path
				return $this->info['file'];
		}
	}

	function toSize()
	{

		if ($size=$this->gFile->size)
		{	
			if ($size>1000000)
				$s=sprintf("%3.2f %s",(float)$size/1000000,$this->desc->getString('MB','sys_units'));
			elseif ($size>1000)
				$s=sprintf("%3.2f %s",(float)$size/1000,$this->desc->getString('KB','sys_units'));
			else
				$s=sprintf("%d %s",$size,$this->desc->getString('B','sys_units'));		
				
			return $s;
		}
		else
			return '';
	}
	
	function toType()
	{
		if ($t=$this->gFile->type)
			return $t;
		else
			return 'default';		
	}
		
	function toTime()
	{
		if ($t=$this->gFile->time)
		{
			$fmt = $this->desc->getProperty("dateTimeFormatLong","%A %d %B %Y, %Hh%Mm%S");
			return strftime($fmt,$t);
		}
		else
			return '';		
	}
	
	function toTitle()
	{
		return $this->gFile->title;
	}	
	
	function toUser()
	{
		if (isset($this->info['user_service']) && $this->info['user_service']!='user')
		{
			return "[{$this->info['user_service']}]";
		}
		if (isset($this->info['user']))
		{
			return $this->info['user'];
		}
		else
			return '';		
	}
	
	function toAttribute($att)
	{
		if ($r=$this->gFile->$att)
			return $r;
		else
			return '';
	}

	/* magic getter */
	function __get($f) {
		if (method_exists ( $this , $tf="to$f" ) || method_exists ( $this , $tf="get$f" ))
		{
			$res=$this->$tf();
			if ($tf=='todb')
				$res=trim($res,'\'');
			return $res;
		}

		switch($f)
		{
			case 'empty':
				return !isset($this->info['n']);
			case 'isOk':
				return isset($this->info['n']);
			case 'url':
				return $this->_getUrl(true);
			case 'base_url':
				return $this->_getUrl(false);
			case 'info':
				return $this->serialise($this->info);
			case 'js':
				return $this->toJS('\'');
			case 'size':
				return $this->toAttribute('size');
			case 'user':
				return $this->toUser();
			case 'user_service':
				return $this->toAttribute('user_service');
			case 'user_oid':
				return $this->toAttribute('user_oid');
			case 'fileName':
			case 'name':
			case 'n':
				return $this->toAttribute('n');
			case 'title':
				return $this->toAttribute('title');
			case 'type':
				return $this->toType();
			case 'time':
				return $this->toTime('time');
		}
		return null;
	}
	

	function toHTML($opts=null,$box=null)
	{
		$format='html';
		$box=null;
		if ($opts!=null) {
			$opts2 = preg_replace_callback('/(js|viewer|info|name|title|size|type|html|url|string|time|user)/i',
				function($m) use(&$format) {$format=$m[1];},$opts);

			$opts2 = preg_replace_callback('/(original|[0-9]+x[0-9]+)/i',
				function($m) use(&$box) {$box=$m[1];},$opts);
		}

		switch($format)
		{
			case 'url':
				return $this->_getUrl(true);
			case 'viewer':
				switch ($this->toType())
				{
					case 'jpg':
					case 'gif':
					case 'png':
					case 'jpg':
						$control =& $this->desc->getControl("image");
						break;
					case 'swf':
						$control =& $this->desc->getControl("swf");
						break;
					default:
						$control =& $this->desc->getControl("file_default");
						break;						
				}		
				return $control->toHTML($this,$this->info,$opts);

			case 'base_url':
				return $this->_getUrl(false);
			case 'info':
				return $this->serialise($this->info);
			case 'js':
				return $this->toJS('\'');
			case 'size':
				return $this->toAttribute('size');
			case 'user':
				return $this->toUser();
			case 'user_service':
				return $this->toAttribute('user_service');
			case 'user_oid':
				return $this->toAttribute('user_oid');
			case 'name':
				return $this->toAttribute('n');
			case 'title':
				return $this->toAttribute('title');
			case 'type':
				return $this->toType();
			case 'time':
				return $this->toTime('time');

			default:
				switch ($this->toType())
				{
					case 'jpg':
					case 'gif':
					case 'png':
					case 'jpg':
						$ctrl='default';
						break;
					default:
						$ctrl='default';
						break;						
				}
				
				$control =& $this->desc->getControl("file_$ctrl");
				return $control->toHTML($this,$this->info,$opts);	
		}

	}

	function _getUrl($full=true)
	{
		if (isset($this->info["n"]))
		{
			$url = $full?$this->desc->getProperty("baseUrlUpload","/files/"):'';
			if (isset($this->info['dir']) && $this->info['dir'])
				$url = $url.urlencode($this->info['dir']).'/';
				
			return $url . $this->info["n"];
		}
		else
			return null;
	}

	function getPath($full=true)
	{
		if (isset($this->info["n"]))
		{
			$path = $this->info["n"];
			if (isset($this->info['dir']) && $this->info['dir'])
				$path = $this->info['dir'].'/'.$path;

			if ($full && $path)
				$path = $this->desc->getProperty("pathUpload").$path;
		}
		elseif(is_string($this->info))
			 $path = $this->info;
		else
			 $path = '';

		return $path;
	}
	
	function toJS($opts='\'',$box=null)
	{
		if ($opts!='')
			$sep=$opts;
		else
			$sep='\'';

		if (!isset($this->info["n"]))
			return 'error:"no file"';

		$url = $this->desc->getProperty("baseUrlUpload","/files/");
		if (isset($this->info['dir']))
			$url = "{$url}{$this->info['dir']}/";
			
		$name=$this->info["n"];
		
		if (isset($this->info['size']))
			$size = $this->info['size'];
		else
			$size=-1;
			
		if (isset($this->info['type']))
			$type = $this->info['type'];
		else
			$type='default';

		return "name:$sep$name$sep,type:$sep$type$sep,size:$sep$size$sep,url:$sep$url" . $this->info["n"]."$sep";
	}
	
	function toForm($opts=null) 
	{
		$desc = & $this->desc;		
		if (NX_FILE_LIB==='none')
			$control =& $desc->getControl($desc->getProperty('control','file'));
		else
			$control =& $desc->getControl($desc->getProperty('control','file_db'));
		return $control->toForm($this,$this->info);	
	}

	// dependedncy on properties:
	// "imageSizes" : name of the box to which the image should be resized to, 
	// otherwise fetches its dimension array in:
	// "image.box.100x100" => array("w"=>100,"h"=>100) or
	// "image.box.thumb" => array("w"=>10,"h"=>10)
	// otherwise if name of the form 100x40, use it as dimensions.
	// If no box or not found, no resize is done (simple copy)
	// NB. use GD for image resizing.
	// Returns image object as:
	// 	"url,w,h,type"
	function _readFromForm(&$recData,&$errorCB)
	{	
		$desc = & $this->desc;			
		if (NX_FILE_LIB==='none')
			$control = $desc->getControl($desc->getProperty('control','file'));
		else
			$control = $desc->getControl($desc->getProperty('control','file_db'));

		return $control->readForm($this,$recData,$errorCB);
	}

	function readFromFile($fpath,$toDir='',$move=false,$fileContent=null)
	{
		if (!$toDir)
			$toDir = basename(dirname($fpath));
		else
			$toDir = trim($toDir,'/');

		$basePath = $this->getProperty('pathUpload',NX_DOC_ROOT.'files/');
		$fdest = $basePath . $toDir .'/'. baseName($fpath);
		
		require_once(NX_LIB.'plugins/NxPlug_FileSys.inc');
		$fs = new NxPlug_FileSys($this->desc);

		if($fileContent!==null)
		{
			$fs->saveFile($fdest,$fileContent,true);

			$this->info = array(
				'n' => baseName($fpath),
				'dir' => $toDir,
				);

			return true;
		}
		elseif($fs->copy($fpath,$fdest))
		{
			if($move)
				$fs->delete($fpath);

			$this->info = array(
				'n' => baseName($fpath),
				'dir' => $toDir,
				);

			return true;
		}

		return false;
	}
}

?>
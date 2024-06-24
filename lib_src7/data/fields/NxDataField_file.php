<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxDataField_file extends NxData_Field
{
	var $info;
	
	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}

	function clear()
	{
		$this->info = null;	
	}

	function & toObject($opts=null)
	{ 
		return $this->info; 
	}

	function readFromStore(&$v){
		$this->info = $v;
	}

	function onDelete(&$errorCB)
	{
		// delete image if exists and if not a reference to image database
		if(isset($this->info) && !(isset($this->info['is_db']) && $this->info['is_db']))
		{
			$fileStore = $this->desc->getPlugIn("FileStore");
			$fileStore->delete($this->info,$errorCB,true);
		}
	}
	
	function clone_it(&$record,&$errorCB,$deepCopy=true)
	{
		// create a new field with resource info
		$f = new NxDataField_file($this->desc);
		$f->record=&$record;

		if (isset($this->info["n"]))
		{
			// duplicate image and returns its descriptor
			$fileStore = $this->desc->getPlugIn("FileStore");
			if ($deepCopy)
				$f->info = $fileStore->clone_it($this->info,$errorCB);
			else
				$f->info = $this->info;

			if ($errorCB->isError())
				nxError('error cloning field file => lose file in clone');
		}
		return $f;
	}

	function readFromDB($recData)
	{
		$this->info = $this->unserialise($this->getFieldData($recData,$this->getFName(),null));
	}

	function toDB($opts=null) 
	{
		if (is_array($this->info))
		{
//			return "'".htmlspecialchars($this->serialise($this->info),ENT_QUOTES)."'";
			return "'".$this->serialise($this->info)."'";
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
		if ($opts!=null)
			/*
			preg_replace(array('/(path|name|size|type|js|html|url|string)/ie','/(original|[0-9]+x[0-9]+)/ie'),
				array('$type="$1"','$box="$1"'),$opts);
			*/

			preg_replace_callback_array(
				array(
					'/(path|name|size|type|js|html|url|string)/i' => 
						(function($m) use (&$type) {
							return $type=$m[1];
						}),

					'/(original|[0-9]+x[0-9]+)/i' => 
						(function($m) use (&$box) {
							return $box="$m[1]";
						})
				),
				$opts);
				
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
				return $this->info['n'];
			case 'size':
				return $this->toSize();
			case 'type':
				return $this->toType();
			case 'dir':
				return $this->info['dir'];
			case 'path':
				// path
				return $this->info['file'];
		}
	}

	function toSize()
	{

		if (isset($this->info['size']))
		{	
			$size=$this->info['size'];
			
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
		if (isset($this->info['type']))
			return $this->info['type'];
		else
			return 'default';		
	}
		
	function toTime()
	{
		if (isset($this->info['time']))
		{
			$fmt = $this->desc->getProperty("dateTimeFormatLong","%A %d %B %Y, %Hh%Mm%S");
			return strftime($fmt,$this->info['time']);
		}
		else
			return '';		
	}
	
	function toTitle()
	{
		if (isset($this->info['title'])) return $this->info['title'];
		if (isset($this->info['n'])) return $this->info['n'];
		return '';
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
		if (isset($this->info[$att]))
			return $this->info[$att];
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
			case 'path':
				// path
				if(isset($this->info['file']))
					return $this->info['file'];
				else
					return null;
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
		if ($opts!=null)
		{
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
			case 'n':
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
				$url = "{$url}{$this->info['dir']}/";
//				$url = $url.urlencode($this->info['dir']).'/';
				
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
		if (defined('NX_FILE_LIB') && NX_FILE_LIB==='none')
			$control = $desc->getControl($desc->getProperty('control','file'));
		else
			$control = $desc->getControl($desc->getProperty('control','file_db'));

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
		if (defined('NX_FILE_LIB') && NX_FILE_LIB==='none')
			$control = $desc->getControl($desc->getProperty('control','file'));
		else
			$control = $desc->getControl($desc->getProperty('control','file_db'));

		return $control->readForm($this,$recData,$errorCB);
	}

	function readFromFile($fpath,$toDir='',$move=false,$fileContent=null)
	{
		if (!$toDir)
		{
			$keep = true; // we keep the same file
			$toDir = basename(dirname($fpath));
		}
		else
		{
			$keep = false; // we copy the file
			$toDir = trim($toDir,'/');
		}

		$basePath = $this->getProperty('pathUpload',NX_DOC_ROOT.'files/'). $toDir .'/';
		$fdest = $basePath . baseName($fpath);
		$baseName=basename($fpath);
		
		require_once(NX_LIB.'plugins/NxPlug_FileSys.inc');
		$fs = new NxPlug_FileSys($this->desc);

		if($fileContent!==null)
		{
			$fs->saveFile($fdest,$fileContent,true);

			$this->info = array(
				'n' => $baseName,
				'dir' => $toDir,
				);

			return true;
		}

		if(!$keep)
		{
			// get ext
			$parts = explode('.',$baseName);
			$ext = $parts[count($parts)-1];
			unset($parts[count($parts)-1]);
			$baseNoExt = implode('.',$parts);

			// get unique name
			$fileStore = $this->desc->getPlugIn("FileStore");
			$fullPath='';
			$storageName='';
			$fileStore->getUniqName($basePath,$baseNoExt,$ext,
					$storageName,$fullPath);
						
			// if a box is defined, resize the image and keep resized version as reference
			$fdest = $fullPath;

			// copy
			if(!$fs->copy($fpath,$fdest))
				return false;

			if($move)
				$fs->delete($fpath);
		}

		$this->info = array(
			'n' => baseName($fpath),
			'dir' => $toDir,
			);

		return true;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
include_once('NxDataField_file.php');

class NxDataField_image extends NxDataField_file
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

	function toStatus() { 
		return isset($this->errorMsg)?'error': 
			(empty($this->info["n"])?'empty':'ok');
	}

	function readFromStore(&$v){
		$this->info = $v;
	}

	function onDelete(&$errorCB)
	{
		// delete image if exists and if not a reference to image database
		if(isset($this->info) && !(isset($this->info['is_db']) && $this->info['is_db']))
		{
			$fileStore = $this->desc->getPlugIn("ImageStore");
			$fileStore->delete($this->info,$errorCB,true);
		}
	}
	
	function clone_it(&$record,&$errorCB,$deepCopy=true)
	{
		// create a new field with resource info
		$f = new NxDataField_image($this->desc);
		$f->record=&$record;

		if (isset($this->info["n"]))
		{
			// duplicate image and returns its descriptor
			$fileStore = $this->desc->getPlugIn("ImageStore");
			if ($deepCopy)
				$f->info = $fileStore->clone_it($this->info,$errorCB);
			else
				$f->info = $this->info;

			if ($errorCB->isError())
				nxError('error cloning field image => lose image file in clone');
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
				return $this->_getUrl();
			case 'js':
				return $this->toJS('\'');
			case 'name':
				return $this->info['n'];
			case 'size':
				return $this->info['size'];
			case 'type':
				return $this->toType();
			case 'dir':
				return $this->info['dir'];
			case 'path':
				// path
				return $this->info['file'];
		}
	}
	
	function toHTML($opts=null,$box=null)
	{
		$type='html';
		$box=null;
		$style='';
		if ($opts!=null)
        {
			preg_replace_callback_array(
                array(
                '/(left|right)/i'=>function($m) use(&$style){$style='float:'.$m[1]; },
                '/(middle)/i'=>function($m) use(&$style){$style='clear:both'; },
                '/(js|html|url|string)/i'=>function($m) use(&$type){$type=$m[1];},
                '/(original|[0-9]+x[0-9]+)/i'=>function($m) use(&$box){$box=$m[1];}
                ),
				$opts);
        }
				
		switch($type)
		{
			case 'url':
				return $this->_getUrl($box);
			case 'js':
				return $this->toJS('\'',$box);
			default:
				// optimisation: store url in db ?
				if (isset($this->info['n']))
					return "<img{$style} border=\"0\" src=\"".$this->_getUrl($box).'" alt=""/>';
				elseif (is_string($this->info) && strlen($this->info)>6)
				{
					$this->info=$this->unserialise($this->info);
					return "<img{$style} border=\"0\" src=\"".$this->_getUrl($box).'" alt=""/>';
				}
				else
					return "&nbsp;";
		}
	}

	function _getUrl($box=null)
	{
		if (isset($this->info["n"]))
		{
			$url = $this->desc->getProperty("baseUrlUpload",NX_BASE_URL.'afiles/');
			if (isset($this->info['dir']) && $this->info['dir'])
				$url = "{$url}{$this->info['dir']}/";
			if ($box==null)
				$box = $this->desc->getProperty("imageSize",null,false);
			if ($box != null && $box != "original")
				$url .= "{$box}/";			
				
			return $url . $this->info['n'];
		}
		else
			return null;
	}

	function _getFileInfo($box,&$destFilepath,&$storageBaseDir,&$destDir)
	{
		if (isset($this->info["n"]))
		{
			$storageBaseDir=$this->desc->getProperty('pathUpload',NX_DOC_ROOT.'files/');
			$storageBaseDir=trim($storageBaseDir,'/').'/';

			if (isset($this->info['dir']) && $this->info['dir'])
				$storageBaseDir = "{$storageBaseDir}{$this->info['dir']}/";

			if ($box==null)
				$box = $this->desc->getProperty("imageSize",null,false);

			if ($box != null && $box != "original")
			{
				$destDir="{$storageBaseDir}{$box}/";
			}
			else
			{
				$destDir="{$storageBaseDir}";
			}
				
			return $destFilepath = $destDir . $this->info['n'];
		}
		else
			return null;
	}
	
	function toJS($opts='\'',$box=null)
	{
		if ($opts!='')
			$sep=$opts;
		else
			$sep='\'';

		if (!isset($this->info["n"]))
			return 'error:"no image"';

		$url = $this->desc->getProperty("baseUrlUpload","/images/");
		if ($box==null)
			$box = $this->desc->getProperty("imageSize",null,false);
		if ($box != null && $box != "original")
			$url .= $box . "/";
		if (isset($this->info['dir']))
			$url = "{$url}{$this->info['dir']}/";
		$name=$this->info["n"];
			
		return "name:$sep$name$sep,url:$sep$url" . $this->info["n"]."$sep,box:$sep$box$sep";
	}
	
	function toForm($opts=null) 
	{
		$desc = & $this->desc;		
		if (NX_IMAGE_LIB==='none'||NX_IMAGE_LIB===false)
			$control = $desc->getControl($desc->getProperty('control','image'));
		else
			$control = $desc->getControl($desc->getProperty('control','image_db'));
		return $control->toForm($this,$this->info);	
	}

	function toBin($opts=null)
	{
		if (!isset($this->info["n"]))
			return null;

		// get size and format
		$this->format='base64';
		$this->box='';
		$h=100;
		$w=100;
		$style='';
		if ($opts!=null)
			preg_replaceX(array(
				'/(base64)/ie',
				'/(original|([0-9]+)x([0-9]+))/ie'),
				array(
					'$this->format="$1"',
					'$this->box="$1"'),
				$opts,$this);

		// create image if doesnt exist
		$name = $this->info['n'];

		$destFilepath=$storageBaseDir=$storageBaseDir=$destDir = null;
		$filePath = $this->_getFileInfo($this->box,$destFilepath,$storageBaseDir,$destDir);

		if (!is_file($filePath))
		{
			// original file name
			$oriPath=$storageBaseDir.$name;

			nxLog("deploying from $oriPath to $filePath...",'BASE64');
			if (!is_file($oriPath))
			{
				// unknown image
				nxLog("cant find original image:$oriPath",'BASE64');
				return false;					
			}
						
			// resize image
			include_once(NX_LIB.'404/NxRes_ResizeImg.inc');
			$resizer = new NxRes_ResizeImg($this->desc);
			$dim = explode("x",$this->box);
			if (!$resizer->resizeImage($oriPath,$filePath,$storageBaseDir,$dim[0],$dim[1]))
			{
				nxError("cant resize image :$oriPath to $filePath",'BASE64');
				return '';
			}
			nxLog("image resized",'BASE64');
		}

		// get image content
		$fs=$this->desc->getPlugin('FileSys');
		$bin=$fs->file_get_contents($filePath,"rb");
		nxLog("image resized BIN",$bin);
		$encoded = chunk_split(base64_encode($bin),76);
		nxLog("image resized B64",$encoded);
		return $encoded;
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
		$control = $desc->getControl($desc->getProperty('control','image_db'));
		return $control->readForm($this,$recData,$errorCB);
	}
}

?>
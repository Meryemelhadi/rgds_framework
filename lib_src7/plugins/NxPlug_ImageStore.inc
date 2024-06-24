<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_ImageStore
{
	function __construct(&$props)
	{
		$this->props =& $props;
		$this->fileTypes = $this->props->getProperty(
							"fileTypes",
							array(
								"image/pjpeg"=>"jpg", 
								"image/jpeg" => "jpg",
								"image/gif"	=> "gif", 
								"image/png"	=> "png", 
								"image/x-png" => "png"
								)
							);
	}
	
	function move_uploaded_file($filePath,$fullPath)
	{
		if (move_uploaded_file($filePath,$fullPath))
		{
			$this->chmod($fullPath,0666);
			return true;
		}
		else
			return false;
	}
	
	function chmod($f,$p)
	{
		if (substr(php_uname(), 0, 7) == 'Windows')
			return true;
			
		if (system('/bin/chmod '.decoct($p)." $f")===false)
		{
			nxError("operation failed: chmod on file $f",'TPL');
			return false;
		}
		else
		{
			nxLog("chmod on file $f succeeded",'TPL');
			return true;
		}
	}
	
	function checkFile($fieldName,&$errorCB,$isRequired=false, $maxFileUploadSize=50000)
	{
		if (!isset($_FILES[$fieldName]))
		{
			/*
			echo "<!-- !isset(_FILES[$fieldName]) -->";
			foreach ($_FILES as $k => $v)
			{
				echo "<!-- f: $k => $v -->";
			}
			foreach ($_POST as $k => $v)
			{
				echo "<!-- p: $k => $v -->";
			}
			*/
			if ($isRequired)
				$errorCB->addError($errorCB, "missing");
				
			return false;		
		}

		// echo "<!-- isset(_FILES[$fieldName] -->";

		$fileInfo = & $_FILES[$fieldName];

		/*
		echo "<!-- size: ".$fileInfo['size']." -->";
		echo "<!-- type: ".$fileInfo['type']." -->";
		echo "<!-- tmp: ".$fileInfo['tmp_name']." -->";
		*/
		
		if ($fileInfo['size'] == 0)
		{
			if ($isRequired)
				$errorCB->addError($errorCB, "missing file");
				
			return false;		
		}
		if ($fileInfo['size'] > $maxFileUploadSize)
		{
			$errorCB->addError($this->props->getString('file too big'),$maxFileUploadSize);
			return false;
		}
		
		if (!isset($this->fileTypes[$fileInfo['type']]))
		{
			$errorCB->addError($this->props->getString("invalid file type"));
			return false;
		}

		if (!is_uploaded_file($fileInfo['tmp_name']))
		{
			$errorCB->addError($this->props->getString("failed upload"));
			return false;
		}

		// echo "<!-- checkfile ok -->";
		
		return true;
	}
	
	function getFileType($type)
	{
		if (isset($this->fileTypes[$type]))
			return $this->fileTypes[$type];
		else 
			return null;
	}

	function getUniqueFileName()
	{
		return md5(uniqid(microtime(),FALSE));
	}
	
	// store an uploaded image
	function store( $fieldName, &$errorCB, $storageName=null,
					$boxes=null, $isRequired=false, $maxFileUploadSize=50000)
	{
		$fileInfo = & $_FILES[$fieldName];

		if (!$this->checkFile($fieldName,$errorCB, $isRequired,$maxFileUploadSize))
			return null;
		
		// get file from download section
		$externalName = $fileInfo['name'];
		$baseName = current(explode(".",$externalName));
			
		$tmpName = $fileInfo['tmp_name'];

		// $mimeType = $fileInfo['type'];
		
		// get file type
		$type = $this->getFileType($fileInfo['type']);
		
		// get file name for storage
		if ($storageName == null)
		{
			global $NX_CHARSET;
			if ($NX_CHARSET=='utf-8')
				$baseName = utf8_decode($baseName);

			// $baseName=preg_replace('/[^a-zA-z0-9_]/','_',$baseName);
			$baseName= preg_replace(
						array('/[éèê]/','/[àâ]/','/[ç]/','/&[^;]+;/','/\W+/','/_$/'),
						array('e','a','c','_',''),
						strtolower($baseName));
		}
	
		// get file location
		$storageBaseDir = $this->props->getProperty("pathUpload",null);
		if ($storageBaseDir == null)
		{
			$errorCB->addError("no upload path");
			return null;
		}
		
		// try to keep name (with user prefix) or add number suffix
		$storageBaseDir = strtr(realpath($storageBaseDir),"\\","/")."/";
		$userName = $this->props->getProperty('user.userID','');
		if ($userName!='')
			$storageBaseDir.="{$userName}/";

		$fullPath='';
		$storageName='';
		$this->getUniqName($storageBaseDir,$baseName,$type,
				$storageName,$fullPath);
		
		// if no defined box, keep the original as the default
		if ($boxes == null)
			$boxes = "original";
			
		// if a box is defined, resize the image and keep resized version as reference
		$fileVars = $this->resizeUploadToBox($tmpName,$fullPath,$type,$boxes,$storageBaseDir,$storageName,$errorCB);
		if ($errorCB->isError())
			return null;

		// return file details of file for retreival
		$fileVars["n"] = $storageName;
		if ($userName!='')
			$fileVars['dir']=$userName;
			
		return $fileVars;
	}

	function getUniqName($storageBaseDir,$baseName,$type,&$storageName,&$fullPath) {
		$storageName = "{$baseName}.{$type}";
		$fullPath = $storageBaseDir . $storageName;
		$uniq=0;
		while(@is_file($fullPath)) {
			$uniq++;
			$storageName = "{$baseName}_{$uniq}.{$type}";
			$fullPath = $storageBaseDir . $storageName;
       }
	}
	
	// resize the file to fit in box which dimensions are defined in properties 
	// as: 
	// "image.box.100x100" => array("w"=>100,"h"=>100) or
	// "image.box.thumb" => array("w"=>10,"h"=>10)
	function resizeUploadToBox($filePath,$fullPath,$type,$boxes,$storageBaseDir,$storageName,&$errorCB)
	{
		$gd = $this->props->getPlugin("ImageGD");
		$us = $this->props->getPlugIn("UserSpace");

		// check that it is a valid image
		$res = $gd->getImageSize($filePath,$errorCB);
		if ($errorCB->isError())
			return null;
		
		// create target directory if not exist
		if (!is_dir($storageBaseDir))
		{
			$storageBaseDir = $us->createTree($storageBaseDir);
			if (!is_dir($storageBaseDir))
			{
				$errorCB->addError("can't create base dir for uploads");
				nxError("can't create base dir for uploads:[$storageBaseDir]");
				return null;
			}
			// $errorCB->addError("can't store uploaded file");
		}
		
		// check if file already exist (shouldn't...)
		if (@is_file($fullPath)) {
			$errorCB->addError('file already exist');
			return null;
       }
		
		// keep the original in case we have to regenerate boxes
		if (!$this->move_uploaded_file($filePath,$fullPath))
		{
			$errorCB->addError("can't store uploaded file: $filePath to $fullPath");
			return null;
		}

		// check that is has been correctly copied
		$res = $gd->getImageSize($fullPath,$errorCB);
		if ($errorCB->isError())
			return null;

		// process boxes and resize image	
		$aboxes = explode(',',trim($boxes,','));
		foreach ($aboxes as $box)
		{
			if ($box=='')
				continue;
				
			// get box details from image.box.$box property
			$boxProp = $this->props->getProperty("image.box.".$box,null);
			if ($boxProp == null)
			{
				// try to get dimensions from box name WxH
				if (preg_match("/^([0-9]+)[xX]([0-9]+)$/", $box, $match) != 0)
				{
					$boxProp = array("w"=>$match[1], "h"=>$match[2]);
				}
				else
				{
					// no box or original => ignore
					continue;
				}
			}

			// resize image and put it in box subdir
			$boxDir = $storageBaseDir . $box ."/";
			if (!is_dir($boxDir))
			{
				nxLog("Creating upload dir for resizing image:".$boxDir);
				$us->createTree($boxDir);
				if (!is_dir($boxDir))
				{
					$errorCB->addError("can't create base dir for uploads");
					nxError("can't create base dir for image resize:[$boxDir]");
					return null;
				}
			}
			
			$boxImgPath = $boxDir . $storageName;
			$gd->resizeImage($fullPath,$boxImgPath,$boxProp["w"],$boxProp["h"],$errorCB);
		}
			
		// return image size
		return $res;
	}

	function delete(&$fileVars,&$errorCB,$checkSubdirs=true)
	{
		if (!isset($fileVars["n"]))
			return false;
			
		$storageName = $fileVars["n"];

		// get base of image directory	
		$storageBaseDir = $this->props->getProperty("pathUpload",null);
		
		// normalise the dir path
		if ($storageBaseDir == null)
		{
			// $errorCB->addError("unknown image dir");
			nxError("unknown image dir");
			return false;
		}

		$storageBaseDir = str_replace("//","/",str_replace("\\","/",$storageBaseDir."/"));
		if (isset($fileVars['dir']))
			$storageBaseDir.="{$fileVars['dir']}/";
		if (!is_dir($storageBaseDir))
		{
			nxError("no image dir");
			// $errorCB->addError("no image dir");
			return false;
		}

		// delete original image in current directory
		$fullPath = $storageBaseDir . $storageName;
		if (is_file($fullPath))
		{
			unlink($fullPath);
		}

		// if no boxes, finished
		if (!$checkSubdirs)
			return true;
		
		// look in all subdirs (boxes) and try to delete the image in them
		if ($dir = opendir($storageBaseDir)) 
		{
			/* This is the correct way to loop over the directory. */
			while (false !== ($subdir = readdir($dir)))
			{
				$d = $storageBaseDir.$subdir;
				if ($subdir!=='.' && $subdir!=='..' && is_dir($d))
				{
					$f = $d.'/'.$storageName;
					if (is_file($f))
						unlink($f);
				}
			}
			closedir($dir);
		}
	}
	
	// clone an existing image and returns its new descriptor.
	// Check subdirectories for other image versions.
	function & clone_it(&$fileVars,&$errorCB,$checkSubdirs=true)
	{
		// get source name
		$storageName = $fileVars["n"];
		
		// get base of image directory	
		$storageBaseDir = $this->props->getProperty("pathUpload",null);
		
		// normalise the dir path
		if ($storageBaseDir == null)
		{
			$errorCB->addError("unknown image dir");
			return null;
		}

		$storageBaseDir = str_replace("//","/",str_replace("\\","/",$storageBaseDir."/"));
		if (isset($fileVars['dir']))
			$storageBaseDir="{$storageBaseDir}{$fileVars['dir']}/";
		if (!is_dir($storageBaseDir))
		{
			$errorCB->addError("no image dir");
			return null;
		}

		// check original image existence
		$fullPath = $storageBaseDir . $storageName;
		if (!is_file($fullPath))
		{
			$errorCB->addError("cant clone missing image file:$$fullPath");
			return null;
		}

		// restore base file _+name+suffix
		$an = explode('.',$storageName);
		$baseName=$an[0];
		$type=$an[1];
		
		$fullPath2='';
		$storageName2='';
		$this->getUniqName($storageBaseDir,$baseName,$type,
				$storageName2,$fullPath2);

		// create new unique name
		//$storageName2 = $this->getUniqueFileName().$baseName;
		//$fullPath2 = $storageBaseDir.$storageName2;

		// duplicate original image
		@copy($fullPath, $fullPath2);
		$fileVars2 = $fileVars;
		$fileVars2["n"]=$storageName2;

		// if no boxes, finished
		if (!$checkSubdirs)
			return $fileVars2;
		
		// look in all subdirs (boxes) and try to copy the image in them
		if ($dir = opendir($storageBaseDir)) 
		{
			/* This is the correct way to loop over the directory. */
			while (false !== ($subdir = readdir($dir)))
			{
				$d = $storageBaseDir.$subdir;
				if ($subdir!=='.' && $subdir!=='..' && is_dir($d))
				{
					$f = $d.'/'.$storageName;
					$f2 = $d.'/'.$storageName2;
					if (is_file($f))
						@copy($f,$f2);
				}
			}
			closedir($dir);
		}
		return $fileVars2;		
	}
}

?>
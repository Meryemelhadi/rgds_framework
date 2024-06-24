<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxPlug_FileStore
{
	var $props;
	function __construct(&$props)
	{
		$this->props =& $props;
	}
	
	function getFileType($type,$externalName)
	{
		switch($type)
		{
			case 'text/plain': $type='txt'; break;
			default:
				// $split = explode( "\.", $externalName);
				// $ext = strtolower($split[ count($split) - 1 ]) ;
                $ext = strtolower(pathinfo($externalName, PATHINFO_EXTENSION));
				switch($ext)
				{
					case 	"jpg" :
					case 	"jpeg" :
						$type='jpeg';
					default:
						$type=$ext;
						break;
				}
		}
		return $type;
	}
	
	function move_uploaded_file($filePath,$fullPath)
	{
		if (move_uploaded_file($filePath,$fullPath))
		{
			$this->chmod($fullPath,0777);
			return true;
		}
		else {
			nxError("operation failed: move_uploaded_file('$filePath','$fullPath')");
			return false;
		}
	}
	
	function chmod($f,$p)
	{
		if (substr(php_uname(), 0, 7) == 'Windows')
			return;
			
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

	// store an uploaded image
	function store( $fieldName, &$errorCB, $storageName=null,
					$isRequired=false, $maxFileUploadSize=50000)
	{
		$fileInfo = & $_FILES[$fieldName];

		if (!$this->checkFile($fieldName,$errorCB, $isRequired,$maxFileUploadSize))
			return null;
		
		// get file from download section
		$externalName = $fileInfo['name'];
		
		// todo support for filenames with several "." ?
		$baseName = current(explode(".",$this->format_filename($externalName)));
			
		$tmpName = $fileInfo['tmp_name'];

		// $mimeType = $fileInfo['type'];
		
		// get file type
		$type = $this->getFileType($fileInfo['type'],$externalName);
		
		// get file name for storage
		if ($storageName == null)
		{
			global $NX_CHARSET;
			if ($NX_CHARSET=='utf-8')
				$baseName = utf8_decode($baseName);

			// $baseName=preg_replace('/[^a-zA-z0-9_]/','_',$baseName);
			$baseName= preg_replace(
						array('/&[^;\\\']+;/','/\W+/','/_$/'),
						array('','_',''),
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
					
		// if a box is defined, resize the image and keep resized version as reference
		$fullPath = $storageBaseDir . $storageName;

		// check/create user directory
		$us = $this->props->getPlugIn("UserSpace");
		
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
		}	
		
		// move file
		if (!$this->move_uploaded_file($tmpName,$fullPath))
		{
			return null;
		}

		// return file details of file for retreival
		$fileVars=array();
		$fileVars["size"] = $fileInfo['size'];
		$fileVars["mime"] = $fileInfo['type'];
		$fileVars["n"] = $storageName;
		$fileVars["title"] = $externalName;
		$fileVars["path"] = $fullPath;
		$fileVars['type'] = $type;
		$fileVars['time'] = time();
		
		if ($GLOBALS['_NX_user'])
		{
			$fileVars['user_service'] = 'user';
			$fileVars['user'] = $GLOBALS['_NX_user']->getProperty('user.userID');
			$fileVars['user_oid'] = $GLOBALS['_NX_user']->getProperty('user.oid');		
		}
		elseif ($GLOBALS['_NX_admin'])
		{
			$fileVars['user_service'] = 'admin';
			$fileVars['user'] = $GLOBALS['_NX_admin']->getProperty('user.userID');
			$fileVars['user_oid'] = $GLOBALS['_NX_admin']->getProperty('user.oid');		
		}
		
		if ($userName!='')
			$fileVars['dir']=$userName;
			
		return $fileVars;
	}
		
	function checkFile($fieldName,&$errorCB,$isRequired=false, $maxFileUploadSize=50000)
	{
		if (!isset($_FILES[$fieldName]))
		{
			if ($isRequired)
				$errorCB->addError($errorCB, "missing");
				
			return false;		
		}

		$fileInfo = $_FILES[$fieldName];
		
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
		

		if (!is_uploaded_file($fileInfo['tmp_name']))
		{
			$errorCB->addError($this->props->getString("failed upload"));
			return false;
		}
		
		return true;
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
			$errorCB->addError("unknown image dir");
			return false;
		}

		$storageBaseDir = str_replace("//","/",str_replace("\\","/",$storageBaseDir."/"));
		if (isset($fileVars['dir']))
			$storageBaseDir.="{$fileVars['dir']}/";
		if (!is_dir($storageBaseDir))
		{
			$errorCB->addError("no image dir");
			return false;
		}

		// delete original image in current directory
		$fullPath = $storageBaseDir . $storageName;
		if (is_file($fullPath))
		{
			unlink($fullPath);
		}

		return true;
	}
	
	// clone an existing file and returns its new descriptor.
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
			$errorCB->addError("no file dir");
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

		return $fileVars2;		
	}
	
	// todo: check map of characters at
	// http://www.unicode.org/Public/MAPPINGS/ISO8859/8859-9.TXT
	// and use tr('\xC0\xC1','aa') etc. or strtr directly from utf8 to filename charsets
	function format_filename($f_utf8)
	{
            $rep = array(
            "%'%"=>'',
            );
        
		   $fichier = utf8_decode($f_utf8);
           if($fichier==$f_utf8)
                 return preg_replace(array_keys($rep),array_values($rep), $fichier); // => not utf8
                 
           $fichier = preg_replace(array_keys($rep),array_values($rep), $fichier);
                                                                                
			// $externalName=utf8_decode($externalName);
	   		$fichier = strtolower(strtr( $fichier,'àâéèêëçîïùüûôö','aaeeeeciiuuuoo'));
			
			return $fichier;
	}

}


?>
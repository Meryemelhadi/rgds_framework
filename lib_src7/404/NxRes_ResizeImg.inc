<?PHP

// automatic deployment of missing resources
class NxRes_ResizeImg
{
	function __construct($props) {
		$this->props=&$props;
		$this->fs=$this->props->getPlugin('FileSys');
	}

	function deployAndSend($url) {
		// get info
		// $name,$type,$skin;
		nxLog('fetching and deploying missing image:'.$url,'404_HDLER');

		$url=urldecode($url);

		// split up the url (ignore any http parameter)
		if (preg_match('#/files/(?:[^/]+)/files/(.*/)(([0-9]+)x([0-9]+))/(([^/]+)[.]([^?]+))$#i',$url,$matches) ||
			preg_match('#^/files/((?:[^/]+).*/)?(([0-9]+)x([0-9]+))/(([^/]+)[.]([^?]+))$#i',$url,$matches))
		{
			list($all,$dir,$box,$w,$h,$name,$basename,$ext)=$matches;
			
			// decode url characters (ex. %20)
			$name=urldecode($name);
			
			// storage
			$storageBaseDir=$this->props->getProperty('pathUpload',NX_DOC_ROOT.'files/').$dir;
			
			// destination file
			$destPath="{$storageBaseDir}{$box}/{$name}";
			
			if (!is_file($destPath))
			{
				// original file name
				$oriPath=$storageBaseDir.$name;

				nxLog("deploying from $oriPath to $destPath...",'404_HDLER');
				if (!is_file($oriPath))
				{
					// unknown image
					nxLog("missing image not found:$url, cant find original:$oriPath",'404_HDLER');
					return false;					
				}
							
				// deploy resource
				if (!$this->resizeImage($oriPath,$destPath,$storageBaseDir,$w,$h,$ext))
				{
					nxError("cant resize image :$oriPath to $destPath, send original instead..",'404_HDLER');
					return $this->sendToBrowser($oriPath,$ext);
				}
				nxLog("image resized",'404_HDLER');

			}
						
			// send it back to browser
			nxLog("send image to browser",'404_HDLER');
			return $this->sendToBrowser($destPath,$ext);
		}	
		
		// standard 404
		nxLog('missing resource not found:'.$url,'404_HDLER');
		return false;
	}
	
	function resizeImage($oriPath,$destPath,$storageBaseDir,$w,$h,$type=null) 
	{
		nxLog("resizing image $oriPath",'404_HDLER');		
		$errorCB = new NxErrorCB();

		$gd = $this->props->getPlugin("ImageGD");
		$us = $this->props->getPlugIn("UserSpace");
					
		// get image size ?
		nxLog("get image size",'404_HDLER');		
		$res = $gd->getImageSize($oriPath,$errorCB);
		if ($errorCB->isError())
		{
			nxError("cant get image size:".$errorCB->getErrorMsg(),'404_HDLER');		
			return null;
		}
		nxLog("got image size",'404_HDLER');		

		// process boxes and resize image	
		// get box details from image.box.$box property
		$box="{$w}x{$h}";

		// resize image and put it in box subdir
		$boxDir = $storageBaseDir . $box ."/";
		nxLog("check dir $boxDir",'404_HDLER');		
		if (!is_dir($boxDir))
		{
			nxLog("creating directory $boxDir for resizing image.. ",'404_HDLER');		
			$us->createTree($boxDir);
			if (!is_dir($boxDir))
				nxError("cant create directory $boxDir for image resize!!",'404_HDLER');		
			else
				nxLog("directory $boxDir created successfully.",'404_HDLER');		
		}
		
		$gd->resizeImage($oriPath,$destPath,$w,$h,$errorCB);
		if ($errorCB->isError())
		{
			nxError("cant get image size:".$errorCB->getErrorMsg(),'404_HDLER');		
			return false;
		}
		else
			return true;
	}
	
	function sendToBrowser($filename,$ext) {
		switch (strtolower($ext)) {
			case "gif": $mime='image/gif';break;
			case "jpeg":$mime='image/jpeg';break;
			case "jpg":$mime='image/jpeg';break;
			case "png": $mime='image/png';break;
			case "css": 
			case "csx": $mime='text/css';break;
			case "js": 
			case "jsx": $mime='text/javascript';break;
			default:return false;
		}
		
		header("content-type: $mime");
		header("Content-Length: ".filesize($filename));		
		$fp = fopen($filename, 'r');
		fpassthru($fp);
		fclose($fp);
		nxLog("Resource sent back to browser:$filename [$mime]",'404_HDLER');
		return true;
//		exit;
	}
}

?>
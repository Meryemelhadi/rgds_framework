<?PHP

require_once(NX_LIB.'view/NxApp_View.inc');


// automatic deployment of missing resources
class NxRes_skinRes
{
	function __construct($props) {
		$this->props=&$props;
		$this->fs=$this->props->getPlugin('FileSys');
	}
	
	function deployAndSend($url) {
		// get info
		// $name,$type,$skin;
		nxLog('fetching and deploying missing resource:'.$url,'404_HDLER');
		
		// split up the url (ignore any http parameter)
		if (preg_match('#/nx/skins/([^/]+)/([^/]+)/(.+)[.]([^.?]+)#i',$url,$matches))
		{
			list($all,$skin,$typeRes,$name,$ext)=$matches;
			
			// init required skin
			$this->initSkin($skin);
			
			// deploy resource
			$f=$this->deployRes($name,$skin,$typeRes,$ext);
			
			// send it back to browser
			return $this->sendToBrowser($f,$typeRes,$ext);
		}	
		
		// standard 404
		nxLog('missing resource not found:'.$url,'404_HDLER');
		return false;
	}
	
	function deployRes($name,$skin,$typeRes,$ext) {
		global $NX_SKIN_PATH;
		
		$deployPath=NX_RES."skins/{$skin}/{$typeRes}/{$name}.{$ext}";
		
		$name=preg_replace('/'._NX_DEVICE_MAP.'$/','',$name);
        
        $names= explode('/',$name);
        if(count($names)>1)
        {
            $package= $names[0];
            $skin = NX_SKIN;
            $packPath[] = GENERIS_PACKAGES."{$package}/view/{$skin}/";
            $packPath[] = GENERIS_PACKAGES."{$package}/view/default/";
            $NX_SKIN_PATH = $packPath+$NX_SKIN_PATH;
        }
		
		// file not found => search it in source directories and compile it
		foreach ($NX_SKIN_PATH as $srcRoot)
		{
			// in current skin
			if ($f=$this->copyRes("{$srcRoot}res/","{$typeRes}/$name",$ext,_NX_DEVICE_MAP,$deployPath))
				return $f;

			if ($f=$this->copyRes($srcRoot,"{$typeRes}/$name",$ext,_NX_DEVICE_MAP,$deployPath))
				return $f;
		}
		
		// not found
		nxLog("can't find required resource:$deployPath",'404_HDLER');
		return null;
	}
	
	// search for a view as tpl file or php script (with or without device)
	// if tpl, compile it, otherwise only copy it.
	function copyRes($srcRoot,$name,$ext,$device,$dest)
	{
		// prefix for resources in skin
		// $res="res/";

		$base = $srcRoot.$name;
		nxLog("Trying resource file: ${base}{$device}.{$ext}",'404_HDLER');
		nxLog("Trying resource file: ${base}.{$ext}",'404_HDLER');
		
		// no template =>search for plain files
		if (is_file($f="${base}{$device}.{$ext}") ||
			is_file($f="${base}.{$ext}"))
		{
			// optimistic: assume dir existence
			if ($this->fs->copy($f, $dest))
			{
				nxLog("deployed resource from $f to $dest","404_HDLER",NX_COMPILE_EVENT);
				return $dest;
			}
				
			// failed: check directory
			if (!is_dir($d=dirname($dest)))
			{
				// create dir
				if ($this->fs->createTree($d))
				{
					// try again to copy the file
					if ($this->fs->copy($f, $dest))
					{
						nxLog("imported view from $f to $dest","404_HDLER",NX_COMPILE_EVENT);
						return $dest;
					}
				}
			}

			// can't copy file => log and run view from source
			nxError("can't copy file:$f to $dest.",'404_HDLER');
			return $f;
		}
		
		return false;
	}
		
	function initSkin($skin,$skinDeft='default') {
		global $NX_SKIN,$NX_SKIN_DFT,$NX_SKIN_COMP,$NX_SKIN_PATH,$NX_SKIN_URL,$NX_DEFT_SKIN_URL,
				$NX_IMAGES,$NX_SKIN_RES,$NX_DEFT_SKIN_RES;

		if (isset($NX_SKIN)) 
			return;

		// $NX_SKIN=$this->getProperty('page.skin',$skin);
		$NX_SKIN=$skin;
		$NX_SKIN_DFT=$skinDeft;
		$NX_DIS_SKIN_ROOT=NX_DIS_CONF.'view/default/';

		// compiled views storage
		$NX_SKIN_COMP=NX_GEN."view/{$NX_SKIN}/";
		

		// standard paths
		if (defined('NX_SKIN_ROOTS'))
			$skinRoots = explode(',',NX_SKIN_ROOTS);
		else
			$skinRoots[] = NX_CONF;

		foreach($skinRoots as $skinRoot)
		{
			$skinRootV=$skinRoot."view/";
			$paths[] = array(
				"{$skinRootV}{$NX_SKIN}/",	// current skin root
			);
		}
		foreach($skinRoots as $skinRoot)
		{
			$skinRootV=$skinRoot."view/";
			$paths[] = array(
				"{$skinRootV}{$NX_SKIN_DFT}/" // default skin root
			);
		}

		$paths[] = array(
			NX_DIS_CONF."view/{$NX_SKIN_DFT}/" // distrib skin root
		);
		if ($NX_SKIN_PATH)
			$paths[] = $NX_SKIN_PATH;

		$NX_SKIN_PATH=call_user_func_array('array_merge',$paths);


/*
		$NX_SKIN_PATH=array();
		$skinRoot=NX_CONF.'view/';
			
		$NX_SKIN_PATH=array(
			"{$skinRoot}{$NX_SKIN}/",	// current skin root
			"{$skinRoot}{$NX_SKIN_DFT}/", // default skin root
			NX_DIS_CONF."view/{$NX_SKIN_DFT}/" // distrib skin root
		);
*/

		// --- URLs for skins
		$NX_SKIN_URL=NX_RES_URL."skins/{$NX_SKIN}/";
		$NX_DEFT_SKIN_URL=NX_RES_URL."skins/{$NX_SKIN_DFT}/";
		$NX_IMAGES="{$NX_SKIN_URL}images/";
	
		//---  deployment of resources on server
		// current skin
		$NX_SKIN_RES=NX_RES."skins/{$NX_SKIN}/";
		// default skin
		$NX_DEFT_SKIN_RES=NX_RES."skins/{$NX_SKIN_DFT}/";	
	}	
	
	function sendToBrowser($filename,$typeRes,$ext) {
		if (!$fp)
		{
			nxError("no filename for sending to brower");
			return false;
		}

		switch ($ext) {
			case "gif": $mime='image/gif';break;
			case "jpg":$mime='image/jpeg';break;
			case "jpeg":$mime='image/jpeg';break;
			case "png": $mime='image/png';break;
			case "css": 
			case "csx": $mime='text/css';break;
			case "js": 
			case "jsx": $mime='text/javascript';break;
			case "xml": $mime='text/xml';break;
			default:return false;
		}
		
		header("content-type: $mime");
		header("Content-Length: ".filesize($filename));		
		$fp = fopen($filename, 'r');
		if (!$fp)
		{
			nxError("Can't read file $filename");
			return false;
		}
		fpassthru($fp);
		fclose($fp);
		nxLog("Resource sent back to browser:$filename [$mime]",'404_HDLER');
		return true;
//		exit;
	}
}

?>
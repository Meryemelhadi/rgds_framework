<?PHP

// automatic generation of image button.
// get de 
/*
	<a href="{=submit.url}" title="Save"><img src="{@NX_IMAGES}/buttons/mac_blue/Save.gif" />
	
	and

	NX_CONF/views/NX_SKIN/images/buttons/mac_blue/desc.inc
	
	background image is stored in 
	
	NX_CONF/views/NX_SKIN/images/buttons/mac_blue/
	
	contains:
	
	$properties=array(
		'background_image' => "spacer.gif",
		'sIconL' => "",
		'sIconR' => "",
	
		'margin_top' => 0,
		'margin_bottom' => 0,
		'margin_left' => 15 ,
		'margin_right' => 15,
			
		'text' => 'Demo Text',
		'font_family' => 'arial',
		'font_size' => 12,
		'color' => "#000000"
	);
*/
class NxRes_skinButton
{
	function __construct($props) {
		$this->props=&$props;
		$this->fs=$this->props->getPlugin('FileSys');
	}
	
	function deployAndSend($url,$skin,$desc,$name,$ext) {
		global $NX_SKIN_RES,$NX_DEFT_SKIN_RES;

		// get info
		// $name,$type,$skin;
		nxLog('fetching and deploying missing image:'.$url,'404_HDLER');

		// init required skin
		$this->initSkin($skin);
		
		// storage dir for button
		$storageBaseDir=$NX_SKIN_RES."images/buttons/$desc/";
		
		// destination file
		$destPath="{$storageBaseDir}{$name}.{$ext}";
				
		if (NX_CHECK_TPL=='compile' || !is_file($destPath))
		{			
			// load button descriptor
			
			$NX_SKIN_DIR=NX_VIEW."{$skin}/";
			$NX_DEFT_SKIN_DIR=NX_VIEW."default/";
			$properties=array();
			if (is_file($descpath=$NX_SKIN_DIR."res/images/buttons/$desc/$desc.inc") ||
				is_file($descpath=$NX_DEFT_SKIN_DIR."res/images/buttons/$desc/$desc.inc"))
			{
				$bgImageDir=dirname($descpath).'/';
				include($descpath);
			}
			else
			{
				// cant find the descriptor, backup on default skin (might find a spacer.gif file..)
				$bgImageDir=$NX_DEFT_SKIN_RES.'res/images/';			
			}
			
			// add url parameters to button descriptor
			$properties = $_GET+$properties;
			
			// add storage dir?
			if (!is_dir($storageBaseDir))
			{
				if ($this->fs->createTree($storageBaseDir)==null)
					nxError("cant create directory $storageBaseDir for button $destPath",'404_HDLER');		
				else
					nxLog("directory $storageBaseDir created successfully.",'404_HDLER');		
					
			}
			
			// gen button
			include(NX_LIB.'button/TTFButton.inc');
			$btn = new TTFButton( $name, NX_CONF.'fonts/', $bgImageDir, $properties) ;
			$btn->show($destPath);
		}
					
		// send it back to browser
		return $this->sendToBrowser($destPath,$ext);
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
		
		$NX_SKIN_PATH=array();
		$skinRoot=NX_CONF.'view/';
			
		$NX_SKIN_PATH=array(
			"{$skinRoot}{$NX_SKIN}/",	// current skin root
			"{$skinRoot}{$NX_SKIN_DFT}/", // default skin root
			NX_DIS_CONF."view/{$NX_SKIN_DFT}/" // distrib skin root
		);
			
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
	
}

?>
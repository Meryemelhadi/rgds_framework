<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
/** Data field : file upload */
class NxDataField_file extends NxData_Field
{
	var $url;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}

	function clear()
	{
		$this->url = null;	
	}

	function & toObject($opts=null)
	{ 
		return $this->url; 
	}

	function readFromStore(&$v){
		$this->url = $v;
	}

	function readFromDB($recData)
	{
		$this->url = $this->getFieldData($recData,$this->getFName(),null);
	}

	function toDB($opts=null) 
	{
		if (isset($this->url))
		{
			return "'".htmlspecialchars($this->url,ENT_QUOTES)."'";
		}
		else
		{
			return "NULL";
		}
	}

	function toHTML($opts=null)
	{
		if (isset($this->url) && $this->url != null)
		{
			return "<a target=\"_blank\" href=\"".$this->url."\">".$this->desc->getLabel()."</a>";
		}
		else
		{
			return "&nbsp;";
		}
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;		
		if (NX_IMAGE_LIB==='none')
			$control =& $desc->getControl($desc->getProperty('control','file'));
		else
			$control =& $desc->getControl($desc->getProperty('control','file'));
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
		$control =& $desc->getControl($desc->getProperty('control','file'));
		return $control->readForm($this,$recData,$errorCB);
	}

	function toForm_old($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return "";

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$this->url."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$this->url."\" />";			
			
		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$size=intval($desc->getProperty("size",$colsMaxBox));
		return "<input class=\"file form-control\" type=\"file\" name=\"".$desc->name."\" size=\"".$size."\" ".$desc->getStyle("input","file"). " />";
	}
	

	/*
	<form ENCTYPE="multipart/form-data" method="POST" action="ultest.php?act=upload">
		File:<INPUT TYPE="FILE" NAME="userfile" SIZE="35">
		<input type="hidden" name="MAX_FILE_SIZE" value="1000000">
		<input type="submit" value="Upload" name="B1">
		Please click only <b>once</b> and wait for confirmation
	</form>
	*/

	function _readFromForm_old(&$recData,&$errorCB)
	{							
		$this->maxFileUploadSize = $this->desc->getProperty("maxFileUploadSize", 1000000);
		
		$fieldName = $this->desc->name;
		
		// check if files has been uploaded
		if (!$this->checkFile($fieldName,$errorCB))
		{
			echo "<!-- readFromForm: checkFile failed -->";
			$this->url = null;	
			return !$this->desc->isRequired();
		}

		echo "<!-- readFromForm: checkFile ok -->";

		$fileInfo = & $_FILES[$fieldName];
		$fileStore = $this->desc->getPlugIn("FileStore");
		if ($fileStore != null)
		{
			echo "<!-- fileStore exists -->";
			$this->url =  $fileStore->store($fileInfo);
			if ($this->url == null)
			{
				echo "<!-- fileStore upload failed -->";
				return false;
			}
			echo "<!-- fileStore upload ok -->";

		}
 		
		return true;
	}
	
	function checkFile($fieldName,&$errorCB)
	{
		if (!isset($_FILES[$fieldName]))
		{	
			if ($this->desc->isRequired())
				$this->addError($errorCB, "missing");
				
			return false;		
		}

		// echo "<!-- isset(_FILES[$fieldName] -->";

		$fileInfo = & $_FILES[$fieldName];
		
		if ($fileInfo['size'] == 0)
		{
			if ($this->desc->isRequired())
				$this->addError($errorCB, "missing file");
				
			return false;		
		}
		if ($fileInfo['size'] > $this->maxFileUploadSize)
		{
			$this->addError("file too big",$this->maxFileUploadSize);
			return false;
		}
		
		if (!in_array($fileInfo['type'],$this->fileTypes))
		{
			$this->addError($errorCB, "invalid file type");
			return false;
		}

		if (!is_uploaded_file($fileInfo['tmp_name']))
		{
			$this->addError($errorCB, "failed upload");
			return false;
		}

		// echo "<!-- checkfile ok -->";
		
		return true;
	}
	
	/*
		If you're on a shared host that won't allow you to modify the php.ini file
		and your max_upload_size is set low, you can create an .htaccess file in 
		the upload handler script directory that contains:
			php_value upload_max_filesize 8000000
		
	*/
}

?>
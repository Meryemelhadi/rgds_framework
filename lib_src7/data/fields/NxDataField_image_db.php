<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxDataField_image_db extends NxData_Field
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

	function toStatus() { 
		return isset($this->errorMsg)?'error': 
			(empty($this->info["n"])?'empty':'ok');
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
		$fileStore = $this->desc->getPlugIn("ImageStore");
		$fileStore->delete($this->info,$errorCB,true);
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
		if (isset($this->info))
		{
//			return "'".htmlspecialchars($this->serialise($this->info),ENT_QUOTES)."'";
			return "'".$this->serialise($this->info)."'";
		}
		else
		{
			return "NULL";
		}
	}

	function toHTML($opts=null)
	{
		$this->type='html';
		$this->box=null;
		$this->style='';
		if ($opts!=null)
			preg_replaceX(array(
				'/(left|right)/ie',
				'/(middle)/ie',
				'/(js|html|url|string)/ie',
				'/(original|[0-9]+x[0-9]+)/ie'),
				array(
					' $this->style="float:$1"',
					' $this->style="clear:both"',
					'$this->type="$1"',
					'$this->box="$1"'),
				$opts,$this);

		switch($this->type)
		{
			case 'url':
				return $this->_getUrl($this->box);
			case 'js':
				return $this->toJS('\'',$this->box);
			default:
				// optimisation: store url in db ?
				if (isset($this->info['n']))
					return "<img{$this->style} border=\"0\" src=\"".$this->_getUrl($this->box)."\">";
				else
					return "&nbsp;";
		}
	}

	function _getUrl()
	{
		if (isset($this->info["n"]))
		{
			$url = $this->desc->getProperty("baseUrlUpload","/images/");
			$box = $this->desc->getProperty("imageSize",null,false);
			if ($box != null && $box != "original")
				$url .= $box . "/";
			return $url . $this->info["n"];
		}
		else
			return null;
	}
	
	function toJS($opts='\'')
	{
		if ($opts!='')
			$sep=$opts;
		else
			$sep='\'';

		if (!isset($this->info["n"]))
			return 'error:"no image"';

		$url = $this->desc->getProperty("baseUrlUpload","/images/");
		$box = $this->desc->getProperty("imageSize",null,false);
		if ($box != null && $box != "original")
			$url .= $box . "/";
		$name=$this->info["n"];
			
		return "name:$sep$name$sep,url:$sep$url" . $this->info["n"]."$sep,box:$sep$box$sep";
	}
	
	function toForm($opts=null) 
	{
		$desc = & $this->desc;
		$url = $this->_getUrl();
		$objUrl = $this->serialise($this->info);

		$fname=$desc->getFormName();
		$F_ = $desc->name;
		$L_ = $this->getLabel();
		
		if (!$desc->isInput())
			return "";

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$url."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value=\"".$url."\" />";			
			
		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$size=intval($desc->getProperty("size",$colsMaxBox));
		
		if ($url == null || $url == "")
			return "<input class=\"file_image form-control\" type=\"file\" name=\"".$fname."\" size=\"".$size."\" ".$desc->getStyle("input","file"). " />";
		
		// update
		
		// todo: manage multiple records?
			
		$src= <<<EOH
<span class="image_update_box">
<input type="radio" value="keep" class="radio_image form-control" 
	id="{$F_}_image_choice_keep" name="{$F_}_image_choice" 
	onclick="Nx_onKeepImage({$F_}_image_choice_image,{$F_}_image_choice_file)" CHECKED="true">
	<span class="radio_image form-control">keep image</span>

<img class="file_image" id="{$F_}_image_choice_image" border="0" height="70" style="float:right" 
	srcKeep="{$url}" src="{$url}" alt="{$L_}" />

<input  type="radio" value="replace" class="radio_image form-control" 
	id="{$F_}_image_choice_replace" name="{$F_}_image_choice" 
	onclick="Nx_onReplaceImage({$F_}_image_choice_image,{$F_}_image_choice_file)">
	<span class="radio_image form-control">replace image</span>

<input type="radio" value="delete"	class="radio_image form-control" 
	id="{$F_}_image_choice_delete" name="{$F_}_image_choice" 
	onclick="Nx_onDeleteImage({$F_}_image_choice_image,{$F_}_image_choice_file)">
	<span class="radio_image form-control">delete image</span>
<input type="radio" value="db"	class="radio_image form-control" 
	id="image_image_choice_db" name="image_image_choice" 
	onclick="Nx_onDBImage(image_image_choice_image,image_image_choice_file)">
	<span class="radio_image form-control">image database</span>
<br>	
<input	type="file" class="file_image_replace" name="{$F_}" id="{$F_}_image_choice_file" size="{$size}" style="visibility:hidden" onpropertychange="Nx_onFileChange(this,{$F_}_image_choice_image)"
	onchange="Nx_onFileChange(this,{$F_}_image_choice_image)" onclick="Nx_onFileChange(this,{$F_}_image_choice_image)" onblur="Nx_onFileChange(this,{$F_}_image_choice_image)" />
	<input type="hidden" name="{$desc->name}_save" value="{$objUrl}" />
</span>
EOH;

		return $src;
		
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
		$choiceName = $desc->name . "_image_choice";

		if (!isset($_POST[$choiceName]))
		{
			$choice = "new";
		}
		else
		{
			$choice = $_POST[$choiceName];
		}

		switch($choice)
		{
			case "delete":
				$fileStore = $this->desc->getPlugIn("ImageStore");
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$fileVars = $this->unserialise($_POST[$savedUrlField]);
					$fileStore->delete($fileVars,$errorCB,true);
				}
				else
				{
					$this->addError($errorCB, "field error");
				}

				$this->info = null;
				break;
				
			case "keep":
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$this->info = $this->unserialise($_POST[$savedUrlField]);
					
				}
				else
				{
					$this->addError($errorCB, "field error");
					$this->info = null;
				}
				break;
				
			case "replace":
				$fileStore = $this->desc->getPlugIn("ImageStore");

				// get image parameters
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$this->info = $this->unserialise($_POST[$savedUrlField]);
				}
				else
				{
					$this->addError($errorCB, "field error");
					$this->info = null;
				}
				
				// delete current file(s)
				$fileStore->delete($this->info,$errorCB,true);
				
				// get uploaded file, resize it if needed and store it in required upload directory
				// with same name as previous
				$this->info =  $fileStore->store($desc->name,
								//$errorCB, $this->info["n"],
								$errorCB, null,
								$this->desc->getProperty("imageSizes",null), // csv list of boxes
								$this->desc->isRequired(), $this->getProperty("maxFileUploadSize",100000));
								
				break;
			case "new":
			default:
				$fileStore = $this->desc->getPlugIn("ImageStore");
				
				// get uploaded file, resize it if needed and store it in required upload directory
				$this->info =  $fileStore->store($desc->name,
								$errorCB,null,
								$this->desc->getProperty("imageSizes",null),
								$this->desc->isRequired(), $this->getProperty("maxFileUploadSize",100000));
				break;
		}
		 		
		return true;
	}
}

?>
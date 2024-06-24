<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_image
{
	function NxControl_image()
	{
	}
	
	function toHTML($field,$desc,$value,$opts=null)
	{
		$type='html';
		$box=null;
		if ($opts!=null)
			preg_replaceX(array('/(js|html|url|string)/ie','/(original|[0-9]+x[0-9]+)/ie'),
				array('$type="$1"','$box="$1"'),$opts);
				
		switch($type)
		{
			case 'url':
				return $field->_getUrl($box);
			case 'js':
				return $field->toJS('\'',$box);
			default:
				// optimisation: store url in db ?
				if (isset($field->info['n']))
					return "<img border=\"0\" src=\"".$field->_getUrl($box)."\">";
				else
					return "&nbsp;";
		}
	}
	
	function toForm($field,$desc,$value) 
	{
		$url = $field->_getUrl();
		$objUrl = $field->serialise($value);

		if (!$desc->isInput())
			return "";

			if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$url."\" />";

		if (!$desc->isEdit())
			return $field->toHTML()."<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$url."\" />";			
			
		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$size=intval($desc->getProperty("size",$colsMaxBox));
		
		if ($url == null || $url == "")
			return "<input class=\"file_image\" type=\"file\" name=\"".$desc->name."\" size=\"".$size."\" ".$desc->getStyle("input","file"). " />";
		
		// update
		$F_ = $desc->name;
		$L_ = $field->getLabel();
		
		// get strings
		$strings=$desc->getChoiceList('form_fields',null,'strings');
		$s_k=$strings->getString('keep image');
		$s_r=$strings->getString('replace image');
		$s_d=$strings->getString('delete image');
		$s_i=$strings->getString('image database');
		
		$src= <<<EOH
<span class="image_update_box">
<input type="radio" value="keep" class="radio_image" 
	id="{$F_}_image_choice_keep" name="{$F_}_image_choice" 
	srcKeep="{$url}" onclick="Nx_onKeepImage(this,{$F_}_image_choice_image,{$F_}_image_choice_file)" CHECKED="true">
	<span class="radio_image">{$s_k}</span>

<img class="file_image" id="{$F_}_image_choice_image" border="0" height="70" style="float:right" 
	srcKeep="{$url}" src="{$url}" alt="{$L_}" />

<input  type="radio" value="replace" class="radio_image" 
	id="{$F_}_image_choice_replace" name="{$F_}_image_choice" 
	onclick="Nx_onReplaceImage({$F_}_image_choice_image,{$F_}_image_choice_file)">
	<span class="radio_image">{$s_r}</span>

<input type="radio" value="delete"	class="radio_image" 
	id="{$F_}_image_choice_delete" name="{$F_}_image_choice" 
	onclick="Nx_onDeleteImage({$F_}_image_choice_image,{$F_}_image_choice_file)">
	<span class="radio_image">{$s_d}</span>
<!-- input type="radio" value="db"	class="radio_image" 
	id="image_image_choice_db" name="image_image_choice" 
	onclick="Nx_onDeleteImage(image_image_choice_image,image_image_choice_file)">
	<span class="radio_image">{$s_i}</span -->
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
	function readForm(&$field,$desc,&$recData,&$errorCB)
	{
		$choiceName = $desc->name . "_image_choice";

		if (!isset($_POST[$choiceName]))
		{
			$choice = "new";
		}
		else
		{
			$choice = $_POST[$choiceName];
		}

		// get strings
		$strings=$desc->getChoiceList('FieldErrors',null,'strings');
		$s_e=$strings->getString('field error');

		switch($choice)
		{
			case "delete":
				$fileStore = $desc->getPlugIn("ImageStore");
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$fileVars = $field->unserialise($_POST[$savedUrlField]);
					// only delete if not from db
					if(!(isset($fileVars['is_db'])&&$fileVars['is_db']))
						$fileStore->delete($fileVars,$errorCB,true);
				}
				else
				{
					$field->addError($errorCB,$s_e);
				}

				$field->info = null;
				break;
				
			case "db":
			case "keep":
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$field->info = $field->unserialise($_POST[$savedUrlField]);
					
				}
				else
				{
					$field->addError($errorCB,$s_e);
					$field->info = null;
				}
				break;
				
			case "replace":
				$fileStore = $desc->getPlugIn("ImageStore");

				// get image parameters
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$field->info = $field->unserialise($_POST[$savedUrlField]);
				}
				else
				{
					$field->addError($errorCB,$s_e);
					$field->info = null;
				}
				
				// delete current file(s)
				$fileStore->delete($field->info,$errorCB,true);
				
				// get uploaded file, resize it if needed and store it in required upload directory
				// with same name as previous
				$field->info =  $fileStore->store($desc->name,
								//$errorCB, $field->info["n"],
								$errorCB, null,
								$desc->getProperty("imageSizes",null), // csv list of boxes
								$desc->isRequired(), $field->getProperty("maxFileUploadSize",100000));
								
				break;
			case "new":
			default:
				$fileStore = $desc->getPlugIn("ImageStore");
				
				// get uploaded file, resize it if needed and store it in required upload directory
				$field->info =  $fileStore->store($desc->name,
								$errorCB,null,
								$desc->getProperty("imageSizes",null),
								$desc->isRequired(), $field->getProperty("maxFileUploadSize",100000));
				break;
		}
		 		
		return true;
	}

}

?>
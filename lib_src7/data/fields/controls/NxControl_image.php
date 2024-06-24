<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_image
{
	function __construct()
	{
	}
	
	function toHTML($f,$value,$opts=null)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
				
		$this->type='html';
		$this->box=null;
		if ($opts!=null)
			preg_replaceX(array('/(js|html|url|string)/ie','/(original|[0-9]+x[0-9]+)/ie'),
				array('$this->type="$1"','$this->box="$1"'),$opts,$this);
				
		switch($this->type)
		{
			case 'url':
				return $f->_getUrl($this->box);
			case 'js':
				return $f->toJS('\'',$this->box);
			default:
				// optimisation: store url in db ?
				if (isset($f->info['n']))
					return "<img border=\"0\" src=\"".$f->_getUrl($this->box).'" alt="" />';
				else
					return "&nbsp;";
		}
	}
	
	function toForm($f,$value) 
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
		$url = $f->_getUrl();
		$objUrl = $f->serialise($value);

		if (!$desc->isInput())
			return "";

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value=\"".$url."\" />";

		if (!$desc->isEdit())
			return $f->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value=\"".$url."\" />";			
			
		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30,false));
		$size=60;//intval($desc->getProperty("size",$colsMaxBox,false));
		
		if ($url == null || $url == "")
			return "<input class=\"file_image\" type=\"file\" name=\"".$fname."\" size=\"".$size."\" ".$desc->getStyle("input","file"). " />";
		
		// update
		$F_ = $fname;
		$L_ = $f->getLabel();
		
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
	srcKeep="{$url}" onclick="Nx_onKeepImage(this,$('{$F_}_image_choice_image'),$('{$F_}_image_choice_file'))" CHECKED="true">
	<span class="radio_image">{$s_k}</span>

<img class="file_image" id="{$F_}_image_choice_image" border="0" height="70" style="float:right" 
	srcKeep="{$url}" src="{$url}" alt="{$L_}" />

<input  type="radio" value="replace" class="radio_image" 
	id="{$F_}_image_choice_replace" name="{$F_}_image_choice" 
	onclick="Nx_onReplaceImage($('{$F_}_image_choice_image'),$('{$F_}_image_choice_file'))">
	<span class="radio_image">{$s_r}</span>

<input type="radio" value="delete"	class="radio_image" 
	id="{$F_}_image_choice_delete" name="{$F_}_image_choice" 
	onclick="Nx_onDeleteImage($('{$F_}_image_choice_image'),$('{$F_}_image_choice_file'))">
	<span class="radio_image">{$s_d}</span>
<br>	
<input	type="file" class="file_image_replace" name="{$F_}" id="{$F_}_image_choice_file" size="{$size}" style="visibility:hidden" onpropertychange="Nx_onFileChange(this,$('{$F_}_image_choice_image'))"
	onchange="Nx_onFileChange(this,$('{$F_}_image_choice_image'))" onclick="Nx_onFileChange(this,$('{$F_}_image_choice_image'))" onblur="Nx_onFileChange(this,$('{$F_}_image_choice_image'))" />
	<input type="hidden" name="{$fname}_save" value="{$objUrl}" />
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
	function readForm(&$f,&$recData,&$errorCB)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		
		$choiceName = $fname . "_image_choice";

		if (!isset($recData[$choiceName]))
		{
			$choice = "new";
		}
		else
		{
			$choice = $recData[$choiceName];
		}

		// get strings
		$strings=$desc->getChoiceList('FieldErrors',null,'strings');
		$s_e=$strings->getString('field error');

		switch($choice)
		{
			case "delete":
				$fileStore = $desc->getPlugIn("ImageStore");
				$savedUrlField = $fname."_save";
				if (isset($recData[$savedUrlField]))
				{
					$fileVars = $f->unserialise($recData[$savedUrlField]);
					// only delete if not from db
					if(!(isset($fileVars['is_db'])&&$fileVars['is_db']))
						$fileStore->delete($fileVars,$errorCB,true);
				}
				else
				{
					$f->addError($errorCB,$s_e);
				}

				$f->info = null;
				break;
				
			case "db":
			case "keep":
				$savedUrlField = $fname."_save";
				if (isset($recData[$savedUrlField]))
				{
					$f->info = $f->unserialise($recData[$savedUrlField]);
					
				}
				else
				{
					$f->addError($errorCB,$s_e);
					$f->info = null;
				}
				break;
				
			case "replace":
				$fileStore = $desc->getPlugIn("ImageStore");

				// get image parameters
				$savedUrlField = $fname."_save";
				if (isset($recData[$savedUrlField]))
				{
					$f->info = $f->unserialise($recData[$savedUrlField]);
				}
				elseif(!isset($f->info['n']))
				{
					$f->addError($errorCB,$s_e);
					$f->info = null;
				}
				
				// delete current file(s)
				$fileStore->delete($f->info,$errorCB,true);
				
				// get uploaded file, resize it if needed and store it in required upload directory
				// with same name as previous
				$f->info =  $fileStore->store($fname,
								//$errorCB, $f->info["n"],
								$errorCB, null,
								$desc->getProperty("imageSizes",null), // csv list of boxes
								$desc->isRequired(), $f->getProperty("maxFileUploadSize",100000));
								
				break;
			case "new":
			default:
				$fileStore = $desc->getPlugIn("ImageStore");
				
				// get uploaded file, resize it if needed and store it in required upload directory
				$f->info =  $fileStore->store($fname,
								$errorCB,null,
								$desc->getProperty("imageSizes",null),
								$desc->isRequired(), $f->getProperty("maxFileUploadSize",100000));
				break;
		}
		 		
		return true;
	}

}

?>
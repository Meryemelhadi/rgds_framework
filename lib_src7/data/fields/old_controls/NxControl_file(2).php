<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_file
{
	function __construct()
	{
	}
	
	function toHTML()
	{
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
			return "<input class=\"file\" type=\"file\" name=\"".$desc->name."\" size=\"".$size."\" ".$desc->getStyle("input","file"). " />";
		
		// update
		$F_ = $desc->name;
		$L_ = $field->getLabel();
		if (isset($field->info['n']))
			$N_=$field->info['n'];
		else
			$N_= '';
		
		$src= <<<EOH
<span class="image_update_box">
<input type="radio" value="keep" class="radio_file" 
	id="{$F_}_file_choice_keep" name="{$F_}_file_choice" 
	srcKeep="{$url}" onclick="Nx_onKeepFile(this,{$F_}_file_choice_file,{$F_}_file_choice_file)" CHECKED="true">
	<span class="radio_file">keep file</span>

<input  type="radio" value="replace" class="radio_file" 
	id="{$F_}_file_choice_replace" name="{$F_}_file_choice" 
	onclick="Nx_onReplaceFile({$F_}_file_choice_file,{$F_}_file_choice_file)">
	<span class="radio_file">replace file</span>

<input type="radio" value="delete"	class="radio_file" 
	id="{$F_}_file_choice_delete" name="{$F_}_file_choice" 
	onclick="Nx_onDeleteFile({$F_}_file_choice_file,{$F_}_file_choice_file)">
	<span class="radio_file">delete file</span>
<!-- input type="radio" value="db"	class="radio_file" 
	id="file_choice_db" name="file_choice" 
	onclick="Nx_onDeleteFile(file_choice_file,file_choice_file)">
	<span class="radio_file">file database</span -->
<br>	
<div class="file" styleX="float:right" srcKeep="{$url}">
<a class="file" target="_blank" href="{$url}">{$L_}: {$N_}</a>
</div>
<input	type="file" class="file_replace" name="{$F_}" id="{$F_}_file_choice_file" size="{$size}" style="visibility:hidden" onpropertychange="Nx_onFileChange(this,{$F_}_file_choice_file)"
	onchange="Nx_onFileChange(this,{$F_}_file_choice_file)" onclick="Nx_onFileChange(this,{$F_}_file_choice_file)" onblur="Nx_onFileChange(this,{$F_}_file_choice_file)" />
	<input type="hidden" name="{$desc->name}_save" value="{$objUrl}" />
</span>
EOH;

		return $src;
		
	}

	// dependedncy on properties:
	// "fileSizes" : name of the box to which the file should be resized to, 
	// otherwise fetches its dimension array in:
	// "file.box.100x100" => array("w"=>100,"h"=>100) or
	// "file.box.thumb" => array("w"=>10,"h"=>10)
	// otherwise if name of the form 100x40, use it as dimensions.
	// If no box or not found, no resize is done (simple copy)
	// NB. use GD for file resizing.
	// Returns file object as:
	// 	"url,w,h,type"
	function readForm(&$field,$desc,&$recData,&$errorCB)
	{
		$choiceName = $desc->name . "_file_choice";

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
				$fileStore = $desc->getPlugIn("FileStore");
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$fileVars = $field->unserialise($_POST[$savedUrlField]);
					$fileStore->delete($fileVars,$errorCB,true);
				}
				else
				{
					$field->addError($errorCB, "field error");
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
					$field->addError($errorCB, "field error");
					$field->info = null;
				}
				break;
				
			case "replace":
				$fileStore = $desc->getPlugIn("FileStore");

				// get file parameters
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$field->info = $field->unserialise($_POST[$savedUrlField]);
				}
				else
				{
					$field->addError($errorCB, "field error");
					$field->info = null;
				}
				
				// delete current file(s)
				$fileStore->delete($field->info,$errorCB,true);
				
				// get uploaded file, resize it if needed and store it in required upload directory
				// with same name as previous
				$field->info =  $fileStore->store($desc->name,
								//$errorCB, $field->info["n"],
								$errorCB, null,
								$desc->isRequired(), $field->getProperty("maxFileUploadSize",100000));
								
				$field->info['url']=$field->_getUrl();
				break;
			case "new":
			default:
				$fileStore = $desc->getPlugIn("FileStore");
				
				// get uploaded file, resize it if needed and store it in required upload directory
				$field->info =  $fileStore->store($desc->name,
								$errorCB,null,
								$desc->isRequired(), $field->getProperty("maxFileUploadSize",100000));
				$field->info['url']=$field->_getUrl();
				break;
		}
		 		
		return true;
	}

}

?>
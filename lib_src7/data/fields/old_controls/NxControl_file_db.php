<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_file_db
{
	function NxControl_file_db()
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
			return "<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$objUrl."\" />";

		if (!$desc->isEdit())
			return $field->toHTML()."<input type=\"hidden\" name=\"".$desc->name."\" value=\"".$objUrl."\" />";			
		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$size=intval($desc->getProperty("size",$colsMaxBox));

		$F_ = $desc->name;
		$L_ = $field->getLabel();
		$dbApp=$desc->getProperty('file_db_url');

		// get strings
		$strings=$desc->getChoiceList('form_fields',null,'strings');
		$s_n=$strings->getString('new file');
		$s_k=$strings->getString('keep file');
		$s_r=$strings->getString('replace file');
		$s_d=$strings->getString('delete file');
		$s_i=$strings->getString('file database');
		
		
		if ($url == null || $url == "")
		{
			if ($dbApp)
			{
			// empty form
			$src= <<<EOH
<div class="image_update_box">
<!-- selectors -->
<input  type="radio" value="new" class="radio_file" CHECKED="true"
	id="{$F_}_choice_replace" name="{$F_}_choice" 
	onclick="Nx_onReplaceFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file)">
	<span class="radio_file">{$s_n}</span>
EOH;

			$src.= <<<EOH
<input type="radio" value="db"	class="radio_file" 
	id="{$F_}_choice_db" name="{$F_}_choice" 
	onclick="Nx_onDBFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file,{$F_}_choice_db_file_url)">
	<span class="radio_file">{$s_i}</span>
EOH;

			$src.= <<<EOH
<br>	
<!-- file URL preview -->
<div class="file" id="{$F_}_preview" styleX="float:right" srcKeep="">
&nbsp;
</div>
<!-- file browse -->
<input	type="file" class="file_image_replace" name="{$F_}" id="{$F_}_choice_file" size="60"  onpropertychange="Nx_onFileChange(this,{$F_}_choice_file)"
	onchange="Nx_onFileChange(this,{$F_}_choice_file)" onclick="Nx_onFileChange(this,{$F_}_choice_file)" onblur="Nx_onFileChange(this,{$F_}_choice_file)" />
	<input type="hidden" name="{$F_}_save" value="" />
<!-- display image url from DB and button calling db application -->
<div id="{$F_}_db_file" style="visibility:hidden;position:relative;top:-1.2em" onclick="Nx_chooseDBFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file,{$F_}_choice_db_file_url,'{$dbApp}')" class="file_db">
	<input type="hidden" class="file_db" name="{$F_}_db" id="{$F_}_choice_db_file_url" value="" /><button >Select...</button>
</div>
</div>
EOH;
			}
			else 
			{
			
			$src= <<<EOH
				<input type="file" class="file" name="{$F_}" id="{$F_}_choice_file" size="60"  onpropertychange="Nx_onFileChange(this,{$F_}_choice_file)"
					onchange="Nx_onFileChange(this,{$F_}_choice_file)" onclick="Nx_onFileChange(this,{$F_}_choice_file)" onblur="Nx_onFileChange(this,{$F_}_choice_file)" />
EOH;
	
			}
		}
		else 
		{		
			// field update
			$N_= $field->toTitle();

			$src= <<<EOH
<span class="image_update_box">
<!-- selectors -->
<input type="radio" value="keep" class="radio_file" 
	id="{$F_}_choice_keep" name="{$F_}_choice" 
	srcKeep="{$objUrl}" onclick="Nx_onKeepFile(this,{$F_}_preview,{$F_}_choice_file,{$F_}_db_file)" CHECKED="true">
	<span class="radio_file">{$s_k}</span>

<input  type="radio" value="replace" class="radio_file" 
	id="{$F_}_choice_replace" name="{$F_}_choice" 
	onclick="Nx_onReplaceFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file)">
	<span class="radio_file">{$s_r}</span>

<input type="radio" value="delete"	class="radio_file" 
	id="{$F_}_choice_delete" name="{$F_}_choice" 
	onclick="Nx_onDeleteFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file)">
	<span class="radio_file">{$s_d}</span>
EOH;

			if ($dbApp)
			{
				$src.= <<<EOH
<input type="radio" value="db"	class="radio_file" 
	id="{$F_}_choice_db" name="{$F_}_choice" 
	onclick="Nx_onDBFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file,{$F_}_choice_db_file_url)">
	<span class="radio_file">{$s_i}</span>
EOH;
			}
			$src.= <<<EOH
<br>	
<!-- file URL preview -->
<div class="file" id="{$F_}_preview" styleX="float:right" srcKeep="{$objUrl}">
<a class="file" target="_blank" href="{$url}">{$N_}</a>
</div>
<!-- file browse -->
<input	type="file" class="file_image_replace" name="{$F_}" id="{$F_}_choice_file" size="60" style="visibility:hidden" onpropertychange="Nx_onFileChange(this,{$F_}_choice_file)"
	onchange="Nx_onFileChange(this,{$F_}_choice_file)" onclick="Nx_onFileChange(this,{$F_}_choice_file)" onblur="Nx_onFileChange(this,{$F_}_choice_file)" />
	<input type="hidden" name="{$F_}_save" value="{$objUrl}" />
<!-- display image url from DB and button calling db application -->
<div id="{$F_}_db_file" style="visibility:hidden;position:relative;top:-1.2em" onclick="Nx_chooseDBFile({$F_}_preview,{$F_}_choice_file,{$F_}_db_file,{$F_}_choice_db_file_url,'{$dbApp}')" class="file_db">
	<input type="hidden" class="file_db" name="{$F_}_db" id="{$F_}_choice_db_file_url" value="" /><button >Select...</button>
</div>
</span>
EOH;
				
/*
			$src= <<<EOH
<span class="file_update_box">
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
*/
		}
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
		$choiceName = $desc->name . "_choice";

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
				$fileStore = $desc->getPlugIn("FileStore");
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
					$field->addError($errorCB, $s_e);
				}

				$field->info = null;
				break;
				
			case "db":
				// get previous file and delete it if exists and not from db
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$field->info = $field->unserialise($_POST[$savedUrlField]);
					
				}
				else
				{
					$field->addError($errorCB, $s_e);
					$field->info = null;
				}
				// delete current file(s)
				// only delete if not from db
				if(!(isset($field->info['is_db']) && $field->info['is_db']) && isset($field->info['n']))
				{
					$fileStore = $desc->getPlugIn("FileStore");
					$fileStore->delete($field->info,$errorCB,true);
				}
					
				// get new file from DB
				$dbUrlField = $desc->name."_db";
				$field->info = null;
				if (isset($_POST[$dbUrlField]))
				{
					$field->info = $field->unserialise($_POST[$dbUrlField]);
					$field->info['is_db']=true;
				}
			
				break;
				
			case "keep":
				$savedUrlField = $desc->name."_save";
				if (isset($_POST[$savedUrlField]))
				{
					$field->info = $field->unserialise($_POST[$savedUrlField]);
					
				}
				else
				{
					$field->addError($errorCB, $s_e);
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
					$field->addError($errorCB, $s_e);
					$field->info = null;
				}
				
				// delete current file(s)
				if(!(isset($field->info['is_db']) && $field->info['is_db']) && isset($field->info['n']))
					$fileStore->delete($field->info,$errorCB,true);
				
				// get uploaded file, resize it if needed and store it in required upload directory
				// with same name as previous
				$field->info =  $fileStore->store($desc->name,
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
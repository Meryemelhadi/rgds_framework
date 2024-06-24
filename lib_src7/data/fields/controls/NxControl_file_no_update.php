<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_file_no_update
{
	function __construct()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm($f,$value) 
	{
	
		$desc=&$f->desc;
		$F_ = $fname = $f->getAlias();
		
		$url = $f->_getUrl();
		$objUrl = $f->serialise($value);

		if (!$desc->isInput())
			return "";

		if ($desc->isHidden())
			return <<<EOH
		<input type="hidden" name="{$F_}" value="$url" /><input type="hidden" value="keep" name="{$F_}_choice"/><input type="hidden" name="{$F_}_save" value="{$objUrl}" />
EOH;


		if (!$desc->isEdit())
			return $f->toHTML(). // "<input type=\"hidden\" name=\"".$fname."\" value=\"".$objUrl."\" />";
		<<<EOH
		<input type="hidden" name="{$F_}" value="$url" /><input type="hidden" value="keep" name="{$F_}_choice"/><input type="hidden" name="{$F_}_save" value="{$objUrl}" />
EOH;

		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$size=intval($desc->getProperty("size",$colsMaxBox));
							
		// update
		$L_ = $f->getLabel();
		if (isset($f->info['n']))
			$N_=$f->info['n'];
		else
			$N_= '';

		if ($url == null || $url == "")
		{
			$src= <<<EOH
				<input type="file" class="file" name="{$F_}" id="{$F_}_choice_file" size="{$size}"  onpropertychange="Nx_onFileChange(this,$('{$F_}_choice_file'))"
					onchange="Nx_onFileChange(this,$('{$F_}_choice_file'))" onclick="Nx_onFileChange(this,$('{$F_}_choice_file'))" onblur="Nx_onFileChange(this,$('{$F_}_choice_file'))" />
EOH;

			//return "<input class=\"file\" type=\"file\" name=\"".$fname."\" size=\"".$size."\" ".$desc->getStyle("input","file"). " />";
		}
		else
		{
			// get strings
			$strings=$desc->getChoiceList('form_fields',null,'strings');
			$s_k=$strings->getString('keep file');
			$s_r=$strings->getString('replace file');
			$s_d=$strings->getString('delete file');
			$s_i=$strings->getString('file database');
							
			$src= <<<EOH
<div class="image_update_box">
<input type="hidden" value="keep" id="{$F_}_choice_keep" name="{$F_}_choice" 
	srcKeep="{$objUrl}" CHECKED="true" />
<a class="file" target="_blank" href="{$url}">{$L_}: {$N_}</a>
</div>
<input	type="file" class="file_replace" name="{$F_}" id="{$F_}_choice_file" size="{$size}" style="visibility:hidden" onpropertychange="Nx_onFileChange(this,('{$F_}_choice_file'))"
	onchange="Nx_onFileChange(this,('{$F_}_choice_file'))" onclick="Nx_onFileChange(this,('{$F_}_choice_file'))" onblur="Nx_onFileChange(this,$('{$F_}_choice_file'))" />
	<input type="hidden" name="{$fname}_save" value="{$objUrl}" />
</div>
EOH;
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
	function readForm(&$f,&$recData,&$errorCB)
	{
		$desc=&$f->desc;
		$fname = $f->getAlias();
		$choiceName = $fname . "_choice";

		if (!isset($recData[$choiceName]))
		{
			$choice = "new";
		}
		else
		{
			$choice = $recData[$choiceName];
		}

		switch($choice)
		{
			case "delete":
				$fileStore = $desc->getPlugIn("FileStore");
				$savedUrlField = $fname."_save";
				if (isset($recData[$savedUrlField]))
				{
					$fileVars = $f->unserialise($recData[$savedUrlField]);
					$fileStore->delete($fileVars,$errorCB,true);
				}
				else
				{
					$f->addError($errorCB, "field error");
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
					$f->addError($errorCB, "field error");
					$f->info = null;
				}
				break;
				
			case "replace":
				$fileStore = $desc->getPlugIn("FileStore");

				// get file parameters
				$savedUrlField = $fname."_save";
				if (isset($recData[$savedUrlField]))
				{
					$f->info = $f->unserialise($recData[$savedUrlField]);
				}
				else
				{
					$f->addError($errorCB, "field error");
					$f->info = null;
				}
				
				// delete current file(s)
				$fileStore->delete($f->info,$errorCB,true);
				
				// get uploaded file, resize it if needed and store it in required upload directory
				// with same name as previous
				$f->info =  $fileStore->store($fname,
								//$errorCB, $f->info["n"],
								$errorCB, null,
								$desc->isRequired(), $f->getProperty("maxFileUploadSize",100000));
							
				$url = $f->_getUrl();
				if($url)
					$f->info['url']=$url;
				break;

			case "new":
			default:
				$fileStore = $desc->getPlugIn("FileStore");
				
				// get uploaded file, resize it if needed and store it in required upload directory
				$f->info =  $fileStore->store($fname,
								$errorCB,null,
								$desc->isRequired(), $f->getProperty("maxFileUploadSize",100000));
				
				$url = $f->_getUrl();
				if($url)
					$f->info['url']=$url;

				break;
		}
		 		
		return true;
	}

}

?>
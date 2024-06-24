<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 	transform a tpl file into a php file to be used as a nx html view.
 	syntax is described here after. NB. [] denotes part of expressions that are not mandatory.
 	
 	- properties:
 	 	{= prop_name[|default]} : output a property if exist or default text
 	
 	- records:
 		{records[:records_prop]} {/records} : loop on records found in records_prop property or "record".
 	- record:
 	{record[:record_prop]} {/records} : 
 	loop on record fields, record is found in record_prop property. If inside a records loop, can be omitted.
 	
 	- {form:props} {record[:record_prop]} {field}

 	- conditional
 	{if:cond:content[:else_content]} or
 	{if:cond} content [{elseif:cond}} elseif_content] [{else} else_content] {/if}
 	
 	- direct php code: <?[php] code ?> 

*/
require_once('NxPlug_Template.inc');

class NxPlug_Content_html extends NxPlug_Template
{
	var $buildlocals;
	
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->pathOffset='';
		$this->ext='inc';
		$this->t_ext='html;htm';
		$this->d_ext='inc';
		$this->mode=NX_CHECK_CONTENT;
	}

	function execute($exe,&$res)
	{
		return true;
	}

	// perform the transformation from source file to dest file. Call parse() for the actual
	// processing.
	function transform($src_path,$out_path,$checkDir=true,$ext="tpl")
	{
		// check if src file exist, dest dir exist and output up to date
		if (!is_readable($src_path))
		{
			nxError("template file does not exist or is not readable:{$src_path}");
			return false;
		}
		
		$this->translator=& $this;
		
		// obsolete ?
		$checkTime=true;
		
		// check if output is up to date
		if ( NX_CHECK_CONTENT==="build" &&
			 $checkTime && is_file($out_path) &&
			 @filemtime($src_path) < @filemtime($out_path))
		{
			// up to date...
			return true;
		}
		
		// check if output directory exist
		if ($checkDir && !is_dir($d=dirname($out_path)))
			if ($this->fs->createTree($d)==null)
				return false;
			
		// transform it and write to file
		
		// get file
		$src = $this->fs->file_get_contents($src_path);
		// nxLog("initial file ------- \n".$src);
		
		// translate it from initial language (ex. xml) to tpl
		$src =  $this->translator->translate($src,$this);
		// nxLog("-----------\n 1) after translation \n------- \n".$src);
		
		// process image, csx and jsx files
		// $src =  $this->processResources($src);
		 // nxLog("-----------\n 3) after include processing \n------- \n".$src);
		 
		// finally parse tpl language and produce php
		$src =  $this->parse($src,$ext);
		 // nxLog("-----------\n 4) after tpl transformation \n------- \n".$src);
		
		$src =  $this->cleanup($src);
		 // nxLog("-----------\n 5) after cleanup \n------- \n".$src);
		
		if ($fp=$this->fs->fopen($out_path,'w'))
		{
			// store php source
			fputs($fp, $src);
			fflush ($fp);
			fclose($fp);
			nxLog("compiled template $src_path to $out_path",'NxPlug_Content',NX_COMPILE_EVENT);
			return true;
		}
		else {
			nxError("cannot create content transformation to file:{$out_path}");
			return false;
		}
	}

	// ================== PREPROCESSOR ===================
	function & translate(& $src,$tpl)
	{
		return $src;
	}
	
	// preprocess includes: read files and insert them in source
	function & processIncludes(&$src)
	{		
		return $src;
	}

	function & rebaseProperties(& $code,$base)
	{
		return $code;
	}

	// =================== RESOURCES ==============
	function & processResources(& $src)
	{
		return $src;
	}
	
	function mapResPath($attr,$path)
	{
		return '';
	}
		
	// add the resource requirement
	function requireRes($name,$ext)
	{
		return '';
	}
	
	// =========== html resource link handlers ===========
	
	function mapCSS($s,$dft,$build)
	{
		return '';
	}

	function mapHeads($s,$dft,$build)
	{
		return '';
	}

	function mapJS($s,$dft,$build)
	{
		return '';
	}


	function mapSubmit($name)
	{
		return '';
	}

	function getButton($name,$build,$urlParams)
	{
		return '';
	}
	
	function mapSpacer($w,$h,$static)
	{
		return '';
	}

}
?>
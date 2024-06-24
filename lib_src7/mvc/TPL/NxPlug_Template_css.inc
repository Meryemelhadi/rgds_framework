<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 	transform a csx file into a css file.
 	
 	Supports the same directive set as HTML templates (tpl) but in the form of css2 directive syntax.
 	ex: 
 		@property(prop) or @prop
 		@if(cond) css_action @elseif(cond) action @else action @endif
 		@include(page.othercss.css)
 		@set:variable v = ?value?;
*/
require_once('NxPlug_Template_csx.inc');

class NxPlug_Template_css extends NxPlug_Template_csx
{
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->ext='css';
		$this->t_ext='css';
		$this->d_ext='css.php';
		$this->pathOffset='res/';
	}

	// parse a string in csx format and returns a php view
	function parse($src,$ext='')
	{
		$src =  $this->processResources($src);

		return $src;
	}
	
	function transformNXL($src)
	{
 		return $src;
	}
	
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
include_once('NxApp_View.inc');

class NxApp_View_XML extends NxApp_View
{
	var $viewName;
	
	function __construct($viewName,$props,&$parentDesc,$propertyBase="")
	{
		parent::__construct($viewName,$props,$parentDesc,$propertyBase);
		$this->filename=$this->getProperty('file.name','file.xls');
	}
	
	function exec_view($f,$ispage) {
		nxLog("run view: $f","NxApp_ViewModule",NX_DEBUG);
				
		global $NX_CHARSET;

		ob_start();
			include($f);
			$buffer='<?xml version="1.0" encoding="'.strtoupper($NX_CHARSET).'"?>'."\n".ob_get_contents();
		ob_clean();

		// if page, send headers
		if ($ispage) {
			$this->getHeaders(strlen($buffer));
		}
		echo $buffer;
		
		return $properties;		
	}
	
	function getHeaders($len) {
		global $NX_CHARSET;
	  
		//We send to a browser
		header('Content-Type: text/xml; charset='.$NX_CHARSET);
		if(headers_sent())
			$this->Error('Some data has already been output to browser, can\'t send XML data');
			
		header('Content-Length: '.$len);
//		header('Content-disposition: inline; filename="'.$this->filename.'"');

		header("Cache-Control: must-revalidate, post-check=0,pre-check=0");
		header("Pragma: public");
		
	}

	// create a button to be used in a view
	function getButton($name,$url=null,$urlParams=null)
	{
		return '';
	}
	
	function _getFormHeader(&$recDesc, $nextStepUrl, $nxs=null, $nxo=null,$method="POST")
	{			
		return '';
	}
	
	function _getSubmitButton($name,$formName=null,$event=null)
	{
		return '';
	}
	
	function _getStyle($styleKey,$deft=null,$props=null)
	{		
		return '';
	}
	
	function _getStyleSheets($urls) {
		return '';
	}
	
	function _getScripts($urls) {
		return '';
	}	
	
}

?>
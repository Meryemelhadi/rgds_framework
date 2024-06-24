<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
include_once('NxApp_View.inc');

// ====== DEVICE SENSITIVE SETTINGS =======

class NxApp_View_XHTML extends NxApp_View
{
	var $viewName;
	
	function __construct($viewName,$props,&$parentDesc,$propertyBase="")
	{
		parent::__construct($viewName,$props,$parentDesc,$propertyBase);
	}
	
	function getHeaders() {
		global $NX_MIME,$NX_CHARSET;
		header('X-UA-Compatible: IE=edge,chrome=1' );
		header("Content-Type: {$NX_MIME}; charset={$NX_CHARSET}"); 		
	}

	// create a button to be used in a view
	function getButton($name,$url=null,$urlParams=null)
	{
		$props = & $this;
			
		if ($url==null)
			$url = $props->getProperty($name.".url",null,true);

		// check if button redirected to another one
		$redirect = $props->getProperty($name.".redirect",null,true);
		if ($redirect != null)
			return $this->getButton($redirect,$url,$urlParams,$props);
			
		if ($url !== null && $props->getProperty($name.".inactive","false",true)!=="true")
		{
			$img    = $props->getProperty($name.".image",null,true);
			$label  = $props->getProperty($name.".label",$name,true);
			$target = $props->getProperty($name.".target",null,true);
			$className = $props->getProperty($name.".class","button",true);
			if ($urlParams!=null)
				// TODO: check if 1rst params and if url has ? or # in it ?
				$url .= $urlParams;
		}
		else 
		{
			return '';
			
			// if inactive button, check if needs to be hidden
			if ($props->getProperty($name.".inactive.hide",null,true) === "true")
				return "&nbsp;";

			// if inactive button, try to redirect to another button
			$redirect = $props->getProperty($name.".inactive.redirect",null,true);
			if ($redirect != null)
				return $this->getButton($redirect,$url,$urlParams,$props);

			// default: just change class name (suffixed by "_inactive")
			$img    = $props->getProperty($name.".image",null,true);
			$label  = $props->getProperty($name.".label",$name,true);
			$target = $props->getProperty($name.".target",null,true);
			$className = $props->getProperty($name.".class","button",true)."_inactive";
			$url = "#";
		}			
	

		if ($target == null)
			$target = $props->getProperty("button.target",null,true);
		
		if ($target != null)
			$target = " target=\"".$target."\" ";
		else
			$target = "";
			
		// I8N
		$label=$props->getString($label);
		
		if ($img == null)
		{
			// $style   = $props->getStyle("button.text");
			// $styleL  = $props->getStyle("button.text.link");
			
			$res = "<span class=\"".$className."_text\" id=\"text\"><a href=\"".$url."\"".$target.">";
			$res .= $label;
			return $res . "</a></span>";
		}
		else
		{
			// $style   = $props->getStyle("button.image");
			// $styleL  = $props->getStyle("button.image.link");
			// $styleI  = $props->getStyle("button.image.image");
			
			$res = "<span class=\"".$className."_image\" id=\"image\"><a href=\"".$url."\"".$target.">";
			$res .= "<img src=\"".$img."\" title=\"".$label."\" alt=\"".$label."\" border=\"0\" />";
			return $res . "</a></span>";
		}
	}
	
	function _getFormHeader(&$recDesc, $nextStepUrl, $nxs=null, $nxo=null,$method="POST")
	{			
		$formName = "form" . $recDesc->getProperty("name","",false);

		if (!isset($this->formID))
			$this->formID = 0;
		else
		{
			$this->formID++;
			$formName .= (string)$this->formID;
		}
		
		$s = "<script>\n  window.check_".$formName."=function(frm) {\nvar error = \"\"; ";

		$fileSize = 0;
		$fdescs = & $recDesc->fdescs;
		foreach(array_keys($fdescs) as $fname)
		{
			$fdesc = & $fdescs[$fname];
			if (isset($fdesc->props["isFile"]) && $fdesc->props["isFile"] == true)
			{
				$fileSize += (int)$fdesc->getProperty("maxFileUploadSize", 1000000);
			}

			$check = $fdesc->onSubmitCheck("frm");
			if ($check!="")
			{
				$s .= "error += ".$check.";";
			}
		}

		$s .= "if (error!=\"\") { alert(Nx_res.getString(\"error, please check following fields\")+\":\"+ error); return false;} return true;\n}</script>\n";

		$s .= "<form name=\"".$formName."\" onsubmit=\"return window.check_".$formName."(this);\" action=\"". $nextStepUrl ."\" method=\"$method\"";
//		if ($method=="POST" && $fileSize > 0)
		if ($method=="POST")
		{
			$s .= " ENCTYPE=\"multipart/form-data\">";
			// workaround php4 bug: MAX_FILE_SIZE not handled properly. 
			// Besides IE6 doesn't support it either... => set it to 100MB
			$fileSize=100000000;	
			$s .= "<input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"".$fileSize."\" /";
		}
		else
		{
			$params = $this->url2Array($nextStepUrl);
			foreach($params as $k=>$v)
			{
				$s .= ">\n<input type=\"hidden\" name=\"{$k}\" value=\"".$v;
			}
		}
		
		$s .= ">";
		if ($nxs!=null)
			$s .= "<input type=\"hidden\" name=\"nxs\" value=\"".$nxs.'\>';
		if ($nxo!=null)
			$s .= "<input type=\"hidden\" name=\"nxo\" value=\"".$nxo.'\>';
			
		return $s;
	}
	
	function url2Array($url){

		//Extract the Querystr pos after ? mark
		$startpos = stripos($url, "?");

		//Extract the Querystr alone
		$qryStr=substr($url, $startpos+1) ;

		//Split the querystring into & pairs
		$qryvalues=explode("&", str_replace('&amp;','&',$qryStr));

		foreach($qryvalues as  $value)
		{
			//Split the single data into two i.e data   |  value
			$buffer=explode("=", $value);

			// Urlencode only the value now , Change it to rawurlencode if necessary
			$par[$buffer[0]]=$buffer[1];
		}

		return $par;
	}
	
	function _getSubmitButton($name,$formName=null,$event=null)
	{
		$props = $this;

		if ($formName) 
		{
			$actionUrl = $props->getProperty($name.".url","#",true);
			$event=' onClick="onSubmitForm(\''.$formName.'\',\''.$actionUrl.'\')" ';
		}
		else
			$event='';

		$redirect = $props->getProperty($name.".redirect",null,false);
		if ($redirect != null)
			return $this->getSubmitButton($redirect,$formName,$event);

		$label = $props->getProperty($name.".label",null,true);
		if ($label == null)
			$label = $props->getString("submit");
			
		$img = $props->getProperty($name.".image",null,true);
		$className = $props->getProperty($name.".class","button",true);
		
		// I8N
		$label=$props->getString($label);

		// $style  = $this->getStyle("style:button");
		if ($img == null)
		{
			// $style   = $props->getStyle("submitButton.text");
			return "<input class=\"".$className."_text\" type=\"submit\" $event value=\"".$label."\"/>";
		}
		else
		{
			// $style   = $props->getStyle("submitButton.image");
			return "<input class=\"".$className."_image\" $event id=\"image\" type=\"image\" alt=\"".$label."\" src=\"".$img."\"/>";
		}
	}
	
	function _getStyle($styleKey,$deft=null,$props=null)
	{		
		if ($props == null)
			$props = & $this->props;
		
		$style = $props->getProperty("style:".$styleKey,$deft);
		if ($style != null)
			$style = " style=\"$style\"";
		else
			$style = "";

		return $style;
	}
	
	function _getStyleSheets($urls) {
		$s='';
		foreach ($urls as $u)
		{
			if ($u!='')
				$s.='<link  type="text/css" rel="stylesheet" href="'.$u."\" />\n";
		}
		return $s;
	}
	
	function _getScripts($urls) {
		$s='';
		foreach ($urls as $u)
		{
			if ($u!='')
				$s.='<script type="text/javascript" language="Javascript" src="'.$u."\"></script>\n";
		}
		return $s;
	}	
	
}

?>
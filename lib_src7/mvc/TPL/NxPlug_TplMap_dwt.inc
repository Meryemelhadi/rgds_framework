<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!defined("NX_DWT_MAP_TITLE"))
	define("NX_DWT_MAP_TITLE","title");
	
class NxPlug_TplMap_dwt
{
	function __construct($props)
	{
		$this->props=$props;
	}

	function translate($src,&$tpl)
	{
		$this->reqs = array();
		$this->tpl =& $tpl;
		
		$patterns = array(
			'/<!--!((((.)(?!-->))*)(.)?)-->/s',
			'|<!-- #BeginEditable "doctitle" -->.*[\n]?.*<title>([^<]*)</title>.*[\n]?.*<!-- #EndEditable -->|i',
			'/<!-- #BeginEditable "([^"]+)" -->[{]?([^}\n]|\n|(?:[}](?!<!--))+)[}]?<!-- #EndEditable -->/e',
			'/<!-- #BeginEditable "([^"]+)" -->[{]?/',
			'/[}]?<!-- #EndEditable -->/',
			'/<!--#include (?:virtual|file)="((?:[^."]|(?:[.](?!dwt|html|htm|tpl|inc)))+)[.](dwt|html|htm|tpl|inc)" -->/i',
			'/<!-- #BeginLibraryItem "((?:[^."]|(?:[.](?!lbi)))+)[.]lbi" -->/e',
			'/<!-- #EndLibraryItem -->/',
/*
			'#((?:src|href)=")[.][.][/]res[/]([^"]+")#ei',
			'#((?:src|href)=")[.][.][/][.][.][/]default[/]res[/]([^"]+")#i',
			'#<script\s+[^>]*src="[{]@@NX_SKIN_URL[}]([^"]+)".*</script>#ei',
			'#<link\s+[^>]*href="[{]@@NX_SKIN_URL[}]([^"]+)"[^>]*>#ei',
			'#(<img\s+[^>]*src="[{]@@NX_SKIN_URL[}]([^"]+)"[^>]*>)#ei',
			'|(</head>)|ei',
*/
			);
			
		$replace = array(
			'$1',
			'<title>{content:'.NX_DWT_MAP_TITLE.'|$1}</title>',
			'$this->editable("$1","$2")',
			'{property:$1|',
			'}',
			'{include:$1}',
			'$this->module("$1")',
			'--}',
/*
			'$this->mapResPath("$1","$2")',
			'$1{@@NX_DEFT_SKIN_URL}$2',
			'$this->requireRes("$1","js")',
			'$this->requireRes("$1","css")',
			'$this->requireImg("$2","$1")',
			'$this->finishHead("$1")',
*/
			);
			
		return preg_replaceX($patterns, $replace,$src,$this);
	}

	function transformNXL($src)
	{
 		return $src;
	}	
	
	function editable($name,$deft)
	{
		return "{property:$name|{$deft}}";
	}

	function module($path)
	{
		$def=substr(strrchr($path,"/"),1);
		$name=substr(strstr($def,'.'),1);
		$lenType=strlen($def) - strlen($name) - 1;
		$type=substr($def,0,$lenType);
		
		if ($type=="button")
		{
			if ($name=="submit")
				return "{property:submitButton}{--";
			else
				return "{button:{$name}}{--";
		}
		else
			return "{object:{$def}}{--";
	}
/*
	function mapResPath($attr,$path)
	{
		nxLog("map path: {$attr}{$path} to {$attr}{@@NX_SKIN_URL}{$path}","DWT");
		return "{$attr}{@@NX_SKIN_URL}{$path}";
	}
	
	// add the resource requirement
	function requireRes($name,$ext)
	{
		nxLog("Require Res: $name.$ext","DWT");
		// in a page, returns a TPL macro
		if ($this->tpl->isPage)
			// if resource has a device suffix, add device option
			return preg_replace('/(?:_IE|_N4|_GK|_IE55)([.][^}]+)}$/','$1 and device}','{@require:'.$name.":skin}");
		
		// else register the requirement in property page.head.ext.req
		$baseName = preg_replace('/([.].+)$/','',$name);
		$noDeviceName = preg_replace('/(?:_IE|_N4|_GK|_IE55)$/','',$baseName);

		if ($noDeviceName==$baseName)
			$isDevice = false;
		else
			$isDevice = true;
		
		$this->tpl->appView->requireRes($ext,$noDeviceName,true,$isDevice);
		
		nxLog("Require Res - end","DWT");
		return '';
	}
	
	// add the image requirement in the head
	function requireImg($name,$s)
	{
		nxLog("Require Img: $name","DWT");
		
		if ($this->tpl->isPage)
		{
			// if resource has a device suffix, add device option
			$this->reqs[] = '{@require:'.$name.":skin}";
			return str_replace("\'","'",$s);
		}

		$ext = substr(strrchr($name,'.'),1);
		$n=substr($name,0,strlen($name)-strlen($ext)-1);
		nxLog("resource full name:$name, basename=$n, ext:$ext","requireImg");	
		$this->tpl->addDeployRes(array("name"=>$n,"type"=>$ext,"ext"=>$ext,"isDevice"=>"false","isSkin"=>"true"));
		
		$res = str_replace("\'","'",$s);
		nxLog("Require IMG - end","DWT");
		return $res;

	}
*/	
	// complete header with resource requirements
	function finishHead($tag)
	{
		nxLog("finish head $tag - start","DWT");
		if (!$this->tpl->isPage)
		{
			nxLog("finish head - end: $tag","DWT");
			return $tag;
		}
			
/*		$src = '{@css}
{@js}
{@heads}
		{css}{js}
';
		nxLog("finish head - end: $src","DWT");
*/	
		return $src."\n$tag";
	}
}

?>
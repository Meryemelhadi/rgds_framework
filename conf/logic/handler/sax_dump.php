<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class handler_sax_dump extends NxPage
{
	function handler_sax_dump($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		// URL
		if (isset($_GET['url']))
			$url=$_GET['url'];
		else
			$url='site.nxl';

		// Disply as html text (display set) or raw html?
		if (isset($_GET['display']))
			$display=true;
		else
			$display=false;
			
		// sax driver
		if (isset($_GET['driver']))
			$driver=$_GET['driver'];
		else
			$driver='php';
			
		include(NX_LIB.'xml/NxSax_dump.inc');
		$path=NX_CONF.'nxl/'.ltrim($url,'/');
		
		$sax = new NxSax_dump($driver,$display);
		$output=$sax->parseFile($path);
		
		// $xml_parser->toPHP();
		$this->runView('page.message',
				array('page.content'=>$output) 
			);
	}
}
?>
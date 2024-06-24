<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB.'module/NxPage.inc');

class content_static_page extends NxPage
{
	function content_static_page($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
		$this->fs=$this->getPlugIn('FileSys');
	}
	
	function run()
	{
		// get document ID
		if (!isset($_GET['docID']))
			$docID=$this->getProperty('page.docID','default');
		else
			$docID=$_GET['docID'];

		$this->replaceProperty('page.docID',$docID);
		
		$docRecords = $this->getRecords($docID,'page.doc','xdocs');
		if (!$docRecords)
		{
			nxError("can't find static document:[$docID]");
			$docRecords = $this->getRecords('default','page.doc','docs');
			if (!$docRecords)
				return false;
		}
		
		// adds properties to current set (NB. assumes only one record...)
		$this->addProperties($docRecords->getProperties());
		
		// get doc section (for default nav and skin)
		if (($section=$this->getProperty('page.meta.section'))==null)
			$section='default';
			
		// get navigation
		if (($menuID=$this->getProperty('page.meta.menu'))==null)
			$menuID=$section;
			
		// load navigation menu as a collection of links into page.links property
		// if ($res =& $this->loadLinks("links/$menuID"))
		//	$this->setProperty('page.links',$res);
		$this->getLinks($menuID,'page.links','links');
			
		// get template
		if (($view=$this->getProperty('page.meta.template'))!=null)
			$view="page.$view";
		else
			$view='page.view';
			
		// display page in template
		$this->runView($view,null,null,$this->getProperty('properties.base',null));		
	}
}

?>
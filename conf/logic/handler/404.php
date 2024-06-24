<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class handler_404 extends NxPage
{
	function handler_404($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function disp404($url)
	{
		$this->setProperty('page.url',$url);
		
		$docID=$this->getProperty('page.docID','404');
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
				
		// get template
		$view='page.view';
			
		// display page in template
		$this->runView($view);		
	}
	
	function deployAndSend($url,$type='php') {
		$this->fs =& $this->getPlugIn('FileSys');

		$path = explode('/',$url);
		$f=null;
		if (count($path)>0 && defined('GENERIS_ROOT'))
		{
			$package=$path[0];
			if (!is_file($f=GENERIS_ROOT."packages/{$package}/www/".$url))
				$f=null;
		}
		
		if (!$f && !is_file($f=NX_DIS.'www'.$url))
			return false;
		
		// deploy file
		$ftarget=NX_DOC_ROOT.ltrim($url,'/');
		if (!$this->fs->copy($f,$ftarget))
			return false;

		if ($type=='php')
			$this->redirectTo($url);	
		else
			include($ftarget);
		
		return true;		
	}
	
	function compileNXL($url)
	{
		set_time_limit(0);
		
		include(NX_LIB.'mvc/NXL/NXL_processor.inc');
		$path=NX_SITE.'nxl/'.ltrim($url,'/');
		if (!is_file($path))
		{
			if (is_file($url))
				$path=$url;
			else
			{
				$this->runView('page.message',
						array('page.content'=>"can't find file: $url") 
					);
				return false;
			}		
		}
		
		$xml_parser = new NXL_processor($this);
		$xml_parser->build($path);
		// $xml_parser->toPHP();
		$this->runView('sys.nxl.compil',
				array('page.content'=>$xml_parser->buildOutput()));
		return true;
	}
	
	function run()
	{
		global $_NX_site;
		if (isset($_GET['url']))
			$url=$_GET['url'];
		/*elseif (isset($_SERVER['REDIRECT_URL']))
			$url=$_SERVER['REDIRECT_URL'];
		*/
		else			
			$url=$_SERVER['REQUEST_URI'];
			
		nxLog("404 handler for $url",'404');
						
		// uploaded image
		if (preg_match('#/files/(.+)/([^.]+)[.](gif|png|jpeg|jpg|bmp)#i',$url,$matches))
		{
			nxLog("uploaded image to resize and deploy:$url",'404');
			include(NX_LIB.'404/NxRes_ResizeImg.inc');
			$view=new NxRes_ResizeImg($_NX_site);
			if ($view->deployAndSend($url))
			{
				return true;
			}			
		}

		// uploaded image (mode ASP)
		if (preg_match('#/afiles/(.+)/([^.]+)[.](gif|png|jpeg|jpg|bmp)#i',$url,$matches))
		{
//			$url='/files/'.$_SERVER['SERVER_NAME']."/{$matches[1]}/{$matches[2]}.{$matches[3]}";
			$url="/files/{$matches[1]}/{$matches[2]}.{$matches[3]}";
			nxLog("uploaded image to resize and deploy (ASP):$url",'404');
			include_once(NX_LIB.'404/NxRes_ResizeImg.inc');
			$view=new NxRes_ResizeImg($_NX_site);
			if ($view->deployAndSend($url))
			{
				return true;
			}			
		}
		
		// skined image button
		// /nx/skins/skin1/buttons/class but/name.gif
		if (preg_match('#/nx/skins/([^/]+)/images/buttons/([^/]+)/([^.]+)[.]([^?]+)#i',$url,$matches))
		{
			nxLog("image button to deploy:$url",'404');
			list($url,$skin,$desc,$name,$ext) = $matches;
			
			include(NX_LIB.'404/NxRes_skinButton.inc');
			$view=new NxRes_skinButton($_NX_site);
			if ($view->deployAndSend($url,$skin,$desc,urldecode($name),$ext))
				return true;
			return false;
		}	
		
		// skined resource
		if (preg_match('#/nx/skins/(.+)/(.+)/([^.]+)[.]([^?]+)#i',$url,$matches))
		{
			nxLog("skinned resource to deploy:$url",'404');
			include(NX_LIB.'404/NxRes_skinRes.inc');
			$view=new NxRes_skinRes($_NX_site);
			if ($view->deployAndSend($url))
				return true;
		}	
		
		// get appli from distrib
		if (preg_match('#/nx/admin/(([^.]+)[.]([^?]+))#i',$url,$matches))
		{
			nxLog("system application page to deploy and execute:$url",'404');	
			if ($this->deployAndSend($url,$matches[3]))
			{
				return true;
			}	
			else
				nxLog("failed to deploy and execute sys application page:$url",'404');			
		}

		// get appli from package
		if (preg_match('#/(([^.]+)[.](php))#i',$url,$matches))
		{
			nxLog("system application page to deploy and execute:$url",'404');	
			if ($this->deployAndSend($url,$matches[3]))
			{
				return true;
			}	
			else
				nxLog("failed to deploy and execute sys application page:$url",'404');			
		}

		// compile nxl
		if (preg_match('#^(.*[.]nxl)#i',$url,$matches))
		{
			nxLog("NXL page to compile:$url",'404');			
			if ($this->compileNXL($url,$matches[0]))
			{
				nxLog("Compiled NXL page: ok",'404');			
				return true;
			}	
			else
				nxLog("failed to compile NXL page:$url",'404');			
		}
		
		// display error message
		nxLog("standard 404 page: unknown resource: $url",'404');	
		$this->disp404($url);		
	}
}
?>
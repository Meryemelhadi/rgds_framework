<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxPage.inc');

class handler_nxl extends NxPage
{
	function handler_nxl($props,&$parentDesc)
	{
		$this->NxPage($props,$parentDesc);
	}
	
	function run()
	{
		$url=$this->getProperty('nxl.url',$_GET['url']);

		$packages = explode('/',$url);
		if (isset($packages[0]))
		{
			$packages2 = explode('/',$url);
			// $package=$packages2[0]; 
			$package=array_shift($packages2);
			$baseUrl = implode('/',$packages2);
			// $baseUrl = str_replace($package.'/','',$url);
		}
		else
			$package='';
		
		// search the file
		$nxl_roots=explode(',',NXL_ROOTS);
		$found=false;
		foreach ($nxl_roots as $nxl_root)
		{
			if ($package)
			{
				$libRootPackage=str_replace('nxl/','packages/'.$package.'/nxl/',$nxl_root);
				if ($libRootPackage != $nxl_root)
				{
					$f = $libRootPackage.ltrim($baseUrl,'/');
					if (is_file($f))
					{
						$url=$f;
						$found=true;
						break;
					}
				}
			}

			if (is_file($nxl_root.$url))
			{
				$url=$nxl_root.ltrim($url,'/');
				$found=true;
				break;
			}
		}
		
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
		/*
		$this->runView('page.view',
				array('page.content'=>$xml_parser->buildOutput()) 
			);
		*/
		echo $xml_parser->buildOutput();
	}
}
?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// cache a page section
class NxCache_Section2
{
	var $fpath;
	
	function __construct()
	{
	}
	
	// try to load the cache from a file or start buffering. 
	// Returns true if could get the cache or false otherwise.
	function load($dir,$resID,$type='html')
	{
		$this->fpath = $dir.$resID;
			
		if( @NX_SECTION_Cache && is_file($this->fpath) && (@filesize($this->fpath)>0))
		{
			global $NX_MIME,$NX_CHARSET;
			header("Content-Type: {$NX_MIME}; charset={$NX_CHARSET}"); 
			readfile($this->fpath);				
			return true;
		}
		else
		{
			// open buffer
			ob_start();
			ignore_user_abort(true);
			return false;
		}
	}
	
	// get buffer content, update cache, and output to property or screen.
	function endCache($cancel=false)
	{
		if ($cancel!==true && $cancel!=='true')
		{
			global $_NX_site;
			$fs=$_NX_site->getPlugIn('FileSys');
	
			// save content
			if ($fp = @$fs->fopen($this->fpath,'wb+',true))
			{
				fputs($fp, ob_get_contents());
				fclose($fp);
			} 
		}		
		ob_end_flush();
	}
}
?>
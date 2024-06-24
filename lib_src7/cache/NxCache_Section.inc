<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// cache a page section
class NxCache_Section
{
	var $props;
	var $resID;
	
	function __construct($resID,&$props,$toProp=null)
	{
		$this->props  =& $props;
		$this->resID  = "{$resID}_".NX_DEVICE_MAP;	
		$this->toProp =  $toProp;
	}
	
	// try to load the cache from a file or start buffering. 
	// Returns true if could get the cache or false otherwise.
	function loadCache($duration=null)
	{
		if ($duration === null)
		{
			$duration = $this->props->getProperty($this->resID.".timeout",null);
		}

		if ($duration === null)
		{
			$duration = $this->props->getProperty("page.cache.timeout","60");
		}
		
		$this->fpath = $GLOBALS['NX_CACHE'].$this->resID;
			
		if(($time = @filemtime($this->fpath)) && 
			((time() - $time) < $duration) && filesize( $this->fpath ))
		{
			// output cached file to property or to output
			if ($this->toProp !== null)
			{
				ob_start();
					readfile($this->fpath);
					$this->props->setProperty($this->toProp,ob_get_contents());
				ob_end_clean();
			}
			else
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
	function endCache()
	{
		// save content
		if ($fp = @fopen($this->fpath,'w'))
		{
			fputs($fp, ob_get_contents());
			fclose($fp);
		} 
				
		// output to property or to output
		if ($this->toProp !== null)
		{
			$this->props->setProperty($this->toProp,ob_get_contents());
			ob_end_clean();
		}
		else
			ob_end_flush();
	}
}

?>
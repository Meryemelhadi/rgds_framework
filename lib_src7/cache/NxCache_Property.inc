<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// cache a page section in a property
class NxCache_Property
{
	var $props;
	var $resID;
	
	function __construct(&$props,$toProp=null)
	{
		$this->props  =& $props;
		$this->toProp =  $toProp;
	}
	
	// try to load the cache from a file or start buffering. 
	// Returns true if could get the cache or false otherwise.
	function loadCache($duration=null)
	{
		// open buffer
		ob_start();
		return false;
	}
	
	// get buffer content, update cache, and output to property or screen.
	function endCache()
	{				
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
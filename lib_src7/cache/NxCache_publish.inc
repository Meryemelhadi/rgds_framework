<?php

class NxCache_publish {
	var $props;
	
	function __construct(&$props) {
		$this->props =& $props;
	}
	
	// cleanup cache directory for a page
	function cleanCache() {
		if (($path=$this->props->getProperty('page.cache.path',null))==null)
			return false;
			
		$fullpath=$GLOBALS['NX_CACHE'].$path;
		$fs=$this->props->getPlugIn('FileSys');
		$fs->cleanDir($fullpath);
		return true;
	}
}

?>
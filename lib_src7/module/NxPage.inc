<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB."module/NxModule.inc");

class NxPage extends NxModule
{
	function __construct($props,&$parentDesc,$aspect=null)
	{
		parent::__construct($props,$parentDesc);
		$GLOBALS['current_page']=&$this;
		

		// setup navigation properties
		$this->toPage=$this->getProperty('page.to_page','#');
		$this->thisUrl=$this->getProperty('thisStep',$this->toPage);

		// property prefix
		$this->view = $this->getProperty('view','');
		$this->pageView = $this->getProperty('page.view','');
        
		// suppression des aspects
		if(defined('NX_PAGE_ASPECT') && NX_PAGE_ASPECT)
			$this->initAspect($aspect);		
	}

	function NxPage($props,&$parentDesc,$aspect=null) {
		return self::__construct($props,$parentDesc,$aspect);
	}

	function __destruct() {
		nxLogStop("FIN APPLICATION",'REQUEST');
	}
	
	function getParams()
	{
		$s="";
		foreach ($_GET as $k=>$v)
		{
			$s .= $k."=".urlencode($v)."&";
		}
		return $s;
	}
	
	function publish() {
		require_once(NX_LIB.'cache/NxCache_publish.inc');
		$cache = new NxCache_publish($this);
		$cache->cleanCache();
	}

	function loadState()
	{
		// setup navigation properties
		$this->setProperty('this.url',isset($_ENV['REQUEST_URI'])?$_ENV['REQUEST_URI']:$_SERVER['REQUEST_URI']); // url after rewrite (ie. with .php and params)
		$this->setProperty('this.external.url',$_SERVER['REQUEST_URI']); // url before rewrite, ie. html type
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
	class NxFrameStack
	description: tracks navigation path from user. Used for getting "back" and/or checking that users
		comes from the right page URL.
		
	NB. this system is probably no longer required.

*/
class NxFrameStack
{
	function __construct(&$props,$withSession,$reset=false)	{
		// property set
		// $this->props=&$props;
		
		if (!isset($_SESSION['_NX_STACK']) || $reset)
		{
			$this->stack=array(array());
			
			if (isset($_SESSION))
				$_SESSION['_NX_STACK']=&$this->stack;
		}
		else
			$this->stack=& $_SESSION['_NX_STACK'];
		
		$this->current=count($this->stack)-1;
		$this->curF=&$this->stack[$this->current];
		
		// store current url (used for back url of next page)
		$this->set('page.url',$_SERVER['REQUEST_URI']);
		
		// retrieve back url
		if ($this->current>0 && isset($this->stack[$this->current-1]['page.url']))
			$this->curF['back.url']=$this->stack[$this->current-1]['page.url'];
		else
			// no previous page => fallback on previous url in properties
			$this->curF['back.url']=$props->getProperty('back.url',NX_BASE_URL);

		// retrieve originating page
		if ($this->current>0 && isset($this->stack[$this->current-1]['page.from_page']))
			$this->curF['page.from_page']=$this->stack[$this->current-1]['page.from_page'];
		else
			// no previous page => fallback on previous url in properties
			$this->curF['page.from_page']=$this->curF['back.url'];
			
			
		// SESSION BASED NAVIGATION (TBD. obsolete ?)
		if ($withSession)
		{
			global $NX_CURRENT_URL;
			if (isset($_SESSION['from_page'])) // page defined as "return point"
				$fp=$_SESSION['from_page'];
			else if (isset($_SESSION['current_page'])) // previous page if exist
				$fp=$_SESSION['current_page'];
			else
				$fp=$NX_CURRENT_URL; // current page
			
			$props->setProperty('page.from_page',$fp);
		
			if (isset($_SESSION['to_page'])) // page defined as "destination point"
				$fp=$_SESSION['to_page'];
			$props->setProperty('page.to_page',$fp);
		
			// storing current page for next page (HTTP_REFERER cant be trusted)
			$props->setProperty('page.current_page',$_SESSION['current_page']=$_SERVER["REQUEST_URI"]);
		}
			
	}
	
	// add another frame (new operation thread)
	function push($props=array()) {
		$this->stack[]=$props;
		$this->current=count($this->stack)-1;
		$this->curF=&$this->stack[$this->current];
		$this->curF['app.params']=$props;
	}
	
	// delete current frame
	function pop() {
		if ($this->current)
			unset($this->stack[$this->current--]);
		else
			$this->stack[0]=array();
	}
	
	function set($p,$v)	{
		$this->curF[$p]=$v;
	}

	function &get($p,$dft=null)	{
		if (isset($this->curF[$p]))
			return $this->curF[$p];
		return $dft;
	}
	
	function getProps() {
		return $this->curF;
	}
	function getParams() {
		return $this->curF['app.params'];
	}
}

?>
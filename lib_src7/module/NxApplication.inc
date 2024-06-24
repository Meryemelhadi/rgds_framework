<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
require_once(NX_LIB."module/NxModule.inc");

// NB:  NxProperties always loaded first (otherwise, the application could not be loaded...)
class NxApplication extends NxModule
{
	var $appName;
	var $operation;
	var $step;
	var $previousStep;
	var $fromPage;
	
	function __construct($appName,$props,&$parentDesc,$aspect=null)
	{
		$this->appName = $appName;
		parent::__construct($props,$parentDesc);
		$this->finished=false;
		$this->loadState();
		$this->initAspect($aspect);		
	}

	function NxApplication($appName,$props,&$parentDesc,$aspect=null) {
		return self::__construct($appName,$props,$parentDesc,$aspect);
	}
	
	function saveState()
	{
		// check if state exists
		if ($this->finished) return;
		
		// save current state in session data
		global $NX_STACK;
		if (isset($NX_STACK))
		{
			$NX_STACK->set('app.operation',$this->operation);
			$NX_STACK->set('app.step',$this->step);
			$NX_STACK->set('page.from_page',$this->fromPage);
		}
	}
	
	function loadState()
	{
		global $NX_STACK;
		// $state =& $NX_STACK->get($this->storeName,array('app.step'=>'','app.operation'=>''));
		if (isset($NX_STACK))
		{
	
			$this->previousOperation = $NX_STACK->get('app.operation','');
			$this->previousStep = $NX_STACK->get('app.step','');
			// $this->operation = 
			$this->previousOperation = $NX_STACK->get('app.operation','');
			$this->fromPage = $NX_STACK->get('page.from_page'); 
		}
		
		// compact application form
		if (isset($_REQUEST['nxa']))
		{
			@list($_REQUEST['nxo'],$_REQUEST['nxs'])=explode('.',$_REQUEST['nxa']);
		}

		// get new state from HTTP params (GET or POST)
		if (isset($_REQUEST['nxo']))
			$this->operation = $_REQUEST['nxo'];
		else {
			$this->operation = '';
			nxLog('no operation defined for page:'.$_SERVER['QUERY_STRING']);
		}

		if (isset($_REQUEST['nxs']))
			$this->step = $_REQUEST["nxs"];
		else			
			$this->step = '';
			
		// store as properties
		$this->setProperty('app.operation',$this->operation);
		$this->setProperty('app.step',$this->step);
		
		// properties for navigation between states (normally overloaded by application state machine)
		if (!isset($this->fromPage))
			$this->fromPage = $this->getProperty('page.from_page','/');

		if (!isset($this->toPage))
			$this->toPage = $this->getProperty('page.to_page',$this->fromPage);

		$this->setProperty('backStep',$this->backUrl=$this->fromPage);
		$this->setProperty('nextStep',$this->nextUrl=$this->toPage);
		
		// setup navigation properties
		$this->setProperty('this.url',isset($_ENV['REQUEST_URI'])?$_ENV['REQUEST_URI']:$_SERVER['REQUEST_URI']); // url after rewrite (ie. with .php and params)
		$this->setProperty('this.external.url',$_SERVER['REQUEST_URI']); // url before rewrite, ie. html type
		$this->thisUrl=$this->getUrl();
	}
	
	function checkPreviousStep($prevStep,$backToOp=null,$backToStep=null,$view='page.error.view')
	{
		// ok if expected step (same or part of a set (wrapped w/ [])
		if (($prevStep == $this->previousStep || !(strpos($prevStep,"[".$this->previousStep."]") === false)) && 
			$this->previousOperation == $this->operation)
			return true;
		else
		{
			$this->operation = $backToOp;
			$this->step = $backToStep;
			$this->setProperty("Error.navigation",$this->getString("navigation error"));
			$MessageProps = array(
				"msg" => $this->getString("Page expired"),
				"button.redirect" => "back",
				"button.url" => $this->getUrl($backToStep,$backToOp)
				);				
			$this->runView($view,$MessageProps);
			return false;
		}
	}
	
	function isPageRefresh()
	{
		if ($this->previousStep == $this->step && $this->previousOperation == $this->operation)
			return true;
		else
			return false;
	}
	
	function getRequest()
	{
		require_once(NX_LIB."NxApp_HttpRequest.inc");
		return new NxApp_HttpRequest();
	}
	
	// store operation parameters
	function startOp()
	{
		if (isset($this->aspect))
			$this->aspect->onStartOperation();
			
		return ;
		
		// get a reference to the parameter slot
		if (!isset($_SESSION))
		{
			return;
		}
		
		// chain params from other operations
		global $NX_STACK;
		if (isset($NX_STACK))
		{
			$NX_STACK->push($_GET);
		}
	}
	
	function finish()
	{
		if (isset($this->aspect))
			$this->aspect->onFinishOperation();
		
		$this->finished=true;
		return ;
		
		// erase state
		// unset($this->step);
		// unset($this->operation);
		
		// pop navigation frame
		global $NX_STACK;
		if (isset($NX_STACK))
		{
			$NX_STACK->pop();	
		}
	}

	function isFinished()
	{
		if ($this->finished)
			return true;
		else
			return false;
	}
	
	// get parameters as a cascading property set (step and operation params)
	function & getParams()
	{
		global $NX_STACK;
		return $this->params=$NX_STACK->getParams();
	}
	
	function & getParam($name,$deft=null)
	{
		if (isset($_REQUEST[$name]))
			return $_REQUEST[$name];
			
		global $NX_STACK;
		if (isset($NX_STACK))
			return $NX_STACK->get($name,$deft);
			
		return $deft;
	}

	function & setParam($name,&$v)
	{
		if (!isset($this->params))
		{
			$this->getParams();
		}
		$this->params->setProperty($name,$v);
		$nxp = $this->storeName."_nxp";
		$_SESSION[$nxp]=$this->params->props;
		$_SESSION[$name]=&$v;
	}
	
	function getUrl($step=null,$operation=null,$url=null)
	{
		if ($operation === null)
		{
			$operation = $this->operation;
			
			if ($step === null)
				$step = $this->step;
		}
		elseif ($step === null)
				$step = '';
			
		if ($url===null)
			$url=$_SERVER['PHP_SELF'];
			
		return  "{$url}?nxo={$operation}&nxs={$step}";
	}
	
	function redirectToOperation($step,$operation=null,$page=null)
	{
		$url=$this->getProperty("site.url.noprefix","").$this->getUrl($step,$operation,$page);
		global $NX_STACK;
		if (isset($NX_STACK))
		{
			$props=$NX_STACK->getParams();
			if (count($props)>2)
			{
				unset($props['nxo']);
				unset($props['nxs']);
				foreach ($props as $k=>$v)
					$url .= "&$k=$v";
			}
		}
					
		header("Location:".$url);
		$this->finish();
		exit();
	}

	// obsolete ?
	function isDefinedTable($tableName)
	{
		return true;
	}
	
	// overload it with actual application code
	// function run()	{echo "Error : application not defined"; }

	function runLogic($module,$props=null,$aspect=null)
	{
		// set view prefix as the module name, without file name,
		$view = $module;
		if ($props===null)
			$props=array();
				
		$props['app.operation']=$this->operation;
		$props['app.step']=$this->step;
		$props['view']=$view;
		if (!isset($props['page.view']))
			// get view in the form table.update.process.view
			$props['page.view']=$this->getProperty($view.'.view');
		else
			// redirected to another view ?
			$props['page.view']=$this->getProperty($props['page.view'],$props['page.view']);
		
		$props['thisStep']=$GLOBALS["NX_CURRENT_URL"];
		
		$props['app.id']=str_replace('.php','',$_SERVER['SCRIPT_NAME']).'/'.$props['app.operation'];
				
		return NxProperties::runLogic($module,$props,$aspect);
	}
	
	function & runCachedLogic($module,$props=null,$propName=null,$aspect=null)
	{
		$props['app.operation']=$this->operation;
		$props['app.step']=$this->step;
		$props['page.view']=$this->getProperty($this->operation.'.'.$this->step.'.view');
		$props['thisStep']=$GLOBALS["NX_CURRENT_URL"];
		
		return NxProperties::runCachedLogic($module,$props,$propName,$aspect);
	}
}
?>
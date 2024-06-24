<?php
  /* logic aspect : set of callback function called by the NX servlet engine. */
  class aspects_application_default extends NxModule {
  	  
  	  function aspects_application_default(&$application){
  	  	  $this->NxModule($application->props,$application->parent);
  	  	  $this->application=&$application;
	  }
	  
	  function onStartOperation()
	  {
	  	  $application=$this->application;
	  	  nxLog("start operation {$this->application->appName}::{$application->operation}.{$application->step}  @{$application->package}",'aspect');
	  	  return true;
	  }
	  function onFinishOperation()
	  {
	  	  $application=$this->application;
	  	  nxLog("finish operation {$application->appName}::{$application->operation}.{$application->step}  @{$application->package}",'aspect');
	  }
	  
	  function onBeforeRun($name,$module)
	  {
	  	  $application=$this->application;
	  	  nxLog("{$application->appName}::{$application->operation}.{$application->step} @{$application->package} : onBeforeRun $name",'aspect');
	  	  return true;
	  }

	  function onRun($name,$module)
	  {
	  	  $application=$this->application;
	  	  nxLog("{$application->appName}::{$application->operation}.{$application->step} @{$application->package} : onRun $name",'aspect');
	  }
	    	  
  }
?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
$GLOBALS['current_page']=null; 

class NxModule extends NxProperties
{
	var $errorLog='';
	var $okLog='';
	var $warnLog='';
	var $aspect = null;

	function __construct($props,&$parentDesc)
	{
		parent::__construct($props,$parentDesc);
				
		// error handler
		$this->error = new NxErrorCB();
	}
	
	function initAspect($aspect=null) {
		if (!$aspect && isset($this->parent->aspect))
			$this->aspect=&$this->parent->aspect;
		elseif(($aspectN = $aspect) || ($aspectN=$this->getProperty('page.aspect')))
		{
			$className=strtr("aspects_application_{$aspectN}",'.','_');
			if (!class_exists($className) &&
				($f = $this->getConfFile('aspects/application',strtr($aspectN,'.','/').'.inc')))
				include_once($f);
								
			if (class_exists($className))
				$this->aspect=new $className($this);
			else
				$this->aspect=null;
		}
		
		return $this->aspect;
	}
	
	// overload it with actual application code
	// function run()	{echo "Error : application not defined"; }
	
	function cachedRun($propName=null)
	{
		ob_start();
			$this->run();
			$ret = ob_get_contents();
		ob_end_clean();
		
		if ($propName != null)
			$this->setProperty($propName,$ret);
			
		return $ret;
	}

	function getDS(&$media,&$view) 
	{
		// override media type ex. handler:prod.record
		if ($view)
		{
			$view2=explode(':',$view);
			if (count($view2)>1)
			{
				if ($media=='db' || $media=='handler')
					$media = $view2[0];

				$view = $view2[1];
			}
		}

		// split media.instance
		$plug=explode('.',$media);
		if (count($plug)>1)
			$inst=$plug[1];
		else
			$inst=false;

		// get data source plugin
		$ds = $this->getPlugIn(strtolower($plug[0]),'NxDS_','ds',true,$inst);

		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","MODULE");
			return null;
		}

		$this->error->clear();
	
		// get view
		if ($view===null)
			$view=$this->view;

		return $ds;
	}
	
	// get a record from DB and sets it to a property
	function getRecord($view=null,$propName=null,$media='db',$addProps=false,$data=null,$props=null)
	{
		// get ds from media and view
		$ds = $this->getDS($media,$view);

		// get records
		$records = $ds->getRecords($view, $this->error,$data,$props);
		
		// get first record from collection
		if ($records != null && $records->count() > 0)
		{
			$recIt = $records->getRecordIterator();
			$record = & $recIt->getCurrent();
			if ($propName)
				$this->setProperty($propName,$record);
				
			if ($addProps===true)
				$this->addProperties($record->getProperties(false,'HTML'));
				
			return $record;
		}
		if ($propName)
			$this->setProperty($propName,null);
		return null;
	}
	
	// count records from records collection if exists, or from DB
	function countRecords($view=null,$records=null,$media='DB',$data=null,$props=null)
	{
		if ($records!=null)
		{
			return $records->count();
		}

		// get ds from media and view
		$ds = $this->getDS($media,$view);
		
		// get records
		$records = & $ds->getRecords($view, $this->error,$data,$props);

		// return record collection if ok and set property
		if ($records != null) // && $records->count() > 0)
			return $records->count();

		return 0;
	}	

	// get a record from DB and sets it to a property
	function getRecords($view=null,$propName=null,$media='DB',$addProps=false,$data=null,$props=null)
	{
		// get ds from media and view
		$ds = $this->getDS($media,$view);

		// get records
		$records = $ds->getRecords($view, $this->error, $data,$props);

		// return record collection if ok and set property
		// if (($records != null) && $propName) // && $records->count() > 0)
		if ($propName)
			$this->setProperty($propName,$records);

		// add properties from first record of collection if only one record
		if ($addProps===true && $records != null && $records->count() >= 1)
		{
			$recIt = $records->getRecordIterator();
			$record = & $recIt->getCurrent();

			$this->addProperties($record->getProperties(false,'HTML'));
		}
			
			
		return $records;
	}	

	// writer: send data to an I/O connexion using given data source
	function putRecords(&$records,$view=null,$op='insert',$media='db',$props=null)
	{
		// get ds name
		if ($dsName = $this->getProperty($view.'.ds',null))
		{
			// get data source descriptor
			$ds=$this->loadResource("ds/$dsName",$this->error);
				
			// get media
			$media=$this->getProperty($dsName.'.media',$media);
		}
		else 
		{
			$dsName = $view;
		}

		// get ds from media and view
		$ds = $this->getDS($media,$view);
		
		if (method_exists($records,'getRecordIterator'))
			// multiple records
			return $ds->putRecords($records,$view,$this->error,$op,$props=null);
		elseif (is_array($records))
		{
			// data as simple array, to be wrapped up as record
			if (($recDesc = & $ds->getRecDesc($view,$errorCB))==null)
				return null;
			require_once(NX_LIB . "NxData_RecordInputSource.inc");
			$record = new NxData_RecordInputSource($recDesc);
		 	$record->initRecord($records, "form", $errorCB);
		 	
		 	return $ds->putRecord($record,$view,$this->error,$op,$props);
		}
		else
		{
			// single record
			return $ds->putRecord($records,$view,$this->error,$op,$props);
		}
	}
	
	// writer: send data to an I/O connexion using given data source
	function putRecord(&$record,$view=null,$op='insert',$media='db',$format='Object',$props=null)
	{
		/*
		if (!$record)
		{
			nxError("can't store empty record to $media",'NX_DS');
			return null;
		}
		*/
		
		// get ds name
		if ($dsName = $this->getProperty($view.'.ds',null))
		{
			// get data source descriptor
			$ds=$this->loadResource("ds/$dsName",$this->error);
				
			// get media
			$media=$this->getProperty($dsName.'.media',$media);
		}
		else {
			$dsName = $view;
		}

		// get ds from media and view
		$ds = $this->getDS($media,$view);
		
		if (is_array($record))
		{
			// data as simple array, to be wrapped up as record
			if (($recDesc = & $ds->getRecDesc($view,$errorCB))==null)
				return null;
			require_once(NX_LIB."NxData_RecordInputSource.inc");
			
			// transform into array of records
			if (!isset($record[0]) || !is_array($record[0]))
				$arecords=array($record);
			else
				$arecords=$record;
				
			foreach ($arecords as $record)
			{
				$records = new NxData_RecordInputSource($recDesc);
			 	$records->initRecord($record, $format, $this->error);
				$ds->putRecord($records->getRecord(),$view,$this->error,$op,$props);
			}
			return;
		}
		else
			return $ds->putRecord($record,$view,$this->error,$op,$props);
	}

	function getLastInsertID($media='db')
	{
		// get ds from media and view
		$ds = $this->getDS($media,$view);

		$ds->getLastInsertID($id,$this->error);
		return $id;
	}
	
	function deleteRecords($view=null, &$records,$media='DB', $deleteRes=true,$props=null)
	{
		// get ds from media and view
		$ds = $this->getDS($media,$view);
		
		// delete resources (files etc.)
		if (!$this->error->isError() && $deleteRes && $records!=null)
			$records->onDelete($this->error);

		// delete records
		$ds->deleteRecords($view, $records, $this->error,$props);

		// trigger aspect
		if (!$this->error->isError() && $records!=null)
			$records->onAfterDelete($this->error);	
	}
	
	// redirect to another page
	function redirectTo($propName,$urlParams="")
	{
		header("Location:".$this->getProperty($propName,$propName).$urlParams);
	}

	function getHandler($class,$root='')
	{
		$className=$this->loadClass($class);
		if ($className)
		{
			return new $className($GLOBALS['current_page']);
		}

		return null;
	}

	/* error management : keep message in session so that it can be displayed in next screen...*/
	function initMsg() {
		if(isset($_SESSION['log.ok']))
			$this->setProperty('log.ok',$this->okLog=$_SESSION['log.ok']);

		if(isset($_SESSION['log.errors']))
			$this->setProperty('log.errors',$this->errorLog=$_SESSION['log.errors']);

		if(isset($_SESSION['log.warn']))
			$this->setProperty('log.warn',$this->errorLog=$_SESSION['log.warn']);
	}

	function msgError($msg) {
		$msg = '<span class="label">'.__('Error').' : </span>'.$msg;
		$this->errorLog .= '<div id="error_log">'.$msg .'<span class="x" onclick="$(this.parentNode).hide()" style="color:#93AC86;font-size:16px;padding:0px;float:right;margin-top:-8px">x</span></div>';
		$this->_msgStore('log.errors',$this->errorLog);
	}

	//ME
	function msgErrorBs($msg) {
		$msg = '<strong>'.__('Error').' : </strong>'.$msg;
		$this->errorLog .= '  <div class="alert alert-danger fade in alert-dismissable"><a onclick="$(this.parentNode).hide()"  class="close" data-dismiss="alert" aria-label="close" title="close">×</a>'.$msg .'</div>';
		$this->_msgStore('log.errors',$this->errorLog);
	}
	function addError($msg) {
		$this->msgError(__($msg));
	}
	function msgOk($msg) {
		$msg = '<span class="label">'.__('Success').' : </span>'.$msg;
		$this->okLog .= '<div id="ok_log">'.$msg .'<span class="x" onclick="$(this.parentNode).hide()" style="color:#93AC86;font-size:16px;padding:0px;float:right;margin-top:-8px">x</span></div>';
		$this->_msgStore('log.ok',$this->okLog);
	}
	//ME
	function msgOkBs($msg) {
		$msg = '<strong>'.__('Success').' : </strong>'.$msg;
		$this->okLog .= '  <div class="alert alert-success fade in alert-dismissable"><a onclick="$(this.parentNode).hide()"  class="close" data-dismiss="alert" aria-label="close" title="close">×</a>'.$msg .'</div>';
		$this->_msgStore('log.ok',$this->okLog);
	}
	function msgOk2($msg) {
		$msg = __('Success').' : '.$msg;
		$this->okLog .= '<div id="ok_log">'.$msg .'</div>';
		$this->_msgStore('log.ok',$this->okLog);
	}
	function msgWarning($msg) {
		$msg = '<span class="label">'.__('Warning').' : </span>'.$msg;
		$this->warnLog .= '<div id="warn_log">'.$msg .'<span class="x" onclick="$(this.parentNode).hide()" style="color:#93AC86;font-size:16px;padding:0px;float:right;margin-top:-8px">x</span></div>';
		$this->_msgStore('log.warn',$this->warnLog);
	}
	function clearMsg() {
		if(defined('KEEP_SESSION') && KEEP_SESSION==false)
			session_start();

		if(isset($_SESSION['log.warn']))
			unset($_SESSION['log.warn']);

		if(isset($_SESSION['log.errors']))
			unset($_SESSION['log.errors']);

		if(isset($_SESSION['log.ok']))
			unset($_SESSION['log.ok']);
	}

	function _msgStore($type,$msg) {
		$this->setProperty($type,$msg);

		if(defined('KEEP_SESSION') && KEEP_SESSION==false)
			session_start();
        $_SESSION[$type] = $msg;
	}

}

?>
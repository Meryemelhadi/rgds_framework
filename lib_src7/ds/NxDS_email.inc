<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// plugin for creating page items

require_once(NX_LIB.'ds/NxDS_Source.inc');

class NxDS_email extends NxDS_Source
{
	function __construct(&$props)
	{
		parent::__construct($props);
	}

	// implement a POP/IMAP client ?
	function getRecords($view, &$errorCB,$data=null)
	{
		return false;
	}	

	function getRecordCount($view, &$errorCB)
	{		
		// check presence of fields ?
		// $_POST[]
		return 1;
	}

	function putRecord(&$record,$view,&$errorCB,$dbOp=null)
	{
		return $this->putRecords($record,$view,$errorCB,$dbOp);
	}
	
	function putRecords(&$records,$view,&$errorCB,$dbOp=null)
	{
		// get connection params
		$cnx=$this->getProperty("{$view}.connexion");
		if ($cnx)
		{
			$this->loadResource("connexion/$cnx",$errorCB);
			$servers=$this->getProperty("connexion.{$cnx}.smtp.servers",'');
			$from=$this->getProperty("connexion.{$cnx}.from");
			$islog = $this->getProperty("connexion.{$cnx}.log",false);
		}
		else
		{
			$servers=$this->getProperty("email.smtp.servers");
			$from=$this->getProperty("email.smtp.from");
			$islog = $this->getProperty("{$view}.log",false);
		}
		
		// get headers from data source
		$to = $this->getProperty("{$view}.to",null);
		$cc = $this->getProperty("{$view}.cc",'');
		$bcc = $this->getProperty("{$view}.bcc",'');
		$subject = $this->getProperty("{$view}.subject",null);
		
		// get html body (view + props) from view execution
		$body= $this->getProperty("{$view}.body",null);
			
		// send email
		require_once(NX_LIB.'ds/email/NxMail_MIME.inc');
		$mime = new NxMail_MIME(
			$from, 
			$to,
			$cc,
			$bcc,
			$subject, 
			$body, 
			null,
			$servers,
			$islog);

		/*
		if ($props->getProperty('view.'.$view.'.html',null))
		{
			$html =& $props->runView('view.'.$view.'.html', 
				array("records" => $records, 'fmethod' => 'toHTML', 'baseURL' => $baseURL,
				'email.subject' => $subject), 'dummy');
			$mime->attach($html, "HTML message", HTML, BASE64);
		}
		*/

		if ($mime->send_mail())
		{
			if ($islog)
				nxLog("Success: LOG:".$mime->getLog(),'MAIL');
				
			return true;
		}
		else
		{
			nxLog('Failed: LOG:'.$mime->getLog(),'MAIL');
		}			
	}
	
	// ds interface
	function deleteRecords($view, &$records, &$errorCB)
	{
		return true;
	}	
}
?>
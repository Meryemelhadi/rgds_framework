<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'ds/email/NxMail_MIME.inc');

class NxPlug_Email
{
	var $props;
	
	function  NxPlug_Email(& $props)
	{
		$this->props = & $props;
	}

	// send an email using given view to email found in record ("to" field with given role)
	function sendEmail($view, &$records, $baseURL=null, $cancelURL=null,$checkURL=null,$emailRole="login.email")
	{
		$props = & $this->props;

		// get "to" field
		$record = & $records->getRecord();
		$to = $record->getProperty($emailRole,null,false,"String");
		if ($to === null)
			$to = $props->getProperty($view.".to",null);

		// get "from" and "cc" field from property
		$from = $record->getProperty($view.".from",null,false,"String");
		if ($from == null)
			$from = $props->getProperty($view.".from",null);
			
		$cc = $record->getProperty($view.".cc",null,false,"String");
		if ($cc == null)
			$cc = $props->getProperty($view.".cc",'');
			
		$bcc = $record->getProperty($view.".bcc",'',false,"String");
		if ($bcc == null)
			$bcc = $props->getProperty($view.".bcc",null);
			
		$subject = $record->getProperty($view.".subject",null,false,"String");
		if ($subject == null)
			$subject = $props->getProperty($view.".subject",null);
		
		// get html body (view + props) from view execution
		// get email details from view
		$viewProps = array(
			"records" => $records,
			"fmethod" => "toHTML",
			"baseURL" => $baseURL,
			"email.subject" => $subject
		);
				
		// get text body (view + props) from view execution
		$body =& $props->runView('view.'.$view.'.text', 
			array("records" => $records, 'fmethod' => 'toString',
			'email.subject' => $subject), 'dummy');
			
		// send email
		$mime = new NxMail_MIME($from, $to, $cc, $bcc, $subject, $body, $props->getProperty("email.smtp.headers",""), 
			$props->getProperty("email.smtp.servers",""),true);
			
		if ($props->getProperty('view.'.$view.'.html',null))
		{
			$html =& $props->runView('view.'.$view.'.html', 
				array("records" => $records, 'fmethod' => 'toHTML', 'baseURL' => $baseURL,
				'email.subject' => $subject), 'dummy');
			$mime->attach($html, "HTML message", HTML, BASE64);
		}

		// echo "Mail to: [$to] bcc:[$bcc]<br>";
		if ($mime->send_mail())
		{
			//echo "Success: LOG:<pre>".$mime->getLog("html")."</pre>";
			return true;
		}
		else
		{
			//echo "Failed: LOG:<pre>".$mime->getLog("html")."</pre>";
		}
		
		// Email error
		$viewProps = array(
			"msg" => "Error can't send email:".$mime->errstr, 
			"b1.redirect" => $view.".cancel",
			"b1.url" => $cancelURL,
			"b2.redirect" => $view.".checkEmail",
			"b2.url" => $checkURL
		);				
		$props->runView('view.dialog2buttons',$viewProps);
		return false;
	}
}
?>

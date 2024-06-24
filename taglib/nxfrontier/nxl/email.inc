<?PHP

// data sources
class taglib_nxfrontier_nxl_email extends NxTagLib
{	
	function header()
	{
		return "// import library email ";
	}
	
	/* <email:send
					subject="#string:your details#"
					text="#property:email.text#" 
					html="#property:email.html#"
					to="#property:user.userID#"
					connexion="register.email.ds"	/>
	*/
	function send($node,$content,$level,$pad)
	{		
		// load email lib
		$this->context->addInclude('ds/email/NxMail_MIME.inc');
		
		if ($this->getAttribute($cnx,$node,'ds'))
		{
			// load data source properties
			$src= "\n{$pad}\$dsID=\$this->getProperty('{$cnx}.ds','{$cnx}');";
			$src.= "\n{$pad}\$this->loadResource('ds/'.\$dsID.NX_HOST_SWITCH,\$this->error,true);";
			
			if (!$this->getAttribute($subject, $node,'subject',null,true,true) || $subject=="''")
				$subject = "\$this->getProperty(\$dsID.\".subject\")";
				
			if (!$this->getAttribute($to, $node,'to',null,true,true) || $to=="''")
				$to = "\$this->getProperty(\$dsID.\".to\")";
				
			if (!$this->getAttribute($from, $node,'from',null,true,true) || $from=="''")
				$from = "\$this->getProperty(\$dsID.\".from\")";
				
			if (!$this->getAttribute($cc, $node,'cc',null,true,true)|| $cc=="''")
				$cc = "\$this->getProperty(\$dsID.\".cc\")";
				
			if (!$this->getAttribute($bcc, $node,'bcc',null,true,true) || $bcc=="''")
				$bcc = "\$this->getProperty(\$dsID.\".bcc\")";
				
			if (!$this->getAttribute($smtp, $node,'smtp',null,true,true) || $smtp=="''")
				$smtp = "\$this->getProperty(\$dsID.\".smtp.servers\")";
				
			if (!$this->getAttribute($headers, $node,'headers',null,true,true) || $headers=="''")
				$headers = "\$this->getProperty(\$dsID.\".smtp.headers\")";

			if (!$this->getAttribute($log, $node,'log',null,true,true) || $log=="''")
				$log = "\$this->getProperty(\$dsID.\".log\")";
		}
		else 
		{
			$src='';
			$this->getAttribute($to,$node,'to','',true,true);
			$this->getAttribute($subject,$node,'subject','',true,true);
			$this->getAttribute($from,$node,'from','',true,true);
			$this->getAttribute($cc,$node,'cc','',true,true);
			$this->getAttribute($bcc,$node,'bcc','',true,true);
			$this->getAttribute($smtp,$node,'smtp','localhost',true,true);
			$this->getAttribute($headers,$node,'headers','',true,true);
			$this->getAttribute($log,$node,'log',false,true,true);
		}
				
		$this->getAttribute($text,$node,'text','',true,true);
		
		// send email
		$src.= "\n{$pad}\$mime = new NxMail_MIME($from, $to, $cc, $bcc,$subject, $text, $headers,$smtp,$log);";
			
		if ($this->getAttribute($html,$node,'html',null,true,true))
		{
			$src.= "\n{$pad}\$mime->attach($html, 'HTML message', HTML, BASE64);";
		}
		
		$src .= $content;
		
		$src.="\n{$pad}if (!\$mime->send_mail()) {
{$pad}	nxLog('Failed: LOG:<pre>'.\$mime->getLog('html').'</pre>','EMAIL');
{$pad}	\$this->error->addError(\$this->getString('error sending email'));
{$pad}}";
		
		return $src;
	}
	
	function attach($node,$content,$level,$pad)
	{		
		if (!$this->getAttribute($file,$node,'file',null,true,true))
			return $this->fatal('no file attribute');
			
		$this->getAttribute($description,$node,'description','',true,true);
		$this->getAttribute($encoding,$node,'encoding','base64',true,true);
		$this->getAttribute($content_type,$node,'content_type','application/octet-stream',true,true);
		$this->getAttribute($content_disposition,$node,'content_disposition','',true,true);
			
		return "\n{$pad}\$mime->fattach($file, $description, $content_type, $encoding,$content_disposition);";
	}	
	
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/* TODO: 
	1) when to,cc or bcc have more than one address => split up headers and generate RCPT TO:
	2) EHLO command to servers => handle 250 codes listing capabilities.
	3) Mass mailing (newsletters): send data per chunks of 100.
	4) handle each bcc recipient with one message.
	5) check attachments (html) with qmail (body to put as a part as well ?)
	6) encode text body when using utf-8 charset = ok done 30/05/2007
*/


/* -----------------------------------------------------------
   Constants used inside the class.   This file is included
   by the class module and these constants an be freely
   used in scripts using MIME_mail class
 ----------------------------------------------------------- */

	define('BASE64', 'base64');
//	define('BIT7', '8bit');
	define('BIT7', '7bit');
	define('QP', 'quoted_printable');
	define('NOSUBJECT', '(No Subject)');
	define('WARNING', 'This is a MIME encoded message');
	define('OCTET', 'application/octet-stream');
	define('TEXT', 'text/plain');
	define('HTML', 'text/html');
	define('EVENT', 'text/calendar');
	define('JPEG', 'image/jpg');
	define('GIF', 'image/gif');
	define('CRLF', "\r\n");
	define('CHARSET', 'us-ascii');
	define('INLINE', 'inline');
	define('ATTACH', 'attachment');
	define('BODY', CRLF.'BODY'.CRLF);

/* ---------------------------------------------------------
    NxMail_ Class:
    Allows creation of e-mail messages via the MIME Standard.
    The class supports multiple attachments and presenting 
    an e-mail in HTML.
 --------------------------------------------------------- */
//  include ("MIME.def");

 class NxMail_MIME {
 //public:
  var $to;
  var $from;
  var $replyto;
  var $subject;
  var $body;
  var $headers = "";
  var $errstr="";

  // these are the names of the encoding functions, user
  // provide the names of user-defined functions

  var $base64_func= '';		# if !specified use PHP's base64  
  var $qp_func = '';      	# None at this time

  // If do not have a local mailer..use this array to pass info of an SMTP object
  // e.g. $mime_mail->mailer = array('name' => 'smtp', method => 'smtp_send()');
  // 'name' is the name of the object less the $ and 'method' can have parameters
  // specific to itself.  If you are using MIME_mail object's to, from, etc.
  // remember to send parameters a literal strings referring 'this' object!!!!
  // If in doubt, you are probably better off subclassing this class...
  var $mailer = "";	# Set this to the name of a valid mail object

 //private:
  var $mimeparts = array(); 

 // Constructor.
 function __construct($from='', $to='',$cc ='',$bcc='',$subject="", $body="", $headers = "", $params=array(),$log=false) 
 {

 	// get smtp server list
	$this->initServerCandidates($from,$params); 	
	
	$replyto=$from;
	if( defined('EMAIL_CATCH_REPLY_TO') ) {
		$replyto = EMAIL_CATCH_REPLY_TO;
	}

	if( defined('EMAIL_CATCH_TO') ) {
		$to = EMAIL_CATCH_TO;
	}
	if( defined('EMAIL_CATCH_FROM') ) {
		$from = EMAIL_CATCH_FROM;
	}

	// email params
	// todo: split recipients as arrays (only one recipient per line supported now...)
	$this->to 		= trim($to);
	$this->from		= $from;
	$this->replyto	= $replyto;
	$this->cc		= $cc;
	$this->bcc		= $bcc;
	
	$this->subject	= $subject;
	$this->body		= $body;
	if (is_array($headers)) 
	{
		if (sizeof($headers)>1) 
			$headers=join(CRLF, $headers);
		else
			$headers=$headers[0];
	}
	
	if ($from && $headers) 
	{
		$headers = preg_replace("!(from:\ ?.+?[\r\n]?\b)!i", '', $headers);
	}
	
	
	$this->headers = chop($headers);
	$this->mimeparts[] = "" ;	//Bump up location 0;
	$this->errstr = "";
	if ($log)
		$this->log="start log";
	else
		$this->log=null;
	return;
 }

function startTLS()
{
    $res = $this->cmd('STARTTLS');
	if ($res!='220' && $res!='250') {
		return false;
	}
	
	//Allow the best TLS version(s) we can
	$crypto_method = STREAM_CRYPTO_METHOD_TLS_CLIENT;
	//PHP 5.6.7 dropped inclusion of TLS 1.1 and 1.2 in STREAM_CRYPTO_METHOD_TLS_CLIENT
	//so add them back in manually if we can
	if (defined('STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT')) {
		$crypto_method |= STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT;
		$crypto_method |= STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT;
	}

	// Begin encrypted connection
	// set_error_handler(array($this, 'errorHandler'));
	$crypto_ok = stream_socket_enable_crypto(
		$this->con,
		true,
		$crypto_method
	);
	//restore_error_handler();
	return $crypto_ok;
}

 function & getLog($format='text')
 {
 	if ($format=='text')
 		return $this->log !== null ? $this->log:'';
 	
	$html = str_replace("<","&lt;",$this->log);
	$html = str_replace(">","&gt;",$html);
	$html = str_replace("\r\n","[CRLF]<BR>",$html);
	$html = str_replace("\r","[CR]<BR>",$html);
	$html = str_replace("\n","[LF]<BR>",$html);
	
	return $html;
 }
 
 function initServerCandidates($from,&$params)
 {
 	// server candidate array
 	$this->server = array();
 	
 	// params for connecting to the server (list provided)
 	if (@isset($params[0]) && is_array($params[0]))
 	{
 		// list of potential servers
	 	foreach ($params as $hostPars)
	 		$this->addServerConf($hostPars);
 	}
 	else if (@isset($params["host"])||isset($params["domain"]))
 	{
 		// only one server
 		$this->addServerConf($params);
 	}
 	else
 	{
 		// no server => try to get it from "from" field
		list($me,$mydomain) = explode("@",$from);
 		$this->addServerConf(array("domain"=>$mydomain));
 	} 	
 }
 
 function & addServerConf($params)
 {
 	// create server array
 	if (!isset($this->server))
	 	$this->server = array();

	// set default parameters
	$par = array();
	$par["port"] = 25;
	$par["helo"] = str_replace('_','',$_SERVER['SERVER_NAME']);
	$par["auth"] = false;
	$par["tls"] = false;
	$par["user"] = "";
	$par["password"] = "";
	$par['method'] = 'smtp';
	$par["authenticated"] = false;

	foreach($params as $key => $value){
		$par[$key] = $value;
	} 	

	// check host or domain
	if (isset($par["host"]))
	{
		// we get an explicit host
		$this->server[] =& $par;
	}
	else if (isset($par["domain"]))
	{
		// no host but domain => get email servers
		
		// get ip server
		$mx = array();
		$weight = array();
		if($this->getmxrr($par["domain"],$mx,$weight))
		{
			// Try them in order of lowest weight first
			array_multisort($mx,$weight);
		}
		
		foreach($mx as $ip)
		{
			// add a param set for each potential server
			$par["host"] = $ip;
	 		$this->server[] = $par;
		}
	}
	else
	{
		// no host or domain, assume we have a local one...
		$par["host"] = "localhost";
		$this->server[] =& $par;
	}
 }
 
 /* ---------------------------------------------------------
  Attach a 'file' to e-mail message
  Pass a file name to attach.
  This function returns a success/failure code/key of current
  attachment in array (+1). Read attach() below.
 --------------------------------------------------------- */
 function fattach($path, $description = "", $contenttype = OCTET, $encoding = BASE64, $disp = '') 
 {
	$this->errstr = "";

	if (!file_exists($path)) 
	{
		$this->errstr = "File does not exist";
		return 0;
	}
	
	// Read in file
	$fp = fopen($path, "rb");	# (b)inary for Win compatability
	if (!$fp) {
		$this->errstr = "fopen() failed";
		return 0;	//failed
	}

	$contenttype .= ";\r\n\tname=".basename($path);
	$data = fread($fp, filesize($path));
	return $this->attach($data, 
			$description, 
			$contenttype,
			$encoding,
			$disp);	
 }

 /* ---------------------------------------------------------
  Attach data provided by user (rather than a file)
  Useful when you want to MIME encode user input
  like HTML. NOTE: This function returns key at which the requested
  data is attached. IT IS CURRENT KEY VALUE + 1!!
  Construct the body with MIME parts
 --------------------------------------------------------- */
 function attach($data, $description = "", $contenttype = OCTET, $encoding = BASE64, $disp = '') 
 {
	$this->errstr = "";
	if (empty($data)) 
	{
		$this->errstr = "No data to be attached";
		return 0;
	}
	
	if (trim($contenttype) == '') $contenttype = OCTET ;
	
	// encoding
	if (trim($encoding) == '') $encoding = BASE64;
	if ($encoding == BIT7) $emsg = $data;
	elseif ($encoding == QP) 
		$emsg = $$this->qp_func($data);
	elseif ($encoding == BASE64) {
		if (!$this->base64_func) 	# Check if there is user-defined function
			$emsg = base64_encode($data);
		else 
			$emsg = $$this->base64_func($data);
	}
	
	$emsg = chunk_split($emsg);
	
	//  patch fy 30/05/2007 for charset support 
	global $NX_CHARSET;
	
	//Check if content-type is text/plain and if charset is not specified append default CHARSET
	if ((preg_match("!^".TEXT."!i", $contenttype)||preg_match("!^".HTML."!i", $contenttype)) && !preg_match("!;charset=!i", $contenttype)) 
		$contenttype .= ";\r\n\tcharset=\"".$NX_CHARSET.'"';
		//$contenttype .= ";\r\n\tcharset=".CHARSET ;
		
	$msg = 
		sprintf("Content-Type: %sContent-Transfer-Encoding: %s%s%s%s",
			$contenttype.CRLF, 
			$encoding.CRLF,
			((($description) && (BODY != $description))?"Content-Description: $description".CRLF:''),
			($disp?"Content-Disposition: $disp".CRLF:""),
			CRLF.$emsg.CRLF);

	if(BODY==$description)
		$this->mimeparts[0] = $msg;
	else
		$this->mimeparts[] = $msg;
	
	return sizeof($this->mimeparts);
 }

 /* ---------------------------------------------------------
  private:
  Construct mail message header from info already given.
  This is a very important function.  It shows how exactly
  the MIME message is constructed.
 --------------------------------------------------------- */
 function build_message() 
 {
	$this->errstr = "";
	$msg = "";
	$boundary = 'PM'.chr(rand(65, 91)).'------'.md5(uniqid(rand()));	# Boundary marker
	$nparts = sizeof($this->mimeparts);

 // Case 1: Attachment list is there.  Therefore MIME Message header must have multipart/mixed
 	if (is_array($this->mimeparts) && ($nparts > 1)):
		$c_ver = "MIME-Version: 1.0".CRLF;
		$c_type = 'Content-Type: multipart/mixed;'.CRLF."\tboundary=\"$boundary\"".CRLF;
		$c_enc = "Content-Transfer-Encoding: ".BIT7.CRLF;
		$c_desc = isset($c_desc)?"Content-Description: $c_desc".CRLF:"";
		$warning = CRLF.WARNING.CRLF.CRLF ;

 // Since we are here, it means we do have attachments => body must become an attachment too.
		if (!empty($this->body)) {
			$this->attach($this->body, BODY, TEXT, BIT7);
		}

 // Now create the MIME parts of the email!
		for ($i=0 ; $i < $nparts; $i++) {
			if (!empty($this->mimeparts[$i])) 
	     		  $msg .= CRLF.'--'.$boundary.CRLF.$this->mimeparts[$i].CRLF;
		}
		$msg .= '--'.$boundary.'--'.CRLF;
		$msg = $c_ver.$c_type.$c_enc.$c_desc.$warning.$msg;
	else:
		if (!empty($this->body)) $msg .= $this->body.CRLF.CRLF;
	endif;
	return $msg;
 }


 /* ---------------------------------------------------------
  public:
  Now Generate the entire Mail Message, header and body et al.
 --------------------------------------------------------- */
 function gen_email($force=false) 
 {
	$this->errstr = "";
	if (!empty($this->email) && !$force) return $this->email ;  // saves processing
	$email = "";
	if (empty($this->subject)) $this->subject = NOSUBJECT;
//	if (!empty($this->from)) $email .= 'From: '.$this->from.CRLF;
	if (!empty($this->headers)) $email .= $this->headers.CRLF;
	$email .= $this->build_message();
	$this->email = $email;
	return $this->email;
 }

 /* ---------------------------------------------------------
  public:
  Printable form
 --------------------------------------------------------- */
 function print_mail($force=false) 
 {
	$this->errstr = "";
	$email =  $this->gen_email($force);
	if (!empty($this->to)) $email = 'To: '.$this->to.CRLF.$email;
	if (!empty($this->subject)) $email = 'Subject: '.$this->subject.CRLF.$email;
	print $email;
 }

 /* ---------------------------------------------------------
  public:
  Send mail via local mailer
 --------------------------------------------------------- */
 function send_mail($force=false) 
 {
	if( defined('EMAIL_SEND') && !EMAIL_SEND ) {
		return true;
	}

	$this->errstr = "";
	$email = $this->gen_email($force);
	if (empty($this->to)) {
		$this->errstr = "To Address not specified";
		return 0;
	}
	if (is_array($this->mailer) && (1 ==  sizeof($this->mailer)) ) {
		$mail_obj = $this->mailer['name'];
		$mail_method = $this->mailer['method'];
		if (empty($mail_obj)) {
			$this->errstr = "Invalid object name passed to send_mail()";nxError($this->errstr,"MAIL");
		}
		global $mail_obj;
		eval("$ret = \$$mail_obj".'->'."$mail_method;");
		return $ret;
	}	

	// NB. the mail function doesn't support long attachments under windows => reimplemented
//	if (substr(php_uname(), 0, 7) == "Windows")
//	if (true)
		return $this->mail($this->to, $this->subject,$email);	
 }
 
 	function mail_sendmail($to,$subject,$msg,$headers='') 
	{		
		$headers="From: {$this->from}\r\n" . "Reply-To: {$this->from}\r\n";
			
		if ($this->cc!='')
			$headers .= "Cc: {$this->cc}\r\n";
		
		if ($this->bcc!='')
			$headers .= "Bcc: {$this->bcc}\r\n";
		
		 return mail($this->to, $this->subject, "", $headers.$email);	
	}
	
 
	function mail($to,$subject,$msg,$headers='') 
	{	
		
		foreach($this->server as $server) 
		{
			if ($server['method']=='sendmail')
				if($this->mail_sendmail($to,$subject,$msg,$headers))
				{
					nxLog("Email sent with success: through server:{$server["host"]}".$this->log,"Nx MIME MAIL");
					return true;
				}
				else
				{
					if ($this->log)
						$this->log .= "sending mail (sendmail method) ".$server["host"]." failed";
						
					nxError("sending mail (sendmail method) ".$server["host"]." failed","mail");
					continue;
				}
				
			
			// open connexion to server (close it if already opened)
			if (!$this->openCon($server["host"],$server["port"]))
			{
				nxError("C:connection to ".$server["host"]." failed","mail");
				
				if ($this->log)
					$this->log .= "C:connection to ".$server["host"]." failed";
					
				continue;
			}
				
			// get prompt
			if($this->getCode() != "220") continue;

			// authenticate if requested	
			if($server["auth"] && !$server["authenticated"])
			{
				if ($this->cmd("HELO ".$server["helo"])!="250")
				{
					continue;
				}

				// enable TLS tranport
				if($server["tls"])
				{
					if($this->startTLS())
					{
						// hello again
						if ($this->cmd("HELO ".$server["helo"])!="250")
						{
							continue;
						}
					}
					else
						continue;
				}
			
				if(!$this->cmdAuth($server["user"],$server["password"]))
					continue;
			}
			else 
			{
				if ($this->doesResContains('ESMTP'))
					$cmd = 'EHLO';
					//$cmd = 'HELO';
				else
					$cmd = 'HELO';
					
				if ($this->cmd("$cmd ".$server["helo"])!="250")
					continue;

				if($server["tls"])
				{
					// cleanup EHLO output
					if($cmd == 'EHLO') do{ $res = $this->getCode(); }	while($res==="250");

					if($this->startTLS())
					{
						// hello again
						if ($this->cmd("$cmd ".$server["helo"])!="250")
							continue;
					}
					else
						continue;
				}
			}

			// cleanup EHLO output
			if($cmd == 'EHLO') do{ $res = $this->getCode(); }	while($res==="250");

			// Envelope from and to
            if(!preg_match('/[<>]/',$this->from))
                $from = '<'.$this->from.'>';
            else
                $from = $this->from;
                
			if ($this->cmd("MAIL FROM:".$from)!="250") continue;
			
			$dest = array_unique(
				explode(',',
					trim(
						preg_replace('/[,;|]+/', ',',"{$to},{$this->cc},{$this->bcc}"),
						',')));

			$ok=true;
			foreach ($dest as $ad)
			{
				if ($this->cmd("RCPT TO:<{$ad}>")!="250") 
				{
					$ok=false;
					break;
				}
			}
			if (!$ok)
				continue;
				
			// message
			if ($this->cmd("DATA")!="354") continue;
			
			// Send To:, From:, Subject:, other headers, blank line, message, and finish
			$this->cmd("To: $to",false);
			$this->cmd("From: <$this->from>",false);
			$this->cmd("Reply-to: <$this->replyto>",false);
			$this->cmd("Return-path: <$this->from>",false);

			global $NX_CHARSET;
			if ($NX_CHARSET=='utf-8')
				$subject = "=?UTF-8?B?" . base64_encode($subject) . "?=";
			$this->cmd("Subject: $subject",false);
			// $this->cmd("X-Mailer: NxMail",false);
			
			$this->cmd("X-Priority: 1",false);
			$this->cmd("X-MSMail-Priority: High",false);
//			$this->cmd("X-Mailer: Microsoft Office Outlook, Build 11.0.5510",false);
			global $NX_DOMAIN;
			if (isset($NX_DOMAIN))
			{
				$website=$NX_DOMAIN;
				$this->cmd("X-Sender: {$website}",false);
				$this->cmd("X-AntiAbuse: This is a solicited email for - {$website} Online mailing list.",false);
			}
			$this->cmd("X-AntiAbuse: Servername - {$_SERVER['SERVER_NAME']}",false);
			$this->cmd("X-AntiAbuse: User - {$this->from}",false);
					
			$msg = $this->formatBody($msg);
			$this->cmd($msg,false);
			if ($this->cmd(".")!="250") continue;
			
			// Say bye bye
			if ($this->cmd("QUIT")!="221") continue;
			
			// finished, it worked...
			$this->closeCon();
			nxLog("Email sent with success:\n".$this->log,"Nx MIME MAIL");
			return true;				
		}

		// didn't work...
		$this->cmd("QUIT");
		$this->closeCon();
		// $this->errstr = "can't contact any SMTP server : " . $this->resCode;	
		$this->errstr = "can't contact any SMTP server";	
		nxError($this->errstr,"mail");
		nxLog("can't send email:\n".$this->log,"Nx MIME MAIL");
		return false;
	}
 
	function formatBody($msg)
	{
		// replace all LF by CRLF
		return str_replace("\n","\r\n",str_replace("\r\n","\n",$msg));
	}
	
	// open a connection to a server (close it first if already opened)
	function & openCon($ip,$port)
	{
		if (isset($this->con) && is_resource($this->con))
		{
			if ($this->log !== null)
				$this->log .= "C:closing connexion";	
			fclose($this->con);
		}
			
		if ($this->log !== null)
			$this->log .= "C:opening connexion to $ip\n";	
			
		$this->con = @fsockopen($ip,$port,$errno, $errstr, 60);
		if ($this->con)
		{
			if (function_exists("stream_set_timeout"))
				@stream_set_timeout($this->con,60,0);
		}
		else
			nxError("error sending email: log is\n".$this->log,'NX_MAIL');
			
		return $this->con;
	}
	
	// close connection if opened
	function closeCon()
	{
		if (isset($this->con) && is_resource($this->con))
		{
			if ($this->log !== null)
				$this->log .= "C:closing connexion";	
			fclose ($this->con);
		}
	}

	// send command to smtp server (check connexion first) and get reply
	// log exchange if log opened
	function cmd($c,$reply=true)
	{
		// checking connection
		if (!is_resource($this->con))
		{
			if ($this->log !== null)
				$this->log .= "C:can't execute command: no connexion\n";	
			
			return false;
		}
		
		// send cmd
		fputs($this->con,"$c\r\n");
		if ($this->log !== null)
			$this->log .= 'C:'.str_replace(array('<','>'),array('&lt;','&gt;'),$c)."\r\n";	

		// get reply
		if ($reply)
		{
			$res=fgets($this->con,256);
			if ($this->log !== null)
				$this->log .= "S:$res\n";
				
			if ($res)
				return $this->resCode=substr($res,0,3);
			else
				return null;
		}
		
		return null;
	}

	function getCode()
	{
		// get reply
		$this->res=fgets($this->con,256);
		
		if ($this->log !== null)
			$this->log .= "S:{$this->res}";

		if ($this->res)
			return substr($this->res,0,3);
		else
			return null;
	}
	
	function doesResContains($str)
	{
		if (strstr($this->res,$str)===FALSE)
			return false;
			
		return true;
	}

	function cmdAuth($u,$p)
	{
		if (!is_resource($this->con))
		{
			if ($this->log !== null)
				$this->log .= "C:can't execute command: no connexion\n";	
			
			return false;
		}
		
		$res = $this->cmd('AUTH LOGIN');
		
		// cleanup EHLO output
		while($res==="250")
			$res = $this->getCode();
			
		// try first auth login
		if(	$res==="334"
			&& $this->cmd(base64_encode($u))==="334"
			&& $this->cmd(base64_encode($p))==="235")
		{
			$this->authenticated = true;
			return true;
		}
		// then try auth plain
		else if ($this->cmd("AUTH PLAIN ".base64_encode($u."\0". $u."\0".$p))==="235")
		{
			$this->authenticated = true;
			if ($this->log !== null)
				$this->log .= 'C: authentication succeed\n';
			return true;
		}
		else
		{
			if ($this->log !== null)
				$this->log .= 'C:AUTH command failed: ' . $this->resCode."\n";
			return false;
		}
	}

	// reimplemented function under windows
	function getmxrr($domain, &$mx,&$weight)
	{
		// on any system but windows, use builtin function
		if (substr(php_uname(), 0, 7) != "Windows")
		{
			return getmxrr($domain, $mx,$weight);
		}

		// windows: emulate the function (NB. only works if DNS entries are properly configure)
		$command = "nslookup -type=mx ".$domain;
		exec ( $command, $result );
		
		$i = 0;
		while ( list ( $key, $value ) = each ( $result ) ) {
		   if ( strstr ( $value, "mail exchanger" ) ) { $nslookup[$i] = $value; $i++; }
		}
		
		if ($i == 0)		
			return false;
			
		while ( list ( $key, $value ) = each ( $nslookup ) ) 
		{
		   $temp = explode ( " ", $value );
		   $mx[$key] = gethostbyname ( $temp[7] );
		   $weight[$key] = $temp[3];
		}
		
		return true;
 	}

} // Class End
?>
<?php

/** message class */
class Imap_Message
{
	var $id;
	var $subject;
	var $text;
	var $attachments=array();
	var $hasAttachments=false;

	function deleteAttachments() 
	{
		foreach ($this->attachments as $i=>$attachments)
		{
			$attachments->delete();
		}

		$this->attachments=array();
	}

	function hasAttachments() {
		return count($this->attachments)>0?1:0;
	}
}

/** email attachment class */
class Imap_MessageAttachment {
	var $path=null;
	var $name=null;
	var $type;
	var $mime;
	var $msg;

	function __construct($msg) {
		$this->msg = $msg;
		$this->mid = $this->msg->uid;
	}

	function init($name,$content,$partno) {
		if ($partno > 0)
		{
			$info = pathinfo($name);
			$name = $info['filename'] .'__'.$partno.'.'.$info['extension'];
		}

		$this->name = $name;
		$this->content = $content;
		$this->partno = $partno;
	}

	function store($fs) {
		if ($fs!=null)
		{
			$this->path = $fs->storeFile($this->msg,$this->name,$this->content,$this->mid);
			$this->fs = $fs;
			$this->content = null;
		}
	}

	function delete() {
		if ($this->path)
			$this->fs->deleteFile($this->path);
	}

}

/** class for storing attachments. Could be replaced by a class storing documents in
a document management tool such as Alfresco or other. */
class Imap_FileStore {

	function __construct($config) {
		$this->basePath = $config->getProperty('store.path');
	}
	
	function storeFile($msg,$name,$content)
	{
		if (!$this->basePath) {
			return null;
		}

		$path = $this->basePath.$name;
		$fp = fopen($path,'w+'); 
		fwrite($fp,$content); 
		fclose($fp);

		return $path;
	}

	function storeMessage($msg)
	{
		$this->storeFile($msg,$msg->uid.'.msg.text',$msg->text);
	}

	function deleteFile($path) {
	}
}

/** class for filtering messages to be retreived from the inbox.
for example: could remove unwanted spam or only keep messages including "CV" in the body ou subject, etc. 
*/
class Imap_MessageFilter {

	function Imap_MessageFilter($config) {
		$this->config = $config;
	}

	function checkHeader($msg,$header)
	{
		$subjectRE = $this->config->getProperty('filter.subject.RE');
		$bodyRE = $this->config->getProperty('filter.body.RE');

		$subject = utf8_decode($msg->subject);

		if (
			(
				preg_match('/'.$subjectRE.'/i',$subject) ||
				preg_match('/'.$bodyRE.'/i',$msg->text)
			) == false)
			return false;

		return true;
	}
	
	function checkSubject($subject)
	{
		return true;
	}
	function checkBody($body)
	{
		return true;
	}
	function checkMessage($msg)
	{
		return true;
	}
	function checkPart($part)
	{
		return true;
	}
}

/** 
	main class for reading emails.
	It is based on the imap PHP extension and can interface imap or pop3 servers.
*/
class Imap_ReadMails 
{
	var $config;
	var $con;
	var $expunge = false;

	function Imap_ReadMails($config,$config_base='cv_inbox.',$fileStore=null,$filter=null,$readAction=null)
	{
		$this->config = $config;
		$this->config_base = $config_base;

		if ($fileStore==null)
			$this->fileStore = new Imap_FileStore($this);
		else
			$this->fileStore = $fileStore;

		if ($filter==null)
			$this->filter = new Imap_MessageFilter($this);
		else
			$this->filter = $filter;

		$this->readAction = $readAction;
	}

	function __destruct(){
		NxCV_SYS::trace("CLOSING IMAP INBOX<br/>");
		$this->close();
	}

	function getProperty($k,$dft='') {
		if (isset($this->config[$this->config_base.$k]))
			return $this->config[$this->config_base.$k];

		return $dft;
	}

	function connect($inboxName='inbox') {
		$inbox = $this->getProperty($inboxName);
		$user = $this->getProperty('user');
		$pwd = $this->getProperty('password');

		$this->con = imap_open($inbox,$user,$pwd); 
		//$list = imap_getmailboxes($this->con, $inbox, "*");
		if (!$this->con)
			nxError("can't connect to inbox : ".imap_last_error(),'IMAP');

		NxCV_SYS::trace("Connected to IMAP inbox $inbox<br/>");
		return $this->con;
	}

	function close() {
		if (!$this->con)
			return false;

		// if deleted messages, need expunge
		if($this->expunge)
		{
			nxLog("Expunged connection",'IMAP');
			imap_expunge($this->con);
			$this->expunge = false;
		}

		nxLog("Closed connection",'IMAP');
		imap_close($this->con);
		$this->con = null;
		NxCV_SYS::trace("Closed IMAP connection<br/>");
	}

	function getMessagesByDate($date,$withParts=false,$withAttachments=false,$fromIndex=0,$charset='utf-8') 
	{
		// search criteria
		$search='ON "' . date('Y-m-d',strtotime($date)) . '"';

		$flags = $this->getProperty('search.flags','');
		if ($flags)
		{
			$search = $flags.' '.$search;
		}

		return $this->getMessagesByCriteria($search,$withParts,$withAttachments,$fromIndex,$charset,$date);
	}

	function getMessages($withParts=false,$withAttachments=false,$fromIndex=0,$charset='utf-8') 
	{
		// search criteria
		$fromTime=$this->getProperty('search.since','today - 10 days');
		$dateFrom = date('d-M-Y',strtotime($fromTime));
		$search='SINCE "' . $dateFrom . '"';

		$flags = $this->getProperty('search.flags','');
		if ($flags)
		{
			$search = $flags.' '.$search;
		}

		return $this->getMessagesByCriteria($search,$withParts,$withAttachments,$fromIndex,$charset);
	}

	function getMessagesByCriteria($search,$withParts=false,$withAttachments=false,$fromIndex=0,$charset='utf-8',$exactdate=null) 
	{
		if (!$this->con)
			return 0;
		
		$messages = array();
		$this->expunge = false;

		// max messages to be processed
		$maxMessages=$this->getProperty('max_messages',50);

		// get messages
		$aIndexes=imap_search($this->con, $search);
		if($aIndexes)
			$nbmess = count($aIndexes);	
		else
			$nbmess = 0;	

		NxCV_SYS::trace("Getting $nbmess messages from IMAP inbox<br/>");
		
		// imap_headers($this->con);
		for ($index=$nbmess-1-$fromIndex;$index>=0;$index--)
		{ 
			// break if too many messages
			if (count($messages)>=$maxMessages)
				break;

			$id = $aIndexes[$index]; 
			$msg = $this->getMessage($id,$withParts,$withAttachments);
			if ($msg)
			{
				if (!$exactdate || $msg->datetime == trim($exactdate))
				{
					$messages[] = $msg;
					$id=$msg->id;
					$this->onReadSuccess($id,$msg);
				}
			}
		} 

		return $messages;
	}

	function onReadSuccess($id,$msg) {
		$actions = $this->getProperty('actions.onread','');
		$ar = explode(',',$actions);
		foreach($ar as $action)
		{
			switch(trim($action)) 
			{
				case 'move':
					$otherInbox = $this->getProperty('action.move.inbox');
					if ($otherInbox)
						$this->moveMessage($id,$otherInbox);

					break;
				case 'delete':
					$this->deleteMessage($id);
					break;
				case 'flag':
					$flag = $this->getProperty('action.flag','\\FLAGGED');
					$this->setMessageFlag($id,$flag);
					break;
				case 'seen':
					$this->setMessageFlag($id,$flag="\\SEEN");
					break;
				case 'store':
					$this->fileStore->storeMessage($msg);
					break;
				case 'custom':
					if($this->readAction)
						$this->readAction->onReadSuccess($this,$id,$msg);
					else
						nxError('IMAP: custom read action is not defined');
					break;
			}
		}
	}

	function getMessageByID($uid,$withParts=true,$withAttachments=false,$charset='utf-8') 
	{
		if (!$this->con)
			return 0;

		$index = imap_msgno($this->con,$uid);	
		$msg = $this->getMessage($index,$withParts,$withAttachments,$charset);

		$id=$msg->id;
		if ($id)
		{
			$this->onReadSuccess($id,$msg);
			return $msg;
		}
		return false;
	}

	function deleteMessage($id) 
	{
		if (!$this->con)
			return 0;

		// $index = imap_msgno($this->con,$uid);
		imap_delete($mbox, $id,FT_UID);
		$this->expunge = true;
	}

	function moveMessage($id,$otherInbox) 
	{
		if (!$this->con)
		{
			nxError('Lost IMAP connection : cant move IMAP message to :'.$otherInbox);
			return 0;
		}

		$res = imap_mail_move($this->con, $id, $otherInbox,CP_UID);
		if(!$res) {
			nxError('cant move IMAP message :'.imap_last_error());
			return 0;
		}

		nxLog("Moved IMAP message $id to :$otherInbox",'IMAP');
		$this->expunge = true;
	}

	function setMessageFlag($uid,$flag="\\SEEN") {
		imap_setflag_full($this->con,$uid,$flag,ST_UID);
		// imap_setflag_full($this->con,$uid,$flag,SE_UID);
	}

	function getMessage($index,$withParts,$withAttachments,$charset='utf-8') 
	{
		if (!$this->con)
			return 0;

		$message = new Imap_Message();

		// get header
		$header = imap_headerinfo($this->con,$index); 
		if(!header)
			return null;

		$message->header = $header;

		// decoded subject
		$message->subject = imap_utf8($header->subject);

		if (!$this->filter->checkSubject($message->subject))
			return null;

		// from
		$from = $header->from;
		foreach ($from as $id => $object) {
			$message->fromName = imap_utf8($object->personal);
			$message->from = $object->mailbox . "@" . $object->host;
		}

		// reply to
		$from = $header->reply_to;
		foreach ($from as $id => $object) {
			$message->replyToName = imap_utf8($object->personal);
			$message->replyTo = $object->mailbox . "@" . $object->host;
		}

		// date
		$message->datetime = date('Y-m-d H:i:s',strtotime($header->date));
		$message->id = imap_uid($this->con,$index);
		$message->uid = $header->message_id;

		if (!$this->filter->checkHeader($message,$header))
			return null;

		// get structure
		if ($withParts)
		{
			$struct = imap_fetchstructure($this->con,$index);
			if (!$struct->parts)  
			{
				// not multipart
				$this->getPart($this->con,$index,$struct,0,$message,$withAttachments);  // no part-number, so pass 0
			}
			else 
			{  
				// multipart: iterate through each part
				foreach ($struct->parts as $pIdx=>$part)
				{
					$this->getPart($this->con,$index,$part,$pIdx+1,$message,$withAttachments,$charset);
				}
			}
		}

		if (!$this->filter->checkMessage($message))
		{
			return null;
		}

		// store attachements
		if ($withAttachments)
			foreach ($message->attachments as $attachment)
			{
				$attachment->store($this->fileStore);
			}

		return $message;
	}


	function getPart($mbox,$mid,$p,$partno,$message,$withAttachments,$charset) 
	{
		// $partno = '1', '2', '2.1', '2.1.3', etc if multipart, 0 if not multipart

		// DECODE DATA
		$data = ($partno)?
			imap_fetchbody($mbox,$mid,$partno):  // multipart
			imap_body($mbox,$mid);  // not multipart

		// Any part may be encoded, even plain text messages, so check everything.
		if ($p->encoding==4)
			$data = quoted_printable_decode($data);
		elseif ($p->encoding==3)
			$data = base64_decode($data);
		// no need to decode 7-bit, 8-bit, or binary

		// PARAMETERS
		// get all parameters, like charset, filenames of attachments, etc.
		$params = array();
		if ($p->parameters)
			foreach ($p->parameters as $x)
				$params[ strtolower( $x->attribute ) ] = $x->value;
		if ($p->dparameters)
			foreach ($p->dparameters as $x)
				$params[ strtolower( $x->attribute ) ] = $x->value;

		$attachment = new Imap_MessageAttachment($message);

		// ATTACHMENT
		// Any part with a filename is an attachment,
		// so an attached text file (type 0) is not mistaken as the message.
		if ($params['filename'] || $params['name']) 
		{
			if ($withAttachments)
			{
				// filename may be given as 'Filename' or 'Name' or both
				$filename = ($params['filename'])? $params['filename'] : $params['name'];

				// filename may be encoded, so see imap_mime_header_decode()
				$filename = imap_utf8($filename);
				// $filename = 'message_'.$message->id . '_' . $filename;
				
				$attachment->init($filename,$data,$partno);
			}
			$message->hasAttachments=true;
		}

		// TEXT
		elseif ($p->type==0 && $data) 
		{
			// Messages may be split in different parts because of inline attachments,
			// so append parts together with blank row.

			// $attachment->charset = $params['charset'];  // assume all parts are same charset
			if(!isset($params['charset']))
			{
				if(strtolower($charset)=='utf-8')
					$data=utf8_encode($data);
			}
			elseif($params['charset'] != $charset)
			{
				if (function_exists(iconv))
					$data=iconv($params['charset'],$charset,$data);
				else if(strtolower($charset)=='utf-8')
					$data=utf8_encode($data);
			}

			if (strtolower($p->subtype)=='plain')
				$message->text .= trim($data) ."\n\n";
			else
				$message->html .= $data ."<br><br>";
		}

		// EMBEDDED MESSAGE
		// Many bounce notifications embed the original message as type 2,
		// but AOL uses type 1 (multipart), which is not handled here.
		// There are no PHP functions to parse embedded messages,
		// so this just appends the raw source to the main message.
		elseif ($p->type==2 && $data) {
			$attachment->text .= trim($data) ."\n\n";
		}

		// SUBPART RECURSION
		if (isset($p->parts)) {
			foreach ($p->parts as $partno0=>$p2)
				$this->getPart($mbox,$mid,$p2,$partno.'.'.($partno0+1),$message,$withAttachments,$charset);  // 1.2, 1.2.1, etc.
		}

		if ($attachment->name && $this->filter->checkPart($attachment))
		{
			$message->attachments[] = $attachment;
			return $attachment;
		}
		else
			return null;
	}
}

?>
<?php

class Messenger
{
	static function inbox($user)
	{
		return messagePeer::getByUser($user->oid);
	}
	static function send($message)
	{
		$message->store();

		$mgr= new MessengerEmail();
		//$mgr->send($message);
	}
}

class MessengerEmail
{
	function send($message,$dsID='register_email')
	{
		include_once(NX_LIB.'ds/email/NxMail_MIME.inc');
		$ctxt = Framework::getContext();
		$ctxt->loadResource('ds/'.$dsID.NX_HOST_SWITCH,$ctxt->error,true);

		if (!($subject=$message->subject->string))
			$subject = $ctxt->getProperty($dsID.'.subject');

		if (!($to=$message->to->string))
			$to = $ctxt->getProperty($dsID.'.to');
				
		if (!($from=$message->from->string))
			$from = $ctxt->getProperty($dsID.'.from');

		if (!($cc=$message->cc->string))
			$cc = $ctxt->getProperty($dsID.'.cc');

		if (!($bcc=$message->bcc->string))
			$bcc = $ctxt->getProperty($dsID.'.bcc');

//		if (!($smtp=$message->smtp->string))
			$smtp = $ctxt->getProperty($dsID.'.smtp.servers');

//		if (!($headers=$message->headers->string))
			$headers = $ctxt->getProperty($dsID.'.smtp.headers');

//		if (!($log=$message->log->string))
			$log = $ctxt->getProperty($dsID.'.log');

		$text = $message->body->string;
			
		// send email
		$mime = new NxMail_MIME($from, $to, $cc, $bcc,$subject, $text, $headers,$smtp,$log);
			
		$html = $message->html->html;
		if ($html)
		{
			$mime->attach($html, 'HTML message', HTML, BASE64);
		}

		if (!$mime->send_mail()) {
			nxLog('Failed: LOG:<pre>'.$mime->getLog('html').'</pre>','EMAIL');
		}
	}
}

class message extends PersistentObject
{
	function store($ds,$op='insert',$media='db',$params=null)
	{
		if (!$this->isOk())
			return false;

		if (!$ds) {
			$ds='message';
		}

		if (!$params) {
			$params= array('oid'=>$this->oid->object);
		}
		
		return parent::store($ds,$op,$media,$params);
	}

	function __toString() {
		if (!$this->isOk())
			return 'void message';

		$s="<strong>{$this->subject->string}</strong><br/><strong>from: </strong>{$this->from->string} <strong>at: </strong>{$this->time->string}";
		return $s;
	}

	function setHTML($view,$props) 
	{
		$ctxt = Framework::getContext();
		$html=$ctxt->runView($view, $props, 'email.html');

		$this->html = preg_replace(
			array('/[ \t]+/','/[\']/'),
			array(' ','\\\''), $html);
	}

}

class messagePeer
{
	// auto factory
	static function getByOid($oid,$ds='message') {
		$o = new message();
		$o->load($ds,'db', array('oid'=>$oid));
		return $o;
	}
	static function getByUser($oid,$ds='message.by_user') {
		$col = new PersistentCollection();
		$col->load($ds,'db', array(
			'userOid'=>$oid
			));
		return $col->message;
	}
	static function getFromRequest($ds='message') {
		$o = new message();
		$o->load($ds,'request');
		return $o;
	}
	static function getEmpty($ds='message') {
		$o = new message();
		$o->load($ds,'empty');
		return $o;
	}
	static function deleteByOid($oid,$ds='message') {
		$o = new message();
		$o->load($ds,'db', array('oid'=>$oid));
		$o->delete($ds,'db',array('oid'=>$oid));
		return $o;
	}
	static function getByDate($start_date,$end_date,$userOid=null,$state=null,$ds='message.by_date') 
	{
		$col = new PersistentCollection();
		$col->load($ds,'db', array(
			'start_date'=>$start_date,
			'end_date'=>$end_date,
			'userOid'=>$userOid,
			'state'=>$state
			));
		return $col->message;
	}
}

class portal_user extends PersistentObject
{
	function sendMessage($message) 
	{
	}
}

class rh_user extends portal_user
{
	var $ds_user = 'rh.user.conge';

	function __toString() {
		if ($this->isOk())
			return $this->first_name->string.' '.$this->last_name->string;
	}

	function displayCounters() {
		$lastYear = 0+($this->current_year->year->object) -1;
		return "Credit {$this->current_year}: {$this->current_credit} + Credit year {$lastYear}: {$this->last_credit} = {$this->reliquat}";
	}

	function store($ds,$op='insert',$media='db',$params=null)
	{
		if (!$this->isOk())
			return false;

		if (!$ds) {
			$ds=$this->ds_user;
		}
		if (!$params) {
			$params= array('oid'=>$this->oid->object);
		}
		
		return parent::store($ds,$op,$media,$params);
	}
}

class rh_userPeer
{
	static $users = array();

	// auto factory
	static function getByOid($oid,$ds='rh.user.by_oid@rh',$cache=true) {
		if ($cache && isset(self::$users[$oid]))
			return self::$users[$oid];

		$o = new rh_user();
		$o->load($ds,'db', array('oid'=>$oid));

		if ($cache && $o->isOk()) 
			self::$users[$oid] = $o;

		return $o;
	}

	// oids : a CSV list of users
	static function getByOidList($oids,$format='rh_user',$ds='rh.user.by_oids') {
		$col = new PersistentCollection($ds,'db',array('user_oids'=>$oids));
		return $col->$format;
	}

	// oids : a CSV list of groups
	static function getByGroups($oids,$format='rh_user',$ds='rh.user.by_group') {
		$col = new PersistentCollection($ds,'db',array('groups_oids'=>$oids));
		return $col->$format;
	}

	// get all users
	static function getAll($format='rh_user',$ds='rh.user.all',$params=null) {
		$col = new PersistentCollection($ds,'handler',$params);
		return $col->$format;
	}
}

class rh_group extends PersistentObject
{
	function getParentOid() {
		$oid=$this->mgc_parent_group_RH->object;
		if ($oid=='?')
			return null;
		else
			return $oid;
	}

	function getUsers() {
		return $this->mgc_parent_group_RH->object;
	}

	function isManager($uoid) {
		$oids=$this->hr_manager->object;
		if (!$oids)
			return false;

		$mgrs = explode('|',$oids);
		return in_array($uoid, $mgrs);
	}

	function __toString() {
		if ($this->isOk())
			return $this->group_name->string;

		return 'unknow group';
	}
}

class rh_groupPeer
{
	static function getByOid($oid,$ds='rh.group.by_oid') {
		$o = new rh_group();
		$o->load($ds,'db', array('oid'=>$oid));
		return $o;
	}

	static function getByPermission($perm,$ds='rh.group.perms') {
		$o = new rh_group();
		$o->load($ds,'db', array('perm'=>$perm));
		return $o;
	}

	static function getAll($format='rh_group',$ds='rh.group',$params=null) {
		$col = new PersistentCollection($ds,'handler',$params);
		return $col->$format;
	}

	static function getByManager($managerOid,$ds='rh.group.by_manager') {
		$col = new PersistentCollection($ds,'db',array('managerOid'=>$managerOid));
		return $col->rh_group;
	}
}

?>
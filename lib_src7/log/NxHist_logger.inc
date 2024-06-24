<?php

/*
	class: NxHist_logger
	description: 
		Class for loging events in CMS history.
		Events are registered by site, user, article, app, etc.
*/

class NxHist_logger {
	var $userID;
	var $db;
	var $error;
	
	function __construct() {}

	// generic log function
	function log(&$props,$category,$action,$comment,$uid,$soid,$aid=0,$said=0,$paid=0) {
		$error = new NxErrorCB();
		$db = &$props->getDB($error);
		/*if ($comment!='')
			$comment=preg_nl2br(htmlspecialchars($comment,ENT_QUOTES)); */
		
		$query="insert into _nx_history(_soid, _user_id, _category, _action, _comment, _timestamp,_aid,_said,_paid) values($soid,'$uid','$category','$action','$comment', NOW(),'$aid','$said','$paid')";
		if ($db->query($query))
			return true;
		
		return false;
	}
	
	// log events related to a site
	function log_site(&$props,$action,$comment) {
		$site=$props->getProperty('site.id','');
		if($site!='')
			$site="site=$site";
			
		return NxHist_logger::log(
			$props,'?-site',$action,$site.$comment,
			$props->getProperty('user.userID',0),
			$props->getProperty('site.oid',0));
	}
	
	// log events related to a user
	function log_user(&$props,$action,$comment) {
		return NxHist_logger::log(
			$props,'?-user',$action,$comment,
			$props->getProperty('user.userID',0),
			$props->getProperty('site.oid',0));
	}

	// log events on a page (aid=article id, said=section id, paid=part id) 
	function log_page(&$props,$action,$comment,$aid,$said=0,$paid=0) {
		$site=$props->getProperty('site.id','');
		if($site!='')
			$site="site=$site";
		return NxHist_logger::log(
			$props,'?-page',$action,$site.$comment,
			$props->getProperty('user.userID',0),
			$props->getProperty('site.oid',0),
			$aid,$said,$paid);
	}
	
	// log events related to an application
	function log_app(&$props,$action,$comment,$app,$resoid=0) {
		$site=$props->getProperty('site.id','');
		if($site!='')
			$site="site=$site";
		$app=strtolower($app);		
		return NxHist_logger::log(
			$props,"?-app-$app",$action,$site.$comment,
			$props->getProperty('user.userID',0),
			$props->getProperty('site.oid',0),
			0,0,$resoid);
	}
}

?>
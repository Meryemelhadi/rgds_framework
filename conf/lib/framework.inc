<?php

include_once(NX_XLIB.'lib/orm/persistence.inc');

class Framework {

	static $context=null;

	static function getContext() {
		return self::$context ? self::$context : $GLOBALS['current_page'];
	}

	static function setContext($context) {
		Framework::$context = $context;
	}

	static function getUser() {
		if (isset($GLOBALS['_NX_user']))
			return $GLOBALS['_NX_user'];
		return null;
	}

	static function getSite() {
		if (isset($GLOBALS['_NX_site']))
			return $GLOBALS['_NX_site'];
		return null;
	}

}

?>
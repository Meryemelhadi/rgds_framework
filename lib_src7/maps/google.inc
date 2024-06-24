<?php

class maps_google
{
	var $props=null;
	
	function maps_google(&$props) {
		$this->props=$props;
	}
	
	function getKey($folder,$key) {
		$key=$this->props->getProperty("google.map.key",'');
		return true;
	}

}

?>
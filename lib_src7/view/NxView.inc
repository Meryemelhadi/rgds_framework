<?php
// for test only, not used in actual code
class NxView
{
	function __construct($out,$db,$widgets)
	{
		$this->out=$out;
		$this->db=$db;
		$this->widgets=$widgets;
	}
	
	function &getWidget($name)
	{
		if (isset($this->widgets[$name]))
			return $this->widgets[$name];
		else
			return null;
	}
	
	function getDB()
	{
		return $this->db;
	}

	function getOutput()
	{
		return $this->out;
	}
}

?>
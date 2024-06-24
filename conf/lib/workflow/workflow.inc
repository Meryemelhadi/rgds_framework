<?php

class WorkflowRequest extends PersistentObject
{
	function setState($state)
	{
		$this->setFieldValue('demand_state',$state,'object');
	}
	function getState()
	{
		return $this->getFieldValue('demand_state','object');
	}
	function isRequestValid()
	{
		return true;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : html code */
class NxDataField_expression extends NxDataField_text
{
	function __construct(&$desc)
	{
		parent::__construct($desc);
	}
	
	function & toObject($opts=null)
	{
		return $this->toHTML();
	}

	function toString($opts=null) {
		// return html_entity_decode($this->toHTML($opts,''));
		return preg_replace("/(\n|\r)/",' ',html_entity_decode($this->toHTML($opts,''),ENT_QUOTES));
	}

	function toHTML($opts=null,$default='&nbsp;')
	{
		$this->value = $this->desc->getProperty("value");
		$record = $this->record;
		$value='';

		if ($this->value != '')
		{
			$s=preg_replace(
				array(
					'/%?\{field\.([^:]+):([^}]+)\}/',
					'/%?\{field:([^}]+)\}/',
				),
				array(
					'$record->$2->$1',
					'$record->$1->html',
				),
				$this->value);

			preg_replace_callback('/^.*$/e', function($m) use(&$value) { return $value=$m[0]; }, $s);

		}
		else
			$value='';

		if ($format = $this->desc->getProperty('format',null))
		{
			$format = preg_replaceX(
				array(
					'/%?\{field\.([^:]+):([^}]+)\}/e',
					'/%?\{field:([^}]+)\}/e',
				),
				array(
					'$this->getFieldVal($this->record,"$2",$1)',
					'$this->getFieldVal($this->record,"$1")',
				),
				$format,$this);

			$value = sprintf($format,$value);
		}

		$c = $this->desc->getProperty("control_html",null,false);
		if ($c) {
			$control = $this->desc->getControl($c);
			return $control->toHTML($value,$this);
		}
		else
			return $value;
	}

	function getFieldVal($rec,$fname,$meth='html') {
		return $rec->$fname->$meth;
	}
}

?>
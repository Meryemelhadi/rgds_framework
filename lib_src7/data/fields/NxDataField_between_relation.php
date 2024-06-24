<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : choice list */
class NxDataField_between_relation extends NxData_Field
{
	var $value;

	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$this->desc->sep=$desc->getProperty('csv_sep','|',false);		
	}

	function clear()
	{
		$this->value = '¦';
	}
		
	// process default value and returns it
	function getDefaultValue()
	{
		$qv = $this->getProperty('value',null,false);
		if ($qv === null)
			$qv = $this->getProperty('default',null,false);
		
		if ($qv === null)
			return null;

		if (is_array($qv))
			$qv=implode('¦',$qv);
		
		return $this->desc->getQValue($qv);
	}
	
	function setValue($deft)
	{
		if ($deft === null)
		{
			$this->value = '¦';
			return false;
		}

		if (is_array($deft))
			$this->value = implode('¦',$deft);
		else
			$this->value = (string)$deft;
			
		return true;
	}
	
	function toUrlParam()		{
		// TODO: check if this should be a raw string instead of HTML...
		return $this->getAlias() . "=" . rawurlencode($this->value);
	}

	function & toObject($opts=null)
	{ 
		return $this->value; 
	}
	
	function readFromStore(&$v){
		$this->value = $v;
		if ($this->value == null)
		{
			$this->value = '¦';
		}
	}
	
	function readFromDB($recData)
	{
		$this->value = $this->getFieldData($recData,$this->getFName(),'¦');
	}

	function toDB($opts=null)
	{	
		if ($opts!=null)
		{
			list($r,$v)=explode('¦',$this->value);		
			if ($opts=='as-relation')
			{
				return $r;
			}
			elseif ($opts=='as-date')
			{		
				// used as a param from sql "where" clause.
				//  NB. "where clause" manages the actual relation operator...
				if (($t=strtotime($v,time()))!==-1)
					return "'".date("Y-m-d",$t)."'";
					
				return '';
			}
			elseif ($opts=='as-value')
			{
				return "'$v'";
			}	
		}
				
		return "'".htmlspecialchars($this->value,ENT_QUOTES)."'";
	}

	function toStringRelation($rel)
	{
		$listName = $this->getProperty("list_relation",'field_relation',false);
		$dir='lists';
		$list = new NxChoiceList2($this,$listName,$dir,'','','');
		return $list->getChoice($rel,'');
	}
	
	function toString($opts=null)
	{
		list($rel,$val)=explode('¦',$this->value);
		
		// get relation string
		$str_rel=$this->toStringRelation($rel);
			
		// get main value string	
		$list =& $desc->getList();
		$listProps =& $list->getListAsProperties();
		
		$res = '';
		if (isset($val))
		{
			$vals = explode("|",$val);
			foreach($vals as $v)
				$res .= $listProps->getProperty($v,$v).',';
				
			$res=rtrim($res,',');
		}
		return "$str_rel : $res";
	}
	
	/* apply a format on the choice list. Format is list of the form:
			array('empty'=>'%s:empty','selected'=>'%s:ok') : display all items
		or
			array('first'=>'[%s','last'=>'%s]','others'=>',%s','unique'=>'[%s]') */
	function toFormat($opts='')
	{
		return $this->toHTML($opts);
	}
	
	function toHTML($opts=null)
	{
		list($rel,$val)=explode('¦',$this->value);
		
		// get relation string
		$str_rel=$this->toStringRelation($rel);
			
		$res='';
		if (isset($val))
		{
			$list =& $this->desc->getList();
			
			$vals = explode($this->desc->sep,$val);
			if (count($vals)>1)
			{
				$sep = $this->desc->getProperty("sep",'<br>',false);
				$vals = explode($this->desc->sep,$val);
				foreach($vals as $v)
					$res[]= $list->getChoice($v);
					
				return implode($sep,$res);
			}
			else
				return $list->getChoice($val);
		}
		return $res;
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;

		if (!$desc->isInput())
			return "";

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$this->value."\" />";

		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$this->value."\" />";	
		// split value
		list($rel,$val)=explode('¦',$this->value);
			
		/* get first part of the form control */		
		$desc_relation=new NxFieldDesc(
			array('name'=>"{$desc->name}_relation",
				  'list'=>$desc->getProperty("list_relation",'field_relation',false),
				  'prompt'=>$desc->getProperty("prompt_relation",null,false),
				  'csv_sep'=>'|'
				  ),
			null);
		$desc_relation->sep='|';
		
		$s='';
		$control =& $desc_relation->getControl($desc->getProperty('control_relation','select',false));
		if ($control!==null)
			$s.= $control->toForm($rel,$desc_relation);
		
		// get second part of control
		$control =& $desc->getControl($desc->getProperty("control","select",false));
		return $s.$control->toForm($val,$this);
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		$desc = $this->desc;
		
		/* get first part of the form control */		
		$desc_relation=new NxFieldDesc(
			array('name'=>"{$desc->name}_relation",
				  'list'=>$desc->getProperty("list_relation",'field_relation',false),
				  'prompt'=>$desc->getProperty("prompt_relation",null,false),
				  'isRequired'=>true
				  ),
			null);			
		$desc_relation->sep='|';
		
		$control =& $desc_relation->getControl($desc->getProperty('control_relation','select',false));
		if ($control!==null)
			$rel = $control->readForm($recData,$desc_relation);
		else
			$rel='=';

		// get control class (ie combo, list, radio, tick, etc.)
		$control =& $desc->getControl($desc->getProperty("control","select",false));
		
		// read control value
		$val = $control->readForm($recData,$this);

		if ($val == null)
		{
			$val = TC_NO_SELECTION;
			if ($desc->isRequired())
			{
				$this->addError($errorCB, "not selected");
				return false;
			}
		}
		
		$this->value="{$rel}¦{$val}";
		
		return true;
	}
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : choice list */
class NxDataField_text_choice extends NxData_Field
{
	var $value;

	function __construct($desc)
	{
		$this->desc = $desc;
		$this->desc->sep=$desc->getProperty('csv_sep','|',false);
	}

	function toStatus() {
		$prompt_value=$this->desc->getProperty('prompt_value','?',false);
		
		return isset($this->errorMsg)?'error': 
			((empty($this->value) || $this->value==$prompt_value)?'empty':'ok');
	}
	
	function clear()
	{
		$this->value = TC_NO_SELECTION;
	}
	function isEmpty()
	{
		return !isset($this->value)||!$this->value||($this->value=='?')||($this->value=='?!');
	}

	function contains($choice) {
        if(!isset($choice)|| !($v=trim($this->value,$this->desc->sep)))
            return false;
            
		$choices = explode($this->desc->sep,$v);
		if ($choices!='' && in_array($choice, $choices))
			return true;
		return false;
	}

	function sortValues($func=false) {
		$vals = $this->toArray();
		if(count($vals)>1)
		{
			$vals2 = array();
			$list = $this->toList();
			$klist = array_keys($list);
			foreach($vals as $v)
			{
				$k = array_search($v, $klist);
				if($k!==false)
					$vals2[$k] = $v;
			}

			if($func)
				$vals2 = $func($vals2);
			else
				ksort($vals2);

			$this->setValue($vals2);
		}
		return $this->value;
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
			$qv=implode($this->desc->sep,$qv);
		
		return $this->desc->getQValue($qv);
	}
	
	function setValue($deft)
	{
		if ($deft === null)
		{
			$this->value = TC_NO_SELECTION;
			return false;
		}

		if (is_array($deft))
			$this->value = implode($this->desc->sep,$deft);
		else
			$this->value = (string)$deft;

		// if ($this->record->inMemory)
			$this->desc->keepValue($this->value);
			
		return true;
	}	
	
	function toArray() { 
		$v = trim($this->value,$this->desc->sep);
		if($v==='')
			return array();

		$vals = explode($this->desc->sep,$v);	
		return $vals;
	}

	function toUrlParam()		{
		// TODO: check if this should be a raw string instead of HTML...
		return $this->getAlias() . "=" . rawurlencode($this->value);
	}

	function toObject($opts=null)
	{ 
		$prompt_value=$this->desc->getProperty('prompt_value','?',false);
		return $this->value==$prompt_value?null:$this->value; 
	}
	
	function toList($options='array') { 
		$list = $this->desc->getList();
		$ar = $list->getListAsArray();
		
		$options=strtolower(trim($options));
		if ($options=='json')
		{
			require_once(NX_LIB.'json/json'.NX_PHP_VER.'.inc');
			return json_encode($ar);
		}
		if ($options=='js')
		{
			require_once(NX_LIB.'js/utils.inc');			
			return PHP2Js($ar,'','literal','as-array');
		}
		
		return $ar;
	}
	
	function readFromStore(&$v){
		$this->value = $v;
		if ($this->value == null)
		{
			$this->value = TC_NO_SELECTION;
		}
		elseif ($this->record->inMemory)
			$this->desc->keepValue($this->value);

	}
	
	function readFromDB($recData)
	{
        $qv = $this->desc->getProperty('value',null,false);
        if ($qv !== null)
            $this->value=$this->desc->getQValue($qv);
        else                
		    $this->value = $this->getFieldData($recData,$this->getFName(),TC_NO_SELECTION);

		$this->value = trim($this->value,$this->desc->sep);
		if ($this->record->inMemory)
			$this->desc->keepValue($this->value);
	}

	function toDB($opts=null)
	{	
		if ($opts=='as-date')
		{
			// strtotime strings (today,tomorrow,etc.) used as a param from sql "where" clause
			if (($t=strtotime($this->value,time()))!==-1)
				return "'".date("Y-m-d",$t)."'";
				
			return 'NULL';
		}

		if ($this->value === null)
			return 'NULL';

		if ($this->value && $this->desc->getProperty("multiple",false,false))
		{
			$this->value=$this->desc->sep.trim($this->value,$this->desc->sep).$this->desc->sep;
		}
		
		return "'".htmlspecialchars($this->value,ENT_QUOTES)."'";
	}

	function toString($opts=null,$sep=',')
	{
		if ($opts)
			return $this->toFormat($opts);
	
		if($this->desc->getProperty("value_is_text"))
			return $this->value;

		if (isset($this->value))
			$this->desc->setProperty('keyword',$this->value);

		$list = $this->desc->getList();
		$listProps = $list->getListAsProperties();
		
		$res = '';
		if (isset($this->value))
		{
			$vals = explode($this->desc->sep,$this->value);
			$a = array();
			foreach($vals as $v)
			{
				if ($v!=='' && $v!='?' && $v!='?!')
				{
					$a[] = $listProps->getProperty($v,$v,false);
				}
			}
				
			$res=implode($sep,$a);
		}
		return $res;
	}
	
	/* apply a format on the choice list. Format is list of the form:
			array('empty'=>'%s:empty','selected'=>'%s:ok') : display all items
		or
			array('first'=>'[%s','last'=>'%s]','others'=>',%s','unique'=>'[%s]') */
	function toFormat($opts='',$fmt=null)
	{
		$list =& $this->desc->getList();
		if(is_array($opts))
			$fmt=$this->desc->getFormat($list->name,$opts);
		else
			$fmt=$this->desc->getFormat($opts);
		
		if (isset($this->value)&&$this->value!='')
			$vals=explode($this->desc->sep,$this->value);
		else
			$vals=array();

		$res='';
		$empty_fmt=$fmt->getChoice('empty');
		if (($sel_fmt=$fmt->getChoice('selected'))!=null)
		{
			$listArray =& $list->getListAsArray();		
			$sel=array_flip($vals);
			foreach ($listArray as $k=>$v) 
			{
				if ($k[0]!='!')
					if (isset($sel[$k]))
						$res.=str_replace('%s',$v,$sel_fmt);
					else
						$res.=str_replace('%s',$v,$empty_fmt);
			}
		}
		elseif (count($vals)>0)
		{
			if (($l=count($vals)-1)>0)
			{
				$res=str_replace('%s',$list->getChoice($vals[0]),$fmt->getChoice('first'));
				$others_fmt=$fmt->getChoice('others');
				
				for($i=1;$i<$l;$i++)
					$res.=str_replace('%s',$list->getChoice($vals[$i]),$others_fmt);
				
				$res.=str_replace('%s',$list->getChoice($vals[$l]),$fmt->getChoice('last'));
			}
			else
			{
				return str_replace('%s',$list->getChoice($this->value),$fmt->getChoice('unique'));
			}
		}
		else
			return $empty_fmt;
			
		return $res;
	}
	
	function toHTML($opts=null)
	{
		if(isset($this->value) && trim($this->value)!=='')
		{
			// get control class (ie combo, list, radio, tick, etc.)
			$control = $this->desc->getControl($this->desc->getProperty("control","select",false));
			return $control->toHTML($this,$opts);
		}
		else
			return '';
	}

	function toForm($opts=null) 
	{
		$desc = & $this->desc;
		if (!$desc->isInput())
			return "";

		if ($desc->isHidden() || $opts=='hidden')
			return "<input type=\"hidden\" name=\"".$this->getFormName()."\" value=\"".$this->value."\" />";

		if (!$desc->isEdit() || $opts=='noedit'|| $opts=='no edit')
			return $this->toHTML()."<input type=\"hidden\" name=\"".$this->getFormName()."\" value=\"".$this->value."\" />";			
		/*
		$style = $desc->getStyle("select","choice");
		$list =& $desc->getChoiceList($this->listName);
		return $list->getFormList($desc->name,$this->value,$style,$desc->getProperty("multipleChoices",false,false));
		*/
		$s='';
		if ($form_relation = $desc->getProperty('form_relation',null,false))
		{
			$control =& $desc->getControl($form_relation);
			if ($control)
				$s.= $control->toForm($this->value,$this);
		}

		// get control class (ie combo, list, radio, tick, etc.)
		$ctrlName = $desc->getProperty('control','select',false);

		if(defined('NX_SELECT_REFRESH') && NX_SELECT_REFRESH && $ctrlName=='select' && !$form_relation)
		{
			$ds=$desc->getProperty('ds',null,false);
			if($ds)
				$ctrlName='select_update';
		}

		$control = $desc->getControl($ctrlName);
		return $control->toForm($this->value,$this);
	}

	function _readFromForm(&$recData,&$errorCB)
	{
		// get control class (ie combo, list, radio, tick, etc.)
		$desc = $this->desc;
		$control = $desc->getControl($desc->getProperty("control","select",false));
		
		// read control value
		$this->value = $control->readForm($recData,$this);

		if ($this->value === null)
		{
			$this->value = TC_NO_SELECTION;
			if ($this->desc->isRequired())
			{
				$this->addError($errorCB, "not selected");
				return false;
			}
		}

		if ($this->record->inMemory)
			$this->desc->keepValue($this->value);

		return true;
	}
}

?>
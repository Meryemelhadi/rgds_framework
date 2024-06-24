<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxChoiceLists
{
	var $parent;
	var $lists;

	function __construct($lists, $parent)
	{
		$this->lists = $lists;
		$this->parent = $parent;
	}
	
	// get a list as key=>value
	// todo: remove lists from properties and eave them in content/lists
	function & getList($name,$lang,$dir='lists')
	{
		if(isset($this->lists) && isset($this->lists[$name]))
			return new NxChoiceList($this->parent,$this->lists[$name],$dir);
		else if (isset($this->parent))
			return  $this->parent->getList($name,$lang);
		else
			return new NxChoiceList($this->parent,null,$lang,$dir);
	}
	
	// get format array (display options)
	function & getFormat($name,$lang,$dir='formats')
	{
		if(isset($this->lists) && isset($this->lists[$name]))
			return new NxChoiceList($this->parent,$this->lists[$name],$dir);
		else
			return new NxChoiceList($this->parent,null,$lang,$dir);
	}
}

class NxChoiceList
{
	var $desc;
	var $props;
	
	function &NxChoiceList(&$props,$desc,$lang=null,$dir='lists',$ds='db:ds_category.list:lists')
	{
		global $NX_LANG,$NX_DEFAULT_LANG;
		
		// if default lang, get it from properties
		if ($lang == null)
			$lang = $NX_LANG;
			
		$this->props =& $props;
		
		// fully/static defined list
		if (is_array($desc))
		{
			// check if list is localised
			$first = reset($desc);
		
			if (is_array($first))
			{
				if (isset($desc[$lang]))
					$desc = $desc[$lang];
				elseif (isset($desc[$NX_DEFAULT_LANG]))
					$desc = $desc[$NX_DEFAULT_LANG];
				else 
					$desc = $first;		
			}
		
			$this->desc = $desc;
			/*
			if (!isset($desc["?"]))
			{
				$this->desc = array("?" => "?");
				foreach ($desc as $v => $l)
				{
					$this->desc[$v] = $l;
				}
			}
			else
			{
				$this->desc = $desc;
			}
			*/
			return;
		}
		
		// list defined by either a static file or a database
		if ($this->loadList($desc,$dir) == null)
		{
			// error can't read list => load empty list ?
		}

		// no list => return empty one
		if ($desc == null)
		{
			$this->desc = array("?" => "?");
			return;
		}
	}

	// get a record from DB and sets it to a property
	function loadList($view,$media)
	// function & getRecords($view=null,$propName=null,$media='DB')
	{
		// get data source plugin
		$ds =& $this->props->getPlugIn(strtolower($media),'NxDS_','ds');
		
		if (!$ds)
		{
			// no data source working with this type of media => direct load
			return $this->loadList_old($view,$media);
			// nxError("Can't find datasource: [$media]",'ChoiceList');
			//return null;
		}
		
		// get array
		$error=new NxErrorCB();
		$this->desc = & $ds->getData($view, $error);
			
		if ($this->desc)
			return true;
		else
			return false;
	}	
	
	function & loadList_old($desc,$dir)
	{
		// decode media: 
		// ex. %{db:view:key alias:value alias}
		//     %{xml:file name w/o extension} => caching of data as php
		//     %(php:file name w/o extension}
		// default to: php file name w/o extension

		// get and translate view name
		$viewName = "{$dir}.{$desc}" ;

		// recompose local path
		$path = strtr($viewName,".","/");
		
		// setup local variables for view
		$properties  =& $this->props;
		
		global $NX_LANG,$NX_DEFT_LANG;
		
		// call view
		if (is_file($f=NX_CONTENT.$path.'_'.$NX_LANG.'.inc'))
			$success = include($f); 
		elseif (is_file($f=NX_CONTENT.$path.'_'.$NX_DEFT_LANG.'.inc'))
			$success = include($f); 
		elseif (is_file($f=NX_CONTENT.$path.".inc"))
			$success = include($f); 
		elseif (is_file($f=NX_DIS_CONTENT.$path.'_'.$NX_LANG.'.inc'))
			$success = include($f); 
		elseif (is_file($f=NX_DIS_CONTENT.$path.'_'.$NX_DEFT_LANG.'.inc'))
			$success = include($f); 
		elseif (is_file($f=NX_DIS_CONTENT.$path.".inc"))
			$success = include($f);
		else
			$success=false;

		if ($success === false)
			return false;
			
		$this->desc = $list;
			
		return true;
	}
	
	// deprecated
	function &getFormList($fname,$value,$style="",$isMultiple=false)
	{
		nxError('use of deprecated method at '.__FILE__.':'.__LINE__);
		$a =& $this->desc;
		
		$sel = array();
		if (isset($value))
		{
			$selected_options = explode("|",$value);
			foreach($selected_options as $option)
			{
				$sel[$option]=true;
			}
		}
		if ($isMultiple)
			$isMultiple = " multiple=\"true\"";
		else
			$isMultiple = "";

		$res = "<select ".$isMultiple." name=\"".$fname."\" ".$style.">";
		foreach (array_keys($a) as $k) 
		{
			$lab = &$a[$k];
			$res .= "<option value=\"$k\" ". (isset($sel[$k]) ? "selected=\"true\"" : "").">".$lab."</option>";
		}
		return $res."</select>";
	}
	
	function &getListAsArray()
	{
		return $this->desc;
	}
	
	function &getChoice($v)
	{
		if (isset($this->desc[$v]))
			return $this->desc[$v];
		else
			return $this->desc["?"];
	}

	function &getString($v)
	{
		if (isset($this->desc[$v]))
			return $this->desc[$v];
		else
			return $v;
	}
	
	function & getProperty($name,$deft=null,$cascade=true)
	{
		return $this->props->getProperty($name,$deft,$cascade);
	}
	
}

?>
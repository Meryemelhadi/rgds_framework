<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!isset($GLOBALS['form_autocomplete']))
	$GLOBALS['form_autocomplete']=false;

$GLOBALS['form_autocomplete_FID']=0;

class NxControl_autocomplete
{
	function toHTML(&$f,$opts)
	{
		if ($opts)
			return $f->toFormat($opts);

		$desc=&$f->desc;
			
		if (isset($f->value) && $f->value!=='')
		{
			if($desc->getProperty("value_is_text"))
				return $f->value;

			$list =$desc->getList();
			
			$vals = explode($desc->sep,$f->value);
			if (count($vals)>1)
			{
				$sep = $desc->getProperty("sep",'<br>',false);
				$vals = explode("|",$f->value);
				foreach($vals as $v)
					$res[]= $list->getChoice($v);
					
				return implode($sep,$res);
			}
			else
				return $list->getChoice($f->value);
		}
		return '';
	}
	
	/** check that files are installed, else copy files from lib distribution */
	function init(&$props) {
		if ($GLOBALS['form_autocomplete'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/autocomplete/autocomplete.css'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/autocomplete/',NX_DOC_ROOT.'nx/controls/autocomplete/');
		}		$GLOBALS['form_autocomplete']=true;

		return false;
	}
	
	function toForm($value,&$f)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();
		$desc->setProperty('keyword',$value);
		$fId = $fname.'_'.($GLOBALS['form_autocomplete_FID']++);

		$recPrefix = $f->record->getPrefix();

		// get choice list array (as "value" => "label")
		if (trim($value) != '')
		{
			$list = $desc->getList();
			$listArray = $list->getListAsArray();
            if (isset($listArray[$value]))
                $inputVal = $listArray[$value];
            elseif (isset($listArray[htmlspecialchars_decode($value)]))
                $inputVal = $listArray[htmlspecialchars_decode($value)];
			else
				$inputVal = $value;
		}
		else
			$inputVal = '';

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $f->toHTML($opts).'<input type="hidden" name="'.$fname."\" value='".$inputVal."' />";

		// prepare ds parameters package, ds, limit, etc.
		$addAuto = $desc->getProperty('autocomplete_add','false');
		if($addAuto=='1' || $addAuto=='true' || $addAuto=='yes' )
			$addAuto=1;
		else
			$addAuto=0;

		$limit = $desc->getProperty('limit','10');
		$package=$desc->getProperty('nx.package','root');
        $ds = $desc->getProperty('ds_by_name');
        if(!$ds)
            $ds = $desc->getProperty('ds');
            
		$dspack = explode('@',$ds);
		if (count($dspack)==1)
			$dspack[1]=$package;
		$ds=implode('@',$dspack);

		$spec = $ds . '|' . $desc->getProperty('ds_field_value','oid').'|'.$desc->getProperty('ds_format','').'|'.$desc->getProperty('ds_format_info','').'|'.$package.'|'.$addAuto;
		$spec=base64_encode($spec);
		$url = $desc->getProperty('url','/nx/controls/autocomplete/autocomplete.php?').'&ds='.$spec;

		if($desc->getProperty('session','',false))
		{
			$url .= '&session=1';
		}

		$s='';

		// add resources once
		if (!$this->init($desc))
		{
			$s.='
	<script>
		if (typeof ajax_load_css != "undefined")
		{
			ajax_load_css("/nx/controls/autocomplete/autocomplete.css");
			// ajax_require("/nx/controls/scriptaculous1.8.1/scriptaculous.js","autocomplete");
		}
	</script>
';
		}

		// get other fields to include
		if($sources = $desc->getProperty("onchange_sources"))
			$ajsources = json_encode(explode(",",$sources));
		else
			$ajsources = '[]';

		// get other fields to include
		if($onchange = $desc->getProperty("onchange"))
		{
			$jonchange=<<<EOH
			{$onchange}
EOH;
		}
		else
			$jonchange='';
		
		$s.=<<<EOH
<input type="hidden" id="{$fId}" name="{$fname}" value="{$value}"/>
<input type="text form-control" id="{$fId}_text" name="{$fname}_text" value="{$inputVal}"/>
<div id="{$fId}_choices" class="autocomplete"></div>
<script type="text/javascript">

if (typeof window.ajax_require != "undefined")
{
		window.{$fId}_autocomplete = new Ajax.Autocompleter("{$fId}_text", "{$fId}_choices", "{$url}", 
		{
				paramName: "value", 
				minChars: 3,
				frequency:0.6,
				
				callback:function(etext, qs) {
					var form = etext.form;
					var sources = {$ajsources};
					if(sources.length > 0)
					{
						for (var i = 0; i < sources.length; i++) 
						{
							var val;
							var sourceFname = sources[i];
							if(form[sourceFname])
								val = form[sourceFname].value;
							else
							{
								if((k='{$recPrefix}'+sourceFname) && form[k])
									val = form[k].value;
								else
									continue;
							}

							qs+='&'+sourceFname+'='+val;
						}
					}
					return qs;
				},

				afterUpdateElement:function(etext, li) {
					var e = $('{$fId}');
					var value = e.value = li.id;
					var form = e.form;
					var text = li.innerHTML;				
					{$jonchange}
				}
		});
	// ajax_onload(function(){},"autocomplete");
}

</script>
EOH;

	return $s;
	}

/*
	function readForm(&$recData,&$f)
	{
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=='')
			return null;
		else
            return str_replace("'","\\'",$value);
          // return nl2br(htmlspecialchars($value,ENT_QUOTES));
	}
*/

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
	
		// $fname = $desc->name;
		$fname = $f->getAlias();
		$fnameText = "{$fname}_text";
		
		if (!isset($recData[$fname]))
			return null;

		if (isset($recData[$fnameText]) && $recData[$fnameText]=='')
			// emptied value
			return '';

		$addAuto = $desc->getProperty('autocomplete_add','false');
		if($addAuto=='1' || $addAuto=='true' || $addAuto=='yes' )
			$addAuto=1;
		else
			$addAuto=0;
			
		$values = $recData[$fname];
		if (is_array($values))
			$avalues = $values;
		else
			$avalues = explode(',',$values);

		foreach($avalues as $v)
		{
			$v = str_replace("'","\\'",$v);

			if($addAuto && preg_match('/\[(.+)\]/',$v,$matches))
			{     
				// found new value
				$v2 = $matches[1];
				if($v3=$this->addValue($f,$v2))
					$res[] = $v3;
			}
			else
				// existing value
				$res[]=$v;
		}

		$value = implode($desc->sep, $res);
		return $value;
	}


	function addValue($desc,$v2) {
		$ds = $desc->getProperty('ds');	
        $fv = $desc->getProperty('ds_field_value','oid');
		$fn = $desc->getProperty('ds_field_add_label');
		if(!$fn)
            $fn = $fv;
		include_once(NX_LIB.'orm/persistence.inc');
		$rec2 = new PersistentObject($ds,'empty');
		$rec2->$fn->db = $v2;
		$rec2->store($ds,'insert');
		if($fv=='oid')
			return $rec2->getLastInsertId();

		return $v2;
	}
}

?>
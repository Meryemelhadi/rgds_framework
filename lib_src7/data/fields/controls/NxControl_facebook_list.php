<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!isset($GLOBALS['form_facebook']))
	$GLOBALS['form_facebook']=false;

class NxControl_facebook_list
{
	function toHTML(&$f,$opts)
	{
		if ($opts)
			return $f->toFormat($opts);

		$desc=&$f->desc;
		if (isset($f->value))
		{
			$list = $desc->getList();
			
			$vals = explode($desc->sep,$f->value);
			if (count($vals)>1)
			{
				$sep = $desc->getProperty("sep",'<br>',false);
				// $vals = explode("|",$f->value);
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
		if ($GLOBALS['form_facebook'])
			return true;

		if (!is_file(NX_DOC_ROOT.'nx/controls/fb_list/fb_list.js'))
		{
			$fs=$props->getPlugin('FileSys');
			$fs->copyDir(NX_DIS.'www/nx/controls/fb_list/',NX_DOC_ROOT.'nx/controls/fb_list/');
		}		
		$GLOBALS['form_facebook']=true;

		return false;
	}
	
	function toForm($value,&$f)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		$desc=&$f->desc;
		$fname = $f->getAlias();

		// get choice list array (as "value" => "label")
		$list = $desc->getList();
		$listArray = $list->getListAsArray();

		$options = '';
		if (isset($value) && $value!='')
		{
			$value=html_entity_decode($value,ENT_QUOTES,$NX_CHARSET);
			$selected_options = explode($desc->sep,$value);
			$fb_value=implode(',',$selected_options);
			foreach($selected_options as $option)
			{
				$option = trim($option);

				if (isset($listArray[$option]))
					$v = $listArray[$option];
				else
					$v = $option;

				if ($v)
				$options .= <<<EOH
<li value="{$option}">{$v}</li>
EOH;
			}
		}

		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$value."' />";
	
		if (!$desc->isEdit())
			return $f->toHTML($opts).'<input type="hidden" name="'.$fname."\" value='".$value."' />";

		$limit = $desc->getProperty('limit','10');
		$package=$desc->getProperty('nx.package','root');
		$ds = $desc->getProperty('ds');
		$dspack = explode('@',$ds);
		if (count($dspack)==1)
			$dspack[1]=$package;
		$ds=implode('@',$dspack);

		$spec = $ds . '|' . $desc->getProperty('ds_field_value','oid').'|'.$desc->getProperty('ds_format','').'|'.$desc->getProperty('ds_format_info','').'|'.$package;
		$spec=base64_encode($spec);
		$url = $desc->getProperty('url','/nx/controls/fb_list/autocomplete.php?').'&amp;ds='.$spec.'&amp;json=true&amp;limit='.$limit.'&amp;';

		$s='';

		// add resources once
		if (!$this->init($desc))
		{
			$s.='
	<link rel="stylesheet" href="/nx/controls/fb_list/fb_list.css" />
	<script>
		if (typeof ajax_require != "undefined")
		{
			/* ajax_require("/nx/controls/fb_list/protoculous-effects-shrinkvars.js","shrinkvars"); */
			ajax_require("/nx/controls/fb_list/textboxlist.js","textboxlist");
			ajax_require("/nx/controls/fb_list/fb_list.js","fb_list");
		}
	</script>
';
		}

		// get other fields to include
		$limit = $desc->getProperty('limit','10');
		$addAuto = $desc->getProperty('autocomplete_add','false');
		if($addAuto=='1' || $addAuto=='true' || $addAuto=='yes' )
			$addAuto=1;
		else
			$addAuto=0;

		$package=$desc->getProperty('nx.package','root');
		$ds = $desc->getProperty('ds');
		$dspack = explode('@',$ds);
		if (count($dspack)==1)
			$dspack[1]=$package;
		$ds=implode('@',$dspack);

		$spec = $ds . '|' . $desc->getProperty('ds_field_value','oid').'|'.$desc->getProperty('ds_format','').'|'.$desc->getProperty('ds_format_info','').'|'.$package.'|'.$addAuto;
		$spec=base64_encode($spec);
		$url = $desc->getProperty('url','/nx/controls/fb_list/autocomplete.php?').'&ds='.$spec.'&json=true&limit='.$limit.'&';

		$add_key = $desc->getProperty("add_key");
		if($add_key=='yes'||$add_key=='true')
			$url .= str_replace('&amp;','&',$f->record->getUrlKey());

		$prompt=$desc->getPrompt();


		$s.=<<<EOH

		<span class="facebook-list">
          <input type="text form-control" value="$fb_value" name="{$fname}" id="{$fname}" class=" form-control"/>
          <div id="{$fname}-auto" class="facebook-auto">
            <div class="default">Entrez : {$prompt} ...</div> 
            <ul class="feed  form-control">
		{$options}
            </ul>
          </div>

			<script>
			if (typeof ajax_require != 'undefined') 
			{ 
				ajax_onload(
					function() 
					{
					  // init
					  {$fname}_list = new FB_List('{$fname}', '{$fname}-auto',{fetchFile:"$url",newValues: true,placeholder:"$prompt"});
					  
					  // fetch and feed
					  /*
					  new Ajax.Request('/nx/controls/fb_list/json.php', 
					  {
						onSuccess: function(transport)
						{
							transport.responseText.evalJSON(true).each(function(t){{$fname}_list.autoFeed(t)});
						}
					  });
					  */
					},"fb_list");
			}
			</script>
		</span>
EOH;

		return $s;
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;
		
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]))
			return null;

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
			if($addAuto && preg_match('/\[(.+)\]/',$v,$matches))
			{
				// found new value
				$v2 = $matches[1];
				if($this->addValue($f,$v2))
					$res[] = $v2;
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
		include_once(NX_LIB.'orm/persistence.inc');
		$rec2 = new PersistentObject($ds,'empty');
		$rec2->$fv->db = $v2;
		$rec2->store($ds,'insert');
		return true;
	}
}

?>
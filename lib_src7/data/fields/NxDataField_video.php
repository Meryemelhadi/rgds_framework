<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxDataField_video extends NxData_Field
{
	var $info;
	
	function __construct(&$desc)
	{
		$this->desc =& $desc;
	}

	function clear()
	{
		$this->info = null;	
	}

	function & toObject($opts=null)
	{ 
		return $this->info; 
	}

	function toStatus() { 
		return isset($this->errorMsg)?'error': 
			(empty($this->info["url"])?'empty':'ok');
	}

	function readFromStore(&$v){
		$this->info = $v;
	}

	function onDelete(&$errorCB)
	{
	}
	
	function clone_it(&$record,&$errorCB)
	{
		// create a new field with resource info
		$f = new NxDataField_video($this->desc);
		$f->record=&$record;

		if (isset($this->info["url"]))
		{
			$f->info = $this->info;
		}
		return $f;
	}

	function readFromDB($recData)
	{
		$this->info = $this->unserialise($this->getFieldData($recData,$this->getFName(),null));
	}

	function toDB($opts=null) 
	{
		if (is_array($this->info))
		{
			return "'".$this->serialise($this->info)."'";
		}
		else
		{
			return "NULL";
		}
	}

	function toHTML($opts=null,$box=null)
	{
		$this->type='html';
		$this->box=null;
		$this->style='';
		if ($opts!=null)
			preg_replaceX(array(
				'/(left|right)/ie',
				'/(middle)/ie',
				'/(js|html|url|string)/ie',
				'/(original|[0-9]+x[0-9]+)/ie'),
				array(
					' $this->style="float:$1"',
					' $this->style="clear:both"',
					'$this->type="$1"',
					'$this->box="$1"'),
				$opts,$this);
				
		if (isset($this->info))
			switch($this->type)
			{
				case 'url':
					if (isset($this->info['url']))
						return $this->info['url'];
				case 'js':
					return ''; // a voir, services webs...
				default:
					// optimisation: store url in db ?
					if (isset($this->info['url']))
						return $this->_toHTML($this->box,false);
					else
						return "&nbsp;";
			}
		
		return '';
	}
	
	function _toHTML($box,$isFull=true) {
		if (empty($this->info))
			return '';
		
		$info=$this->info;
		
		if ($box==null)
			// get box from DML
			$box = $this->desc->getProperty("box",$this->desc->getProperty("imageSize",null,false),false);
			
		if ($box != null && $box != "original")
		{
			// get dimensions from box name WxH
			if (preg_match("/^([0-9]+)[xX]([0-9]+)$/", $box, $match) != 0)
			{
				$info['w']=$match[1];
				$info['h']=$match[2];
			}
		}
		
		$s= <<<EOH
<div><object width="{$info['w']}" height="{$info['h']}"><param name="movie" value="{$info['url']}"></param><param name="allowfullscreen" value="true"></param><embed src="{$info['url']}" type="application/x-shockwave-flash" width="{$info['w']}" height="{$info['w']}" allowfullscreen="true"></embed></object>
EOH;

		if ($isFull)
		$s.=<<<EOH
<br /><b><a href="{$info['page_url']}">{$info['titre']}</a></b><br /><i>envoy&eacute; par <a href="{$info['author_url']}">{$info['author_name']}</a></i></div>
EOH;
		return $s;
	
	}
	
	// to finish...
	function toJS($opts='\'',$box=null)
	{
		if ($opts!='')
			$sep=$opts;
		else
			$sep='\'';

		if (!isset($this->info["url"]))
			return 'error:"no image"';

		$url = $this->desc->getProperty("baseUrlUpload","/images/");
		if ($box==null)
			$box = $this->desc->getProperty("imageSize",null,false);
		if ($box != null && $box != "original")
			$url .= $box . "/";
		if (isset($this->info['dir']))
			$url = "{$url}{$this->info['dir']}/";
		$name=$this->info["url"];
			
		return "name:$sep$name$sep,url:$sep$url" . $this->info["url"]."$sep,box:$sep$box$sep";
	}
	
	function toForm($opts=null) 
	{
		$desc = & $this->desc;
		
		$value=$this->_toHTML(null);

		// appear in the form
		if (!$desc->isInput())
			return '';
				
		// is hidden field
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$value."\" />";

		// is hidden field + display?
		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$desc->getFormName()."\" value=\"".$value."\" />";			

		// get control class (ie text, telephone, email, etc.)
		$control =& $desc->getControl($desc->getProperty("control","text",false));
		return $control->toForm(htmlspecialchars($value,ENT_QUOTES),$this);
	}

	// dependedncy on properties:
	// "imageSizes" : name of the box to which the image should be resized to, 
	// otherwise fetches its dimension array in:
	// "image.box.100x100" => array("w"=>100,"h"=>100) or
	// "image.box.thumb" => array("w"=>10,"h"=>10)
	// otherwise if name of the form 100x40, use it as dimensions.
	// If no box or not found, no resize is done (simple copy)
	// NB. use GD for image resizing.
	// Returns image object as:
	// 	"url,w,h,type"
	function _readFromForm(&$recData,&$errorCB)
	{	
		$desc = & $this->desc;			
		//$control =& $desc->getControl($desc->getProperty('control','image_db'));
		//return $control->readForm($this,$desc,$recData,$errorCB);
		$this->info=array();

		// $fname = $desc->name;
		$fname = $this->getAlias();
		
		if (!isset($recData[$fname]))
		{
			nxLog("Video field doesnt exist [$fname]");
			return null;
		}

		
		preg_replaceX(
			array(
				'#\\\\#', // pour magic quote...
				'/src="([^"]+)"/ie',
				'/width="([^"]+)"/ie',
				'/height="([^"]+)"/ei',
				'#<b><a href="([^"]+)">([^<]+)</a></b>#ie',
				'#par <a href="([^"]+)">([^<]+)</a>#ie',
				),
			array(
			'',
			'$this->info["url"]="$1";',
			'$this->info["w"]="$1";',
			'$this->info["h"]="$1";',
			'$this->info["page_url"]="$1"?$this->info["titre"]="$2":"";',
			'$this->info["author_url"]="$1"?$this->info["author_name"]="$2":""',	
		), $recData[$fname],$this);

		if (!isset($this->info['url']))
		{
			$this->info=array();
			nxLog("Video field [$fname] : bad format: {$recData[$fname]}");
		}
		
		return true;
	}
}


?>
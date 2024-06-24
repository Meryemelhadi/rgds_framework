<?PHP

include_once(NX_TAG_LIB.'nxfrontier/xo/core/taglib_nxfrontier_xo_core.inc');

// cms article library
class taglib_nxfrontier_cms_article extends taglib_nxfrontier_xo_core
{
	/*
	// default handlers
	function _tag_default($node,$level)
	{
		return true;
	}
	function tag_default($node,$content,$level,$pad,$ns,$op)
	{
		return $content;
		// return $this->passThrough($node,$content,$pad);
	}
	*/
	
	// generate article type list
	function articles($node,$content,$level,$pad)
	{
		$src='{!-- article types
	values are keys for article type descriptions (data,logic,templates).
	(do not edit, automatically generated from NXL)
 --}
';
		$arts=$this->context->getResources('article',null,true);
		if (count($arts)>0)
		{
			foreach ($arts as $type=>$label)
			{
				$src.="{$type}={$label}\n";
			}
		}
		$this->context->saveSrc(NX_CONF."content/lists/article_types.values",$src);
		return $content;	
	}
	
	// <article	type="about" label="About">
	function article($node,$content,$level,$pad)
	{
		if (($type=$node->getAttribute('type'))===null)
			return $this->fatal('no type attribute');
		
		$label=$node->getAttribute('label',$type);
		
		$this->context->addResource($label,'article',$type);
		$src='';
		/*
		parts=simple
		subparts=textbox		
		*/
		$parts=$this->context->getResources('part',null,true);
		if (count($parts)>0)
		{
			$src.='parts=';
			foreach ($parts as $part)
			{
				$src.="{$part['name']},";
			}
			$src=rtrim($src,',')."\n";
		}
		
		$parts=$this->context->getResources('subpart',null,true);
		if (count($parts)>0)
		{
			$src.='subparts=';
			foreach ($parts as $part)
			{
				$src.="{$part['name']},";
			}
			$src=rtrim($src,',')."\n";
		}

		$this->context->saveSrc(NX_CONF."articles/{$type}.props",$src);
		nxltrace("Article type:$type",'compil');
		return '';
	}
	
	function parts($node,$content,$level,$pad)
	{
		return '';
	}
	
	// <part name="simple" label="main text" multiple="no" group="main"/>
	function part($node,$content,$level,$pad)
	{
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('no name attribute');
			
		$label=$node->getAttribute('label',$name);
		$group=$node->getAttribute('group','main');
		if ($multiple = $this->getBool($node->getAttribute('multiple','no')))
			$type='subpart';
		else
			$type='part';
		
		$this->context->addResource(array('name'=>$name,'label'=>$label,'multiple'=>$multiple,'group'=>$group),$type);
		return '';
	}
}

?>
<?PHP

// data sources
class taglib_nxfrontier_nxl_button extends NxTagLib
{	
	function header()
	{
		return "// import library button";
	}
	
	function set($node,$content,$level,$pad)
	{
		// button name
		if (($name=$node->getAttribute('name'))==null)
		{
			return $this->fatal('no "name" attribute');
		}	
		$name=$this->parseVal($name,true);

		// parameters
		$this->getAttribute($params,$node,'params','');
		$params=trim($params);
		if ($params!='')
		{
			$params=".$params";
		}
		$params.=trim($content);
		
		// url?
		if (!$this->getAttribute($url,$node,'url',null,true,true))
		{			
			// page
			if($page=$node->getAttribute('page'))
			{
				$page=$this->parseVal($page,true);
				
				// try to guess page mode
				if ($page[0]=='/' ||($page[0]=='\'' && $page[1]=='/'))
					$mode='absolute';
				else
					$mode='relative';
					
				// if mode specified, then fine else we use our guess
				$this->getAttribute($mode,$node,'mode',$mode,true,false);

				if($mode=='absolute')
				{
					$page = '(defined(\'PORTAL_URL_PREFIX\')?PORTAL_URL_PREFIX:\'\').' . $page;
				}
				
			}
			else
			{
				$page='$_SERVER["PHP_SELF"]';
				$mode='absolute';
			}

			// add domain name ?
			if ($this->getAttribute($isdomain,$node,'withDomain') && $this->getBool($isdomain))
			{
				if ($mode=='relative')
				{
					$page='dirname($_SERVER[\'PHP_SELF\']).\'/\'.'.$page;
					$mode='absolute';
				}
				
				$url="\$this->getProperty('site.url.noprefix','').$page";
			}
			else
				$url=$page;
				
			// operation and step
			if ( $this->getAttribute($op,$node,'operation') ||
				 $this->getAttribute($step,$node,'step'))
			{
				$op=$this->parseVal($node->getAttribute('operation','#prop:app.operation#'),true);	
				$step=$this->parseVal($node->getAttribute('step',''),true);
				$params=".'nxo='.{$op}.'&amp;nxs='.{$step}{$params}";
			}
		}

		// add ? if any param
		if ($params!='')
			$url="$url.'?'{$params}";
			
		// label
		if (($label=$node->getAttribute('label'))!==null)
		{
			$label=$this->parseVal($label,true);
			$src="\n{$pad}\$this->setProperty(".$name.".'.label',{$label});";
		}
		else
			$src='';		

		// class
		if (($this->getAttribute($cl,$node,'class',null,true,true)!=null) && trim($cl)!="''")
		{
			$src.="\n{$pad}\$this->setProperty(".$name.".'.class',{$cl});";
		}
			
	//	return "\$this->getUrl('{$step}','{$op}','{$page}')";
		return  $this->simplifyExpr("{$src}\n{$pad}\$this->setProperty(".$name.".'.url',{$url});");
	}
	
	function get_url($node,$content,$level,$pad)
	{
		// parameters
		$this->getAttribute($params,$node,'params','');
		$params=trim($params);
		if ($params!='')
		{
			$params=".$params";
		}
		$params.=trim($content);
		
		// url?
		if (!$this->getAttribute($url,$node,'url',null,true,true))
		{			
			// page
			if($page=$node->getAttribute('page'))
			{
				$page=$this->parseVal($page,true);
				
				// try to guess page mode
				if ($page[0]=='/')
					$mode='absolute';
				else
					$mode='relative';
					
				// if mode specified, then fine else we use our guess
				$this->getAttribute($mode,$node,'mode',$mode,true,false);
				
			}
			else
			{
				$page='$_SERVER["PHP_SELF"]';
				$mode='absolute';
			}

			// add domain name ?
			if ($this->getAttribute($isdomain,$node,'withDomain') && $this->getBool($isdomain))
			{
				if ($mode=='relative')
				{
					$page='dirname($_SERVER[\'PHP_SELF\']).\'/\'.'.$page;
					$mode='absolute';
				}
				
				$url="\$this->getProperty('site.url.noprefix','').$page";
			}
			else
				$url=$page;
				
			// operation and step
			if ( $this->getAttribute($op,$node,'operation') ||
				 $this->getAttribute($step,$node,'step'))
			{
				$op=$this->parseVal($node->getAttribute('operation','#prop:app.operation#'),true);	
				$step=$this->parseVal($node->getAttribute('step',''),true);
				$params=".'nxo='.{$op}.'&amp;nxs='.{$step}{$params}";
			}
		}

		// add ? if any param
		if ($params!='')
			$url="$url.'?'{$params}";
			
	//	return "\$this->getUrl('{$step}','{$op}','{$page}')";
		return  $this->simplifyExpr($url);
	}
	
	// url param for button
	function param($node,$content,$level,$pad)
	{
		// name
		if (($name=$node->getAttribute('name'))===null)
		{
			return ".$content";
		}
		$name=$this->parseVal($name,true);
		
		// value
		$this->getAttribute($v,$node,'value',$content,true,true);
		
		if ($this->getAttribute($parse,$node,'encode','false') && $this->getBool($parse))
		{
			$v="urlencode($v)"; 
		}

		
		return ".'&amp;'.$name.'='.$v";
	}
}

?>
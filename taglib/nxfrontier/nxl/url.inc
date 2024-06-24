<?PHP

// parse url for generating get params
class taglib_nxfrontier_nxl_url extends NxTagLib
{		
	/* <url:parse regex="^([^-./]+)(-([^./-]+))?\.html$">
			<url:param name="topic" value="$1" /> 
			<url:param name="page" value="$3" /> 
		</url:parse>

	preg_replace('#^/?([^-./]+)(-([^./-]+))?\.html$#ei','($_GET["topic"]="$1").($_GET["page"]="$3");', $_SERVER['REQUEST_URI']);
	$NX_CACHE=NX_DOC_ROOT."article/$NX_DOMAIN/{$_GET['topic']}/";
	$NX_CACHE_ID=(($_GET["page"]!='')?$_GET["page"]:'index'). '.html';
	 */
	function _parse()
	{
		$this->context->indentHTML=false;
		$this->getResources('parse_url_deft',null,true);
	}
	
	function parse($node,&$content,$level,$pad)
	{	
		// get expression to map
		if(!$this->getAttribute($regex,$node,'regex',null,true,false))
			return $this->fatal('no regex param');
		$this->context->indentHTML=true;
		
		// get server var
		$this->getAttribute($mode,$node,'mode','direct');
		switch ($mode)
		{
			case 'redirect': 
				$var="\$_SERVER['REDIRECT_URL']";
				break;
			// case 'direct':
			default;
				$var="\$_SERVER['REQUEST_URI']";
		}
				
		
		return 	"preg_replace('#$regex#ei',\n{$pad}'$content\"\"',\n{$pad}{$var});\n".
					$this->getResources('parse_url_deft',"\n",true)."\n";
	}
	
	function param($node,&$content,$level,$pad)
	{	
		// get name
		if(!$this->getAttribute($name,$node,'name',null,false,false))
			return $this->fatal('no name param');
			
		// get value
		$this->getAttribute($value,$node,'value',$content,false,false);
		
		// get default if any
		if ($this->getAttribute($default,$node,'default',null,true,true))
		{
			$this->addResource("if (!isset(\$_GET['$name']) || \$_GET['$name']=='') \$_GET['$name']=$default;",'parse_url_deft');
		}
		
		return 	"(\$_GET[\"$name\"]=\"$value\").";		
	}
	
	function _func_current($node,$args,$level=0)
	{
		return 'htmlentities(urlencode($_SERVER[\'REQUEST_URI\']))';
	}
	
	function set_current($node,&$content,$level,$pad)
	{
		if (!$this->getAttribute($value,$node,'value',null,true,true))
			return $this->fatal('no value attribute');
		
		return "\n{$pad}\$_SERVER['REQUEST_URI']=$value;";
	}
}

?>
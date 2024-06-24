<?PHP

// library for the creation of javascript classes
class taglib_nxfrontier_js_js extends NxTagLib
{	
	function script_($node,$content,$level,$pad)
	{
		// get path details
		if ($this->getAttribute($name,$node,'name'))
		{
			$name=trim($name,'/\\');

			$this->getAttribute($path,$node,'path','/');					
			$path=trim($path,'/\\');

			if ($path)
				$name="$path/$name";

			$this->getAttribute($skin,$node,'skin','default');			
			$rootPath=NX_CONF."view/{$skin}/res/js/";
			
			$name=preg_replace('|[.]|','/',$name);
			$path="{$rootPath}{$name}.jsx";
		}
		else
			$path='';
		
		// trace
		if ($this->getAttribute($trace,$node,'trace','no') && $this->getBool($trace))
		{
			nxltrace("CLIENT CODE: [$path] <br/><pre>".$content."</pre>",'compil');
		}
		else
			nxltrace("CLIENT CODE: [$path]",'compil');
		
		if ($path!='')
		{
			$this->context->saveSrc($path,$content);
			return '';
		}
		else
		{
			return "\n<script>\n$content\n</script>";
		}
	}
	
	function include_($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no name attribute');
			
		$name = preg_replace('/[.]/','/',$name);
		
		// return "{include:js/{$name}:skin and device}\n";
		return "@include(js/{$name},skin and device)\n";
	}
	
	
	function _class_(&$node,$level)
	{
		if (!$this->getAttribute($cl,$node,'name'))
			return $this->fatal('missing name attribute');
		$this->className=$cl;

		if ($this->getAttribute($inh,$node,'inherit'))
			$this->inherit=$inh;
		else
			$this->inherit=null;
	}

	// inherit from NxHtmlElt
	// NxInput.prototype= new NxHtmlElt();

	function class_($node,$content,$level,$pad)
	{
		return "\n// class {$this->className}$content";
	}
	
	function constructor($node,$content,$level,$pad)
	{	
		$params=$this->context->getResources('params',',',true);
		
		$src="function {$this->className}($params) {{$content}\n}\n";
		
		if ($this->inherit!='')
		{
			$src .= "\n{$this->className}.prototype=new {$this->inherit}();\n";
		}
		

		return $src;	
	}
	
	function method($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');

		$params=$this->context->getResources('params',',',true);
		
		$src="{$this->className}.prototype.{$name}=function($params){{$content}\n}";

		return $src;	
	}
	
	function param($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('missing name attribute');
			
		$this->context->addResource($name,'params');
	}
	
	/*
		<js:navigate url="imagePreview.php" target="preview">
			<js:urlParam name="file" value="name" />
			<js:urlParam name="box" value="box" />
		</js:navigate>
	*/
	function navigate($node,$content,$level,$pad)
	{
		// get url
		$this->getAttribute($url,$node,'url','');
		if (strstr($url,'?')===false)
			$url.='?';
		else
			$url.='&';
			
		// get url params
		$url.=$this->context->getResources('urlParams','&',true);
			
		if ($this->getAttribute($target,$node,'target'))
		{
			// frame or iframe
			return "	document.frames['$target'].location.href='$url';";
			
		}
		else
		{
			// current doc
			return "	document.location.href='$url';";
		}
	}
	
	function urlParam($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no name attribute');
			
		$this->getAttribute($v,$node,'value','');

		$this->context->addResource("$name='+$v+'",'urlParams');
		return '';
	}
	
	function text_($node,$content,$level,$pad)
	{	
		return $this->parseVal($node->data);
	}	
	
	function expression_($node,$expr)
	{
		switch(trim($expr))
		{
			case 'num_and':return '&';
			case 'num_or':return '|';
			
			case 'and': return '&&';
			case 'or': return '||';
			case 'lt': return '<';
			case 'gt': return '>';
			case 'lte': return '<=';
			case 'gte': return '>=';
			
			case 'nbsp': return '&nbsp;';
			case 'amp': return '&amp;';
			default: return $expr;
		}
	}

	function load_lib($node,$content,$level,$pad)
	{
		return "require_once NX_LIB.'js/utils.inc';\n";
	}

	function php2js($node,$content,$level,$pad)
	{
		/* value */
		$this->getAttribute($value,$node,'value',$content);
		
		// get a variable ID
		/* if (!$this->getAttribute($name,$node,'name'))
			$name=$this->createVarID('array'); */
			
		$this->getAttribute($name,$node,'name');
				
		// get mapping type for arrays: as objects or as arrays
		$this->getAttribute($array_map,$node,'array','as-object',true,true);
			
		/* compil or runtime */
		if ($this->getAttribute($mode,$node,'mode') && $mode=='compil')
		{		
			// compil time transformation
			include_once(NX_LIB.'js/utils.inc');
			eval("\$php=$content;");
			return PHP2Js($php,$name,'literal',$array_map);
		}

		// runtime transformation
		
		// add link to run time js lib
		$this->context->addInclude('js/utils.inc');
		
		if ($name!==null)
			$sep=';';
		else
			$sep='';
			
		return "\n{$pad}PHP2Js({$value},null,'literal',{$array_map}){$sep}";
	}
}

?>
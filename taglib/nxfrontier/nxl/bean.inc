<?PHP

// core library for nxl language
class taglib_nxfrontier_nxl_bean extends NxTagLib
{
	function header()
	{
		return "// import library bean";
	}
	
	function call($node,$content,$level,$pad)
	{			
		/* class */
		if (!$this->getAttribute($class,$node,'class','',true,true))
			return $this->fatal('error in bean:call : no "class" attribute');

		/* name */
		if (($name=$node->getAttribute('method'))===null)
			return $this->fatal('error in bean:call : no "method" attribute');
		
		/* value */
		$this->getAttribute($params,$node,'params',$content);
		// instruction or expression ?

		/* root */
		$this->getAttribute($root,$node,'root','',true,true);
		// instruction or expression ?

		if ($this->context->isInBlock($node))
			$sep=';';
		else
			$sep='';

		return "\n{$pad}((\$__o=\$this->getHandler($class,$root)) ? \$__o->$name($params) : null)$sep";
	}
}

?>

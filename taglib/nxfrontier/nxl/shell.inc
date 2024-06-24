<?PHP

// all data format operations
class taglib_nxfrontier_nxl_shell extends NxTagLib
{		
	function exec($node,&$content,$level,$pad)
	{		
		if (!$this->getAttribute($cmd,$node,'cmd',null,true,true))
			return $this->fatal('no cmd attribute');

		// instruction or expression ?
		if ($this->context->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
				
		return "\n{$pad}shell_exec(escapeshellcmd($cmd).' 2>&1'){$sep}";
	}

}

?>
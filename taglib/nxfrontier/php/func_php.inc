<?PHP

// array collection
class taglib_nxfrontier_php_func_php extends NxTagLib
{		
	
	function tag_default($node,$content,$level,$pad,$ns,$op)	
	{	
		if (!function_exists($op))
		{
			// function not builtin, try in pear compat
			$compat_f="PHP_Compat/Compat/Function/$op.php";
			if (is_file(NX_PEAR.$compat_f))
				$this->context->addInclude($compat_f,'NX_PEAR');
			else
				return $this->fatal("php function is not defined and not in pear compat (".NX_PEAR."$compat_f): $op");
		}

		$fp='';
		if ($this->getAttribute($p1,$node,'p1','(_none_)') && $p1!='(_none_)')
		{	
			$fp=$p1;
			for ($i=2;$i<10;$i++)
			{
				if (!$this->getAttribute($p,$node,"p{$i}",'(_none_)') || $p=='(_none_)')
					break;
					
				$fp.=",$p";		
			}
		}

		return "$op($fp)";
	}

	function _script_($node)
	{	
		$this->keepAllText($node,true);
	}
	
	function script_($node,$content,$level,$pad)
	{
		return "{$pad}/*start script*/\n{$content}\n{$pad}/*end script*/";
	}
}

?>
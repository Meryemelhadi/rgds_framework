<?PHP

// all data format operations
class taglib_nxfrontier_functions_numeric extends NxTagLib
{		
	function decrement($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "(($v) - 1)";
		}
		else
			return ((int)$v) - 1;
	}
	
	function increment($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "(($v) + 1)";
		}
		else
			return ((int)$v) + 1;
	}

	function mod($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		
		if (!$this->getAttribute($div,$node,'divider',null,true,false))
			return $this->fatal('no divider attribute');	
		
		if (preg_match('/\$\w/',$v) || preg_match('/\$\w/',$div))
		{
			return "((float)($v) % (int)($div))";
		}
		else
			return (int)($v) % (int)$div;
	}

	function mul($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		
		if (!$this->getAttribute($div,$node,'mul',null,true,false))
			return $this->fatal('no mul attribute');	
		
		if (preg_match('/\$\w/',$v) || preg_match('/\$\w/',$div))
		{
			return "($v * $div)";
		}
		else
			return ($v * $div);
	}
	
	function apply($node,&$content,$level,$pad)
	{		
		if (!$this->getAttribute($op,$node,'op',null,true,false))
			return $this->fatal('no op attribute');	
			
		$this->getAttribute($v,$node,'value',$content,true,false);
		
		if (!$this->getAttribute($v2,$node,'value2',null,true,false))
			return $this->fatal('no value2 attribute');	
		
		return "($v $op $v2)";
	}

	function cast($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);

		if (!$this->getAttribute($type,$node,'type',null,true,false))
			return $this->fatal('no type attribute');	
			
		return "($type)($v)";
	}
}

?>
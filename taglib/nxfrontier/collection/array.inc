<?PHP

// array collection
class taglib_nxfrontier_collection_array extends NxTagLib
{		
/*
	<set name="branch">
		<array:create>
			<array:item key="id"><ds:field name="cat.id" record="#$record#" format="object" /></array:item>
			<array:item key="name"><ds:field name="name" record="#$record#" format="object" /></array:item>
			<array:item key="fullname"><ds:field name="fullname" record="#$record#" format="object" /></array:item>
		</array:create>
	</set> */	
	function create(&$node,&$content,$level,$pad)
	{	
		if ($content!='')
			return "array(\n$content\n$pad)";
		else
			return "array()";
	}

	function item($node,&$content,$level,$pad)
	{

		if (!$this->getAttribute($value,$node,'value',null,true,true))
			if ($content!='')
			{
				if (preg_match('/\$/',$content))
					$value=$content;
				else
					$value="'$content'";
			}
			else
				return $this->fatal('no value');
				
		if ($this->getAttribute($name,$node,'key',null,true,true))
			return "{$pad}$name=>$value,\n";
		else
			return "{$pad}$value,\n";
	}

	// add an item to an array
	// <array:add array="#$clients#" key="#$oid#">
	//			<ds:field name="name" format="String" record="#$item#"/></array:add>
	function add($node,&$content,$level,$pad)
	{
		if (!$this->getAttribute($value,$node,'value',null,true,true))
			if ($content!='')
			{
				if (preg_match('/\$/',$content))
					$value=$content;
				else
					$value="'$content'";
			}
			else
				return $this->fatal('no value');

		if (!$this->getAttribute($var,$node,'array',null,true,true))
			return $this->fatal('no array attribute');
				
		if ($this->getAttribute($name,$node,'key',null,true,true))
			return "\n{$pad}{$var}[$name]=$value;\n";
		elseif ($this->getAttribute($name,$node,'index',null,true,false))
			return "\n{$pad}{$var}[(int)$name]=$value;\n";
		else
			return "\n{$pad}{$var}[]=$value;\n";
	}
	
	function tag_default($node,$content,$level,$pad,$ns,$op)	
	{
		if (!function_exists($op))
			$op="array_$op";
			
		if (!function_exists($op))
		{
			// function not builtin, try in pear compat
			$compat_f="PHP_Compat/Compat/Function/$op.php";
			if (is_file(NX_PEAR.$compat_f))
				$this->context->addInclude($compat_f,'NX_PEAR');
			else
				return $this->fatal("array function is not defined and not in pear compat (".NX_PEAR."$compat_f): $op");
		}
			
		$this->getAttribute($p1,$node,'p1','null');
		$this->getAttribute($p2,$node,'p2','null');
		$this->getAttribute($p3,$node,'p3','null');

		return "$op($p1,$p2,$p3)";
	}
}

?>
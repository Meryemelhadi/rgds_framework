<?PHP

// array collection
class taglib_nxfrontier_collection_properties extends NxTagLib
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
		return "new NxProperties(\$p=array($content),\$parent=null)";
	}

	function item($node,&$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'key',null,true,true))
			return $this->fatal('no key');

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
				
		return "{$pad}$name=>$value,\n";
	}
	
	function loadProperties($node,&$content,$level,$pad)
	{
		if (!$this->getAttribute($from,$node,'from',null,true,true))
			return $this->fatal('no "from" attribute');

		if (!$this->getAttribute($to,$node,'to',null,true,true))
			return "\n{$pad}\$this->addProperties($from);";
		else
			return "\n{$pad}$to->addProperties($from);";
	}
}

?>
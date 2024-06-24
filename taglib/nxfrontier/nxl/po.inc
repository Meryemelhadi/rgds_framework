<?PHP

// data sources
class taglib_nxfrontier_nxl_po extends NxTagLib
{	
	function header()
	{
		return "// import library PO: persistent objects";
	}
	
	// <ds:load media="post" ds="#param:ptype#.edit" prop="page.records" multiple="no"/>
	function load($node,$content,$level,$pad)
	{
		/* media */
		$media=$node->getAttribute('media','db');
		$media=$this->parseVal("'{$media}'");
		
		/* ds */
		if (($ds=$node->getAttribute('ds'))==null)
			return '/* error in ds:load : no "ds" attribute*/';			
		$ds=$this->parseVal("'{$ds}'");
		
		/* property */
		$prop=$node->getAttribute('property','page.records');
		$prop=$this->parseVal("'{$prop}'");
		
		/* multiple */
		if ($mult=$this->getBool($node->getAttribute('multiple','no')))
			$method='getRecords';
		else
			$method='getRecord';

		/* data (ex. record source) */
		
		/* records */
		if (!$this->getAttribute($data,$node,'records') &&
			!$this->getAttribute($data,$node,'record'))
			$this->getAttribute($data,$node,'data','null');

		/* properties */
		$this->getAttribute($props,$node,'properties','null');
		
		// instruction or expression ?
		if ($this->context->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';

		// get params for the DML
		$params = $this->context->getResources('ds:property','',true);
		if ($params)
			$props="array({$params})";
		else
			$this->getAttribute($props,$node,'properties','null');
			
		// add properties to current set ?
		if (($add=$node->getAttribute('addProperties'))!==null
			&& $this->getBool($add))
		{
			return "\n{$pad}\$this->$method($ds,$prop,$media,true,$data,$props){$sep}";
		}
		else
			// no properties to add
			return "\n{$pad}\$this->$method($ds,$prop,$media,false,$data,$props){$sep}";
	}

	function property($node,&$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name',null,true,true))
			return $this->fatal('no name');

		$this->getAttribute($dft,$node,'default',$content);

		if (!$this->getAttribute($value,$node,'value',$dft,true,true))
			return $this->fatal('no value');
				
		$this->context->addResource("{$pad}$name=>$value,\n",'ds:property');
		return '';
	}
	

	/*
		<ds:store ds="#param:ptype#.edit" media="db" mode="insert">
			<ds:load media="post" ds="#param:ptype#.edit" prop="page.records" multiple="no"/>
		</ds:store>
	*/
	function _store($node)
	{
		// push default record ds
		$this->getAttribute($ds,$node,'ds',null);
		$this->context->addResource($ds,'ds:ds');
	}
	
	function store($node,$content,$level,$pad)
	{
		// pop ds
		$this->context->getLastResource('ds:ds',true);

		/* media */
		$media=$node->getAttribute('media','db');
		$media=$this->parseVal("'{$media}'");
					
		/* mode */
		$mode=$node->getAttribute('mode','insert');
		$mode=$this->parseVal("'{$mode}'");

		// get params for the DML
		$params = $this->context->getResources('ds:property','',true);
		if ($params)
			$props="array({$params})";
		else
			$this->getAttribute($props,$node,'properties','null');
				
		/* records */
		if (($records=$node->getAttribute('records'))!==null)
		{
			// multiple records
			$records = $this->parseQvalue($records);
			return "\n{$pad}\$this->putRecords($records,$ds,$mode,$media,$props);";
		}
		else if (($record=$node->getAttribute('record'))!==null)
		{
			// one record
			$record = $this->parseQvalue($record);
			return "\n{$pad}\$this->putRecord($record,$ds,$mode,$media,'Object',$props);";
		}
		else if (($record=$node->getAttribute('value'))!==null)
		{
			// one record
			$record = $this->parseQvalue($record);
			return "\n{$pad}\$this->putRecord($record,$ds,$mode,$media,$props);";
		}
		else if (trim($content)!='')
		{
			// content
			return "\n{$pad}\$this->putRecord($content,$ds,$mode,$media,'Object',$props);";
		}	
		else
		{
			nxltrace('no record for ds:update. Assume it is not needed...?','warning');
			// return $this->warning('no records, record or content specified');
			return "\n{$pad}\$this->putRecord(\$null=null,$ds,$mode,$media,'Object',$props);";
		}
	}
	
	function delete($node,$content,$level,$pad)
	{
		/* media */
		$media=$node->getAttribute('media','db');
		$media=$this->parseVal("'{$media}'");
				
		/* ds */
		if (($ds=$node->getAttribute('ds'))==null)
			return '/* error in ds:store : no "ds" attribute*/';			
			
		$ds=$this->parseVal("'{$ds}'");

		// get params for the DML
		$params = $this->context->getResources('ds:property','',true);
		if ($params)
			$props="array({$params})";
		else
			$this->getAttribute($props,$node,'properties','null');
				
		/* records */
		if (($records=$node->getAttribute('records'))!==null)
		{
			// multiple records
			$records = $this->parseQvalue($records);
			return "\n{$pad}\$this->deleteRecords($ds,$records,$media,true);";
		}
		else if (trim($content)!='')
		{
			// inline content
			return "\n{$pad}\$this->deleteRecords($ds,$content,$media,true);";
		}
		else
		{
			// no record specified, load them
			$content="\$this->getRecords($ds,null,$media)";
			return "\n{$pad}\$this->deleteRecords($ds,$content,$media,true);";
		}
	}
	
}

?>
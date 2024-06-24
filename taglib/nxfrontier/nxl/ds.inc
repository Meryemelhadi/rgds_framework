<?PHP

// data sources
class taglib_nxfrontier_nxl_ds extends NxTagLib
{	
	function header()
	{
		return "// import library DS: data source";
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
			
		/* multiple */
		if ($mult=$this->getBool($node->getAttribute('multiple','no')))
			$method='getRecords';
		else
			$method='getRecord';

		/* property */
		if($mult)
			$dftP = 'page.records';
		else
			$dftP = 'page.record';

		$prop=$node->getAttribute('property',$dftP);
		$prop=$this->parseVal("'{$prop}'");

		/* data (ex. record source) */
		
		/* records */
		if (!$this->getAttribute($data,$node,'records') &&
			!$this->getAttribute($data,$node,'record'))
			$this->getAttribute($data,$node,'data','null');

		/* properties */
		$this->getAttribute($props,$node,'properties','null');
		
		// instruction or expression ?
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';

		// get params for the DML
		$params = $this->context->getResources('ds:property','',true);
		
		// get custom queries
		$queries = $this->context->getResources('ds:query','',true);
		if ($queries)
			$queries=','.$queries;

		if ($params)
			$props="(((\$__params=array({$params}))&&(\$__props=new NxProperties(\$__params,\$this)))?\$__props:null)";
			// $props="(($__o=array({$params}))?$o:null)";
		else
			$this->getAttribute($props,$node,'properties','null');
			
		// add properties to current set ?
		if (($add=$node->getAttribute('addProperties'))!==null
			&& $this->getBool($add))
		{
			//if ($mult)
				//return $this->fatal('cant add properties of multiple records');

			/*	
			$var=$this->createVarID('record');
			if ($sep==';')
				// statement
				return "\n{$pad}if((\${$var}=\$this->getRecord($ds,$prop,$media))!==null)
{$pad}\$this->addProperties(\${$var}->getProperties());";
			else
				// expression
				return "\n{$pad}(((\${$var}=\$this->getRecord($ds,$prop,$media))&&
{$pad}	\$this->addProperties(\${$var}->getProperties()))?\${$var}:null)";
			*/
			return "\n{$pad}\$this->$method($ds,$prop,$media,true,$data,$props{$queries}){$sep}";
		}
		else
			// no properties to add
			return "\n{$pad}\$this->$method($ds,$prop,$media,false,$data,$props{$queries}){$sep}";
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

	function query($node,&$content,$level,$pad)
	{
		$this->getAttribute($dft,$node,'default',$content);

		if (!$this->getAttribute($value,$node,'value',$dft,true,true))
			return $this->fatal('no value');
				
		$this->context->addResource("{$pad}$value",'ds:query');
		return '';
	}

	// count number of records collection
	function record_count($node,$content,$level,$pad)
	{
		if ($this->isInBlock($node))
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

		// direct SQL count ?
		if ($this->getAttribute($ds,$node,'ds',null,true,true))
		{
			$this->getAttribute($media,$node,'media','db',true,true);
			return "\n{$pad}\$this->countRecords({$ds},null,{$media},$props){$sep}";
		}
		
		// count a set of records that are already loaded..
		
		/* records */
		if (($records=$node->getAttribute('records'))!==null)
		{
			$records = $this->parseQvalue($records);
		}
		else
		{
			$records=$content;
		}

		/* select */
		// only support "first" for now..., but actually loads current record..
		// to be fixed !
		// $select=$node->getAttribute('select','first');
		
		// temp variable
		$recs_var=$this->createVarID('records');
				
		return "\n{$pad}((\${$recs_var}=$records)!=null?\${$recs_var}->count():0){$sep}";
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
		
		/* ds */
		if (($ds=$node->getAttribute('ds'))==null)
			return '/* error in ds:store : no "ds" attribute*/';			
			
		$ds=$this->parseVal("'{$ds}'");
			
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

	function update_record($node,$content,$level,$pad)
	{
		if(!$this->getAttribute($record,$node,'record'))
			return $this->fatal("no record attribute");
			
		if( !$this->getAttribute($other,$node,'update_with')&&
			!$this->getAttribute($other,$node,'updateWith',$content))
			return $this->fatal("no updateWith attribute or record content");

		/* if field exists, override it?
			override=yes, no, empty */
		if(!$this->getAttribute($override,$node,'override','empty'))
			return $this->fatal("no override attribute");

		/* if field doesnt exist append it?
			append=yes, no*/
		if(!$this->getAttribute($append,$node,'append','yes'))
			return $this->fatal("no append attribute");
			
		// instruction or expression ?
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';

		return "\n{$pad}((\$rec=&{$record}&&\$other=&{$other})?\$rec->updateWith(\$other,'$override','$append'):null)$sep";
	}

	// ======== TRANSACTIONS ===========
	function start_transaction($node,$content,$level,$pad)
	{
		return '';
	}
	
	function commit_transaction($node,$content,$level,$pad)
	{
		return '';
	}

	function rollback_transaction($node,$content,$level,$pad)
	{
		return '';
	}
	
	// ======== /TRANSACTIONS ===========
	
	function insert_id($node,$content,$level,$pad)
	{
		/* media */
		$media=$node->getAttribute('media','db');
		$media=$this->parseVal("'{$media}'");
		// instruction or expression ?
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';

		if($this->getAttribute($record,$node,'record'))
		{
			return "\n{$pad}{$record}->getLastInsertID{$sep}";
		}
							
		return "\n{$pad}\$this->getLastInsertID($media){$sep}";
	}
	
	// 
	// Collection of records
	// <ds:records ds="site.boot">
	// 	  <ds:record>
	// 		<ds:field name="name" value="test" />
	// 	  <ds:record>
	// <ds:record>
	//
	function _records($node,$content,$level,$pad)
	{				
		$ds=$this->context->getLastResource('ds:ds',false);
		
		// set default ds for records
		$this->getAttribute($ds,$node,'ds',$ds);
		$this->context->addResource($ds,'ds:ds');
	}
	
	function records($node,$content,$level,$pad)
	{		
		// pop records deft ds
		$ds=$this->context->getLastResource('ds:ds',true);
		
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
			
		// default ds?
		$ds=$this->context->getLastResource('ds:ds',false);
		
		/* ds */
		$this->getAttribute($ds,$node,'ds',$ds,true,true);

		$this->context->addInclude('NxData_RecordInputSource.inc');

		$d_var=$this->createVarID('records');

		return "\n{$pad}\${$d_var}=array({$content}){$sep}";
	}	

	function create_records($node,$content,$level,$pad)
	{		
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
			
		$this->context->addInclude('NxData_ArrayRecords.inc');

		$d_var=$this->createVarID('records');
		
		$this->getAttribute($data,$node,'value',$content);

		return "\n{$pad}\${$d_var}=new NxData_ArrayRecords({$data}){$sep}";
	}	

	function add_record($node,$content,$level,$pad)
	{
		if(!$this->getAttribute($records,$node,'records'))
			return $this->fatal("no records attribute");

		if(!$this->getAttribute($record,$node,'record'))
			return $this->fatal("no record attribute");
			
		return "((\$recs=&{$records})?\$recs->addRecord($record):null);";
	}

	function clone_records($node,$content,$level,$pad)
	{
		if(!$this->getAttribute($records,$node,'records',$content))
			return $this->fatal("no records attribute or content");
			
		return "((\$recs={$records})?\$recs->getRecordArray():null)";
	}
	
	// 2 interfaces:
	// 1) get record from collection of records
	// <ds:record records="sections" select="first" addProperties="yes"/>
	// 
	// 2) create record from data
	// <ds:record ds="site.boot">
	// 		<ds:field name="name" value="test" />
	// <ds:record>
	//
	function record($node,$content,$level,$pad)
	{		
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
			
		/* records */
		if (!$this->getAttribute($records,$node,'records',null,true,false))
		{
			// default ds?
			$ds=$this->context->getLastResource('ds:ds',false);
			
			/* ds */
			if (! $this->getAttribute($ds,$node,'ds',$ds,true,true))			
				return '/* error in ds:record : no "ds" attribute*/';

			$this->context->addInclude('NxData_RecordInputSource.inc');
			
			if (($parent=$this->context->getParentElement($node)) && $parent->localName=='records')
			{
				// collection orf records
				return "\n{$pad}array({$content}),";
			}
			else
			{
				// single record
				$d_var=$this->createVarID('record_data');	
				return "\n{$pad}\${$d_var}=array({$content}){$sep}";
			}
		}
		
		/* select */
		// only support "first" for now..., but actually loads current record..
		// to be fixed !
		// $select=$node->getAttribute('select','first');
		
		// temp variable
		$recs_var=$this->createVarID('records');
				
		// add properties to current set ?
		if (($add=$node->getAttribute('addProperties'))!==null
			&& $this->getBool($add))
		{				
			$rec_var=$this->createVarID('records');
			
			return "\n{$pad}(
{$pad}	(
{$pad}		(\${$recs_var}=$records)!=null && 
{$pad}		(\${$rec_var}=\${$recs_var}->getCurrent()) && 
{$pad}		\$this->addProperties(\${$rec_var}->getProperties())
{$pad}	)
{$pad}	? \${$rec_var}
{$pad}	: null
{$pad}){$sep}";
		}
		else
			// no properties to add
			return "\n{$pad}((\${$recs_var}=$records)!=null?\${$recs_var}->getCurrent():null){$sep}";
	}
	
	// <ds:foreach sequence="var:myprops" records="#var:sections#" item="section">...</ds:foreach>
	function _foreach_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}
	
	function foreach_($node,$content,$level,$pad)
	{
		// collection
		if (($collection=$node->getAttribute('sequence'))===null &&
			($collection=$node->getAttribute('records'))===null)
			return $this->fatal('foreach : no "records" or "sequence" attribute');
		$collection=$this->parseVal($collection);
		
		// record/item
		if (($item=$node->getAttribute('item'))===null &&
			($item=$node->getAttribute('record'))===null)
			return $this->fatal('foreach : no "record" or "item" attribute');
		
		// index variable?
		if (($index=$node->getAttribute('index'))===null)
		{
			$setIndex='';
			$incrIndex='';
		}
		else
		{
			$setIndex="\n{$pad}\$$index=0;";
			$incrIndex="\n{$pad}\$$index++;";
		}
		
		$iter_var=$this->createVarID('iterRec');
		$recs_var=$this->createVarID('records');
		
		return "\n{$pad}if(\${$recs_var}={$collection}) {{$setIndex}
{$pad}for (	\${$iter_var} = {$collection}->getRecordIterator(); 
{$pad}	\${$iter_var}->isValid(); 
{$pad}	\${$iter_var}->next())
{$pad}{
{$pad}\${$item}=&\${$iter_var}->getCurrent();{$content}
$incrIndex{$pad}}}";
	}
	
	function fields($node,$content,$level,$pad)
	{
		// collection
		if (($collection=$node->getAttribute('sequence'))===null &&
			($collection=$node->getAttribute('record'))===null)
			return $this->fatal('foreach : no "record" or "sequence" attribute');
		$collection=$this->parseVal($collection);

		// field/item
		if (($item=$node->getAttribute('item'))===null &&
			($item=$node->getAttribute('field'))===null)
			return $this->fatal('foreach : no "field" or "item" attribute');

		$fieldIndex=$this->createVarID('fieldIndex');
		$iter_var=$this->createVarID('iterField');
		$rec_var=$this->createVarID('record');
		
		return "\n{$pad}if(\${$rec_var}={$collection}) {
		\$$iter_var = \${$rec_var}->getFieldIterator();
		\$$fieldIndex=0;
		for (\${$iter_var}->reset(); \${$iter_var}->isValid(); \${$iter_var}->next())
		{
			\${$item} = & \${$iter_var}->getCurrent();
			if (!\${$item}->isHidden())
			{
		{$content}
		
			\$$fieldIndex++; //index of non hidden fields
			} /* if */
		}} /* iter field */
";
	}
		
	// 2 interfaces:
	// 1) get field from record
	// <ds:field name="ptype" format="string" default="" record="var:records"/>
	// 
	// 2) create record from data field
	// <ds:record ds="site.boot">
	// 		<ds:field name="name" value="test" format="HTML"/>
	// <ds:record>
	//
	function field($node,$content,$level,$pad)
	{
		/* field name */
		if (($field=$node->getAttribute('name'))==null && 
			($field=$node->getAttribute('alias'))==null)
			return '/* error in ds:field : no "name" or "alias" attribute*/';			
			
		$field=$this->parseVal("'{$field}'");
		
		// format
		$format=$node->getAttribute('format','object');
		$format=$this->parseVal("'{$format}'");		
		
		/* record */
		if ($this->getAttribute($records,$node,'record'))
		{
			if (!preg_match('/[$][^:]/',$records))
				$records = $this->parseQvalue($records);
		}
		/* created from data */
		elseif( $this->getAttribute($value,$node,'value',null,true,true) || (($value=trim($content))!=''))
		{
			return "\n	{$pad}$field=>$value,";
		}		
/*		else if ($content!='')
		{
			$records=$content;
		} 
*/
		else
			$this->fatal('no record attribute');


		// display options
		$this->getAttribute($opts,$node,'options','',true,true);

		// default value
		$this->getAttribute($deft,$node,'default','',true,true);
		
		// instruction or expression ?
		if ($this->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
			
		// if records is a variable, use it directly, otherwise use temp variable
		if (preg_match('/^[$]\w+$/',trim($records)))		
			return "\n{$pad}{$records}->getProperty($field,$deft,false,$format,$opts){$sep}";
		else
			return "\n{$pad}(\$_nx_record={$records})!=null?\$_nx_record->getProperty($field,$deft,false,$format,$opts):{$deft}{$sep}";
	}
	
	// <ds:set_field name="ptype" record="var:records" value="'xxx'" />
	function setField($node,$content,$level,$pad)
	{
		/* field name */
		if (($field=$node->getAttribute('name'))==null && 
			($field=$node->getAttribute('alias'))==null)
			return '/* error in ds:field : no "name" or "alias" attribute*/';			
			
		$field=$this->parseVal("'{$field}'");
		
		/* value */
		$this->getAttribute($value,$node,'value',$content,true,true);

		/* records */
		if (($records=$node->getAttribute('record'))!==null)
		{
			$records = $this->parseQvalue($records);
		}
		else
		{
			$records=$content;
		}
		
		$rec_var=$this->createVarID('rec');
		$recs_var=$this->createVarID('recs');
		
		// return "\n{$pad}if((\$$recs_var=&$records)!=null && \$$rec_var=&\${$recs_var}->getCurrent()) \${$rec_var}->setField($field,$value);";
		return "\n{$pad}if((\$$rec_var=&$records)!=null) \${$rec_var}->setField($field,$value);";
	}
	
	function deleteResources($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($var,$node,'record') &&
			!$this->getAttribute($var,$node,'records'))
			return $this->fatal('no "record" or "records" attribute');
			
		return "\n{$pad}{$var}->onDelete(\$this->error);";
	}

	// <ds:expr record="var:rec" expression="record.url_key" />
	// <ds:expr field="var:f" expression="field.url_key" />
	function expr($node,$content,$level,$pad)
	{	
		if (!$this->getAttribute($expr,$node,'expression'))
			return $this->fatal('no expression attribute');
			
		if (($record=$node->getAttribute('record'))!==null)
		{
			$record = $this->parseQvalue($record);
			$rec_var=$this->createVarID('rec');
			
			$s="((\$$rec_var=$record)!=null?";
			$patterns = array(
				'/record\.url_key/i', // only key fields 
				'/record\.as_url/i', // all fields
				'/record\.nb_fields/i',
				'/record\.lastInsertId/i',
				'/record\.view/i',
				'/record\.table/i',
				'/_nx_r->/'
				);
				
			$replace = array(
				'_nx_r->getUrlKey(false)',
				'_nx_r->getUrlKey(true)',
				'_nx_r->fieldCount()',
				'_nx_r->lastInsertID',
				'_nx_r->recDesc->getTableName()',
				'_nx_r->recDesc->getViewName()',
				"\${$rec_var}->"
				);
			$s.= preg_replace($patterns,$replace,$expr);
			$s.=":'')";
		}
		elseif ($this->getAttribute($fld,$node,'field'))
		{
			$s='/* ds field expr not implemented */';
		}
		else
			return $this->fatal('no record or field attribute');

		return $s;
	}
	
	function saveProperties($node,$content,$level,$pad)
	{
		// get variable to store it to
		if (!$this->getAttribute($to,$node,'to'))
			$this->getAttribute($to,$node,'props','$this');
		
		$to='$'.trim($to,'$');
		
		$s="\n{$pad}if(!isset($to)) $to=new NxProperties(\$p=array(),\$parent=null);";
		
		// get format
		$this->getAttribute($format,$node,'format','HTML',true,true);
		
		// process record
		if ($this->getAttribute($rec,$node,'record'))
		{
			return "$s\n{$pad}{$rec}->saveAsProperties({$to},false,{$format});";
		}
		
		return $this->fatal('no record attribute');	
	}
		
	// ========= functions called from nxl expressions =======
	
	// count records
	// <set name="cnt" value"#ds:count($records)#" />
	function _func_count($node,$args,$level=0)
	{
		return "(($args[0]==null)?0:{$args[0]}->count())";
	}

}

?>
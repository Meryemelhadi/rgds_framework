<?PHP

// core library for DML language that generates all data descriptors and SQL requests.
// This layer implements inheritance (data models) and "modes" (data subsets used by applications,
// with key and field lists).
/*
    fy: added database management: when specified in data, tables, views, defer it to table or view
*/

class taglib_nxfrontier_dml_dml extends NxTagLib
{
	var $queries = array();

	function __construct(&$context) {
		parent::__construct($context);
	}

	// default handlers
	function _tag_default($node,$level)
	{
		return true;
	}
	function tag_default($node,$content,$level,$pad,$ns,$op)
	{
		return $this->passThrough($node,$content,$pad);
	}
	
	// default handler
	function comment_($node,$content,$level,$pad)
	{
		if (@$content[0]!='[')
			return '';
			
		if ($this->getAttribute($v,$node,'value'))
			$content=$v.trim($content,'#!|[]');
		
		return "\n{$pad}<!-- {$content} -->";		
	}
	
	
	function _data(&$node,&$level)
	{
		$this->init='';
		$this->getAttribute($this->comp,$node,'component',$this->getProperty('component','data'));

		// decode package
		$this->package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($this->package,'package');				

        // get database if specified
        $db=$this->context->getLastResource('db');
        $this->getAttribute($db,$node,'database',$db);
        if ($db)
            $this->context->addResource($db,'db');         

        $this->getAttribute($config,$node,'config',null);
        $this->context->addResource($config,'config');                

		$level=0;
		return true;		
	}
		
	/*	todo: read DB handler from data tag and/or from site_config and add it to data tag for 2nd pass.
		this handler - "MYSQL" by default - maps high level data types to SQL types. */
	function data($node,$content,$level,$pad)
	{
		// get currenty package (and pop package resource)
		$this->package=$this->context->getLastResource('package',true);

		// get current config (and pop package resource)
		$this->context->getLastResource('config',true);
		
		if ($content=='')
			return '';
			
		// generate DML source
		$src=$this->passThrough($node,$content,$pad);
		
		// save source for debug
		$this->context->saveSrc(NX_GEN."content/data/{$this->package}/{$this->comp}.xml",$src);

		// Call DML parser/generator:
		// compile record descriptors and SQL requests
		// NB. this second pass requires detailed analysis of fields for generating queries,
		// and will complete fields were needed. It hasnt been implemented as a NXL taglib
		// because when it was created the NXL framework wasnt powerful enough to handle this
		// complexity.
		// TODO: implement second pass as standard NXL lib..
		include_once(NX_LIB.'mvc/DML/DML_processor.inc');
		$processor= new DML_processor($this->props);
		$processor->processXML($src,$this->comp,$this->package);

		// perform actions if requested: create DB
		if ($this->init && $this->comp)
		{
			$actions = explode(',',$this->init);
			$this->props->setProperty($this->comp.'.db',$this->comp);
            if ($this->package)
                $packageSuf='@'.$this->package;
            else
                $packageSuf='';
            
			
			// $db = $this->props->getPlugIn('DB');
			require_once(NX_LIB."ds/db/NxDB_Database.inc");
			$errorCB = new NxErrorCB();
			$db = new NxDB_Database($this->props,$errorCB);
			if ($errorCB->isError())
				nxltrace("Cant connect to database:".$errorCB->getErrorMsg(),'compil');
			else
			{
				foreach ($actions as $action)
				{
					switch(strtolower($action))
					{
						case 'drop':
							// delete tables
							// $errorCB = new NxErrorCB();
							if( $db->executeDBQuery($this->comp.$packageSuf,"deleteSQLTables",$errorCB,true) )
							{
								$msg = "Tables for component ".$this->comp.$packageSuf." were successfully deleted";
							}
							else
							{
								// error: database error
								$msg = "Deletion of tables for component ".$this->comp.$packageSuf." failed : " . $errorCB->getErrorMsg();
							}
							break;
						case 'create':
							// create tables
							// $errorCB = new NxErrorCB();
							if( $db->executeDBQuery($this->comp.$packageSuf,"createSQLTables",$errorCB,true) )
							{
								$msg = "Tables for component ".$this->comp.$packageSuf." were successfully created";
							}
							else
							{
								// error: database error
								$msg = "Creation of tables for component ".$this->comp.$packageSuf." failed : " . $errorCB->getErrorMsg();
							}
							break;
					}
					nxltrace("DML(1) SQL table: $msg",'DML');
				}
			}
			$this->init='';
		}
		
        // pop database if specified
        $this->getAttribute($db,$node,'database',null);
        if ($db)
            $this->context->getLastResource('db',true);

		nxltrace("DML(1) \n".htmlentities($src),'debug');
		// echo "DML=\n".htmlentities($src);

		
		if(count($this->queries)>0)
		{
			nxltrace("DML(3) : custom queries\n",'DML');
			foreach ($this->queries as $f=>$query)
			{
				list($name,$code)=$query;
				$code = $this->mapCode($name,$code);
				$code = $this->cleanSQLSrc($code);
				$this->saveFile($f,$code);
			}
			$this->queries=array();
		}
		
        // pop database if specified
        $this->getAttribute($postProcessorURI,$node,'post_processor_uri',null);
        if ($postProcessorURI)
        {
			// map lib for polymorphism with "this" ns mapped to current taglib
			//$store_lib=$this->context->mapURI('http://www.nxfrontier.com/dml/dml',$postProcessorURI); /* -REF */

			$src2=<<<EOH
<xo:map_uri target="{$postProcessorURI}" 
uri="http://www.nxfrontier.com/dml/dml" xmlns:xo="http://www.nxfrontier.com/xo/core"
xmlns="http://www.nxfrontier.com/dml/dml">
$src
</xo:map_uri>
EOH;
			nxltrace("DML($postProcessorURI) ============= \n",'DML');
			
        	$out = $this->transformNXL($src2);
			nxltrace("DML($postProcessorURI) \n".htmlentities($out),'DML');

			// pop lib
			//$this->context->restoreLibUri('http://www.nxfrontier.com/dml/dml',$store_lib);
		}
	}
	
	function transformNXL($src)
	{	
		include_once(NX_LIB.'mvc/NXL/NXL_processor.inc');

		$GLOBALS['nxl_indent']=0;
		$GLOBALS['nxl_pad']='';
		$GLOBALS['nxl_error_number']=0;
						
		$xml_parser = new NXL_processor($this->props);
		$xml_parser->genSrc($src);
		$src=$xml_parser->buildOutput();
		
		return $src;
	}
	
	// model for tables: allows inheritance of keys and fields for tables and views	
	function models($node,$content,$level,$pad)
	{		
		$this->getAttribute($comp,$node,'component',$this->comp);
		$this->context->saveSrc(NX_GEN."content/data/models/{$this->comp}.xml",
			$this->passThrough($node,$content,$pad));
			
		return '';
	}
		
	// apply parent model?
	function _model($node)
	{
		return $this->inheritModel($node);
	}
	
	// model for table/views (cuurently support keys and fields)
	function model($node,$content,$level,$pad)
	{
		// inherit model from other model?
		$parent_model=$this->model;

		if ($this->getAttribute($name,$node,'name',''))
		{
			$model = array( 'keys'=>$this->context->getResources('keys',null,true)+$parent_model['keys'],
							'fields'=>$this->context->getResources('fields',null,true)+$parent_model['fields'],
							'sort'=>$this->context->getResources('sort',null,true)+$parent_model['sort'],
							'group_by'=>$this->context->getResources('group_by',null,true)+$parent_model['group_by'],
							'pages'=>$this->context->getResources('pages',null,true)+$parent_model['pages'],
							'queries'=>$this->context->getResources('queries',null,true)+$parent_model['queries'],
							);
							
			// add model that can be inherited by views and tables
			$this->context->addResource($model,'model',$name);
			nxltrace("Added DML model : $name","trace");

		}
		
		// reset inherited model
		$this->model=null;
		
		foreach ($model['keys'] as $mode=>$key)
		{
			$content .= "\n{$pad}		$key";
		}
		foreach ($model['sort'] as $mode=>$sort)
		{
			$content .= "\n{$pad}		$sort";
		}
		foreach ($model['group_by'] as $mode=>$sort)
		{
			$content .= "\n{$pad}		$sort";
		}
		foreach ($model['pages'] as $mode=>$p)
		{
			$content .= "\n{$pad}		$p";
		}
		foreach ($model['queries'] as $mode=>$p)
		{
			$content .= "\n{$pad}		$p";
		}
		foreach ($model['fields'] as $mode=>$fields)
		{
			$content .= "\n{$pad}	<fields mode=\"$mode\">$fields\n{$pad}	</fields>";
		}
		
		return $this->passThrough($node,$content,$pad);
	}
	
	function _tables($node,$level)
	{	
		$this->getAttribute($this->comp,$node,'component',$this->comp);

        // get database if specified
        $db=$this->context->getLastResource('db');
        $this->getAttribute($db,$node,'database',$db);
        if ($db) {
            $this->context->addResource($db,'db');
            $node->setAttribute('database',$db);
        }

		return true;
	}
	
	function tables($node,$content,$level,$pad)
	{
		if (!isset($_GET['init']))
			$_GET['init']='';
			
        // pop database if specified
        $this->context->getLastResource('db',true);

		$this->init=$node->getAttribute('init',$_GET['init']);
		return $this->passThrough($node,$content,$pad,true);
	}	
	
	function _table($node)
	{
        // get database if specified
        $db=$this->context->getLastResource('db');
        $this->getAttribute($db,$node,'database',$db);
        if ($db) {
            $this->context->addResource($db,'db');
            $node->setAttribute('database',$db);
        }

		$this->getAttribute($name,$node,'name','?');
		$this->context->addResource($name,'view');
		$this->context->addResource($name,'table');

		return $this->inheritModel($node);
	}
	
	function table($node,$content,$level,$pad)
	{
        // pop database if specified
        $this->context->getLastResource('db',true);
        $this->context->getLastResource('view',true);
        $this->context->getLastResource('table',true);

        $config=$this->context->getLastResource('config');
        $this->getAttribute($config,$node,'config',$config);
		if($config)
			$node->setAttribute('config',$config);

		return $this->genView($node,$content,$level,$pad,$config);
	}	
	
	function _view($node)
	{
        // get database if specified
        $db=$this->context->getLastResource('db');
        $this->getAttribute($db,$node,'database',$db);
        if ($db) {
            $this->context->addResource($db,'db');
            $node->setAttribute('database',$db);
        }

		$this->getAttribute($name,$node,'name','?');
		$this->context->addResource($name,'view');
        $this->getAttribute($table,$node,'table',$name);
		$this->context->addResource($table,'table');

		return $this->inheritModel($node);
	}
	
	function view($node,$content,$level,$pad)
	{		
        // pop database if specified
        $this->context->getLastResource('db',true);
        $this->context->getLastResource('view',true);
        $this->context->getLastResource('table',true);

        $config=$this->context->getLastResource('config');
        $this->getAttribute($config,$node,'config',$config);
		if($config)
			$node->setAttribute('config',$config);

		return $this->genView($node,$content,$level,$pad,$config);
	}
	
	// check inheritance and set up model to be followed by children nodes
	// NB. implements a multi inheritance system.
	// ex. <table name="user" inherit="login;register" >...</table>
	function inheritModel($node)
	{
		$this->model=array('keys'=>array(),'fields'=>array(),'sort'=>array(),'group_by'=>array(),'pages'=>array(),'queries'=>array());		
		$nodestr='&lt;'.$node->localName." name=".$node->getAttribute("name").'&gt;';

		if (!$this->getAttribute($inherit,$node,'inherit') &&
			!$this->getAttribute($inherit,$node,'extends'))
			return;
			
		nxltrace("DML models to apply : $inherit to ".$nodestr,"trace");
		
		$modelNames=explode(';',strtr($inherit,',',';'));
		foreach ($modelNames as $modelName)
		{
			$model=$this->context->getResource('model',$modelName);
			if (!isset($model))
				nxltrace("Error cant find DML model : $modelName for : $nodestr",'error');
			else
				nxltrace("Apply DML &lt;model name=\"$modelName\" /&gt; &lt;&lt;--  $nodestr","trace");
							
			if (isset($model['keys']))
				$this->model['keys']+=$model['keys'];

			if (isset($model['queries']))
				$this->model['queries']+=$model['queries'];

			if (isset($model['fields']))
				$this->model['fields']+=$model['fields'];
			
			if (isset($model['sort']))
				$this->model['sort']+=$model['sort'];
			
			if (isset($model['group_by']))
				$this->model['group_by']+=$model['group_by'];
			
			if (isset($model['pages']))
				$this->model['pages']+=$model['pages'];
		}
	}
	
	/* utility function for generating table/view, in various modes */
	function genView($node,$content,$level,$pad,$config)
	{
		$this->getAttribute($name,$node,'name','?');
        $this->getAttribute($table,$node,'table',$name);
        $this->getAttribute($db,$node,'database','');
        $this->getAttribute($aspect,$node,'aspect','');
        $this->getAttribute($handler,$node,'handler','');
        $this->getAttribute($class,$node,'class','');
        $this->getAttribute($extendsdWithDs,$node,'extendsdWithDs','');
		
		$tag=$node->localName;
		$model=$this->model;
		$this->model=null;
				
		// get fields from current view (model already applied in fields tag) in all modes
		$fields=$this->context->getResources('fields',null,true);
		
		// complement with other model modes
		if (count($model['fields'])>0)
			$fields +=$model['fields'];
		
		if (isset($fields['default']))
			$fDeft=$fields['default'];
		else		
			$fDeft='';

		if (isset($fields['*']))
			$content="{$fields['*']}\n{$pad}	</fields>$content";
		else
			$content="\n{$pad}	</fields>$content";

		nxltrace("DML $tag: [$name]",'DML');
		
		// get sort fields (from current view and from model) in all modes
		$sorts=$this->context->getResources('sort',null,true);
		if (isset($sorts['default']))
			$content .=$sorts['default'];
				
		if (count($model['sort'])>0)
			$sorts +=$model['sort'];

		// get query (from current view and from model) in all modes
		$queries=$this->context->getResources('queries',null,true);
		if (isset($queries['default']))
			$content .=$queries['default'];
				
		if (count($model['queries'])>0)
			$queries +=$model['queries'];

		// get group_by fields (from current view and from model) in all modes
		$group_bys=$this->context->getResources('group_by',null,true);
		if (isset($group_bys['default']))
			$content .=$group_bys['default'];
				
		if (count($model['group_by'])>0)
			$group_bys +=$model['group_by'];

		// get pages in all modes
		$pages=$this->context->getResources('pages',null,true);
		if (count($model['pages'])>0)
			$pages +=$model['pages'];
			
		if (isset($pages['default']))
			$content .=$pages['default'];
		
		// get keys (from current view and from model) in all modes
		$keys=$this->context->getResources('keys',null,true);
		
		if (count($model['keys'])>0)
			$keys +=$model['keys'];
		
		if (count($keys)==0)
		{
			// no key
			//return $this->passThrough($node,"{$pad}	<fields>$content",$pad);
		}
					
		// default key ?
		if (isset($keys['default']))
		{
			$src=$this->passThrough($node,"{$keys['default']}\n{$pad}	<fields>\n{$fDeft}$content",$pad,true);
			unset($keys['default']);
		}
		else
			$src=$this->passThrough($node,"\n{$pad}	<fields>\n{$fDeft}$content",$pad,true);

		// generate views:
		//  - list all modes from keys and fields sets
		$modes=array_keys($keys+$fields+$queries);
		
		// create a view for each mode
		foreach ($modes as $mode)
		{
			if ($mode=='*' || $mode=='default')
				continue;
				
			$vname="{$name}.{$mode}";
			
			// key
			if (isset($keys[$mode]))
				$key=$keys[$mode];
			else
				$key='';
			
			// sort
			if (isset($sorts[$mode]))
				$sort=$sorts[$mode];
			else
				$sort='';
				
			// queries
			if (isset($queries[$mode]))
				$quer=$queries[$mode];
			else
				$quer='';

			// group_by
			if (isset($group_bys[$mode]))
				$group_by=$group_bys[$mode];
			else
				$group_by='';
				
			// pages
			if (isset($pages[$mode]))
				$page=$pages[$mode];
			else
				$page='';
			
			// fields
			if (isset($fields[$mode]))
				$f=$fields[$mode];
			else
				$f=$fDeft;
                
            $dbatt='';
            
            if ($db)
                $dbatt=" database=\"$db\"";
                
            if ($aspect)
			{
                $dbatt.=" aspect=\"$aspect\"";
			}
				
            if ($handler)
                $dbatt.=" handler=\"$handler\"";
				
            if ($class) {
                $dbatt.=" class=\"$class\"";
			}
            if ($extendsdWithDs) {
                $dbatt.=" extendsdWithDs=\"$extendsdWithDs\"";
			}

            if ($config) {
                $dbatt.=" config=\"$config\"";
			}
				
			$src.=$v="\n{$pad}<view name=\"$vname\"$dbatt table=\"$table\">\n{$pad}	{$key}\n{$pad}	{$sort}\n{$pad}	{$quer}\n{$pad}	{$group_by}\n{$pad}	{$page}\n{$pad}	<fields>{$f}\n{$content}\n{$pad}</view>\n";
			nxltrace("DML(1) view: [$vname]",'DML');
		}
		
		return $src;
	}
	
	function keys($node,$content,$level,$pad)
	{
		// apply model and return the whole result
		return $this->passThrough($node,$this->model['keys'].$content,$pad);
	}	
	
	// keys: if no mode => default value (overloaded by others)
	function key($node,$content,$level,$pad)
	{
		if ($content)
			$src=$this->passThrough($node,$content,$pad);
		else
			// if no field in keys => leave it empty
			$src='';
			
		$mode=$node->getAttribute('mode','default');
		$this->context->addResource($src,'keys',$mode);
		return '';		
	}
	
	// keys: if no mode => default value (overloaded by others)
	function sort($node,$content,$level,$pad)
	{
		if ($content)
			$src=$this->passThrough($node,$content,$pad);
		else
			// if no field in keys => leave it empty
			$src='';
			
		$mode=$node->getAttribute('mode','default');
		$this->context->addResource($src,'sort',$mode);
		return '';		
	}

	// keys: if no mode => default value (overloaded by others)
	function queries($node,$content,$level,$pad)
	{
		if ($content)
			$src=$this->passThrough($node,$content,$pad);
		else
			// if no field in keys => leave it empty
			$src='';
			
		$mode=$node->getAttribute('mode','default');
		$this->context->addResource($src,'queries',$mode);
		return '';		
	}

	// keys: if no mode => default value (overloaded by others)
	function group_by($node,$content,$level,$pad)
	{
		if ($content)
			$src=$this->passThrough($node,$content,$pad);
		else
			// if no field in keys => leave it empty
			$src='';
			
		$mode=$node->getAttribute('mode','default');
		$this->context->addResource($src,'group_by',$mode);
		return '';		
	}
	
	// paging limit: if no mode => default value (overloaded by others)
	function pages($node,$content,$level,$pad)
	{
		$src=$this->passThrough($node,'',$pad);			
		$mode=$node->getAttribute('mode','default');
		$this->context->addResource($src,'pages',$mode);
		return '';		
	}
	
	function _fields()
	{
		$this->fields=null;
	}
	
	// Fields:
	//	complete set with one from model
	//  if no mode => apply to all (*) modes (complete existing field sets)
	function fields($node,$content,$level,$pad)
	{
		$mode=$node->getAttribute('mode','default');
		
		// is there a field set in model for this mode ?
		if (@isset($this->model['fields'][$mode]))
		{
			// yes => apply model
			$content = $this->model['fields'][$mode].$content;
		}
		
		// apply field set for all modes (except if already done when current mode is '*')
		if ($mode!='*' && @isset($this->model['fields']['*']))
		{
			// yes => apply model
			$content = $this->model['fields']['*'].$content;
		}
		
		// store fields by mode key
		$this->context->addResource($content,'fields',$mode);
				
		return '';
	}
	
	function _group($node)
	{
	}
	
	function group($node,$content,$level,$pad)
	{		
		return $this->passThrough($node,$content,$pad,true);
	}

	function or_($node,$content,$level,$pad)
	{		
		static $where_op=0;
		
		return '<field type="where_op" name="where_or_'.($where_op++).'" is_transient="true" nary="2" op="OR" action="start"/>'.
			$this->passThrough($node,$content,$pad,true).
			'<field type="where_op" is_transient="true" nary="2" name="where_or_'.($where_op++).'" op="OR" action="end"/>';
	}
	function and_($node,$content,$level,$pad)
	{		
		static $where_op=0;
		
		return '<field type="where_op" is_transient="true" nary="2" name="where_and_'.($where_op++).'" op="AND" action="start"/>'.
			$this->passThrough($node,$content,$pad,true).
			'<field type="where_op" is_transient="true" nary="2" op="AND" name="where_and_'.($where_op++).'" action="end"/>';
	}

	function not_($node,$content,$level,$pad)
	{		
		static $where_op=0;
		
		return '<field type="where_op" is_transient="true" nary="1" name="where_not_'.($where_op++).'" op="NOT" action="start"/>'.
			$this->passThrough($node,$content,$pad,true).
			'<field type="where_op" is_transient="true" nary="1" op="NOT" name="where_not_'.($where_op++).'" action="end"/>';
	}

	function field($node,$content,$level,$pad)
	{
		$type=$node->getAttribute('type','');
		$method="field_".preg_replace('/[-.]/','_',$type);
		if (method_exists($this,$method))
		{
			return $this->$method($node,$content,$level,$pad);
		}
		else
		{
			return $this->passThrough($node,$content,$pad,true);
		}
	}
	
	function help($node,$content,$level,$pad)
	{
		return $this->passThrough($node,$content,$pad);
		/*
		if (($field=&$this->context->getParentElement($node))!=null && ($field->localName=='field'))
		{
			$field->setAttribute('help',preg_replace(
				array('/"/',"/'/"),array('&quot;','&rsquo;'),$content));
		}*/
	}

	function field_text_choice($node,$content,$level,$pad)
	{
		//$name=$node->getAttribute('name','');
		//$node->setDeftAtt('list',preg_replace('/[-.]/','_',$name));
		$src=$this->passThrough($node,$content,$pad,true);
		nxltrace("field result:$src\n",'debug');
		return $src;
	}
	
	function query($node,$content,$level,$pad) {
		$this->getAttribute($name,$node,'name');
		$this->getAttribute($mode,$node,'mode');
		$this->getAttribute($type,$node,'type','select');

		$s='$properties["query"]="'.$content.'";';

		$s=preg_replace(
			array(
				'/\{(?:PAGES|LIMIT):([0-9]+)\}\s*";$/i',
				'/\{(?:PAGES|LIMIT):([a-z0-9_]+)\}\s*";$/i',
				'/\{(?:PAGES|LIMIT):([a-z0-9_]+),([0-9]+)\}\s*";$/i',
				'/\{(?:PAGES|LIMIT)\}\s*";$/i',
			),
			array(
			'";$properties["pageOffset"]=0+(isset($_GET["page"])?$_GET["page"]:"0");
		$properties["pageAlias"]=\'page\'; 
		$properties["nbRecords"]=0+\'$1\'; 
		$properties["query"] .= " LIMIT ".(string)($properties["nbRecords"]*$properties["pageOffset"]).",".$properties[\'nbRecords\'];'
		,
			'";$properties["pageOffset"]=0+(isset($_GET["page"])?$_GET["page"]:"0");
		$properties["pageAlias"]=\'page\'; 
		$properties["nbRecords"]=\$props->getProperty(\'$1\',\'20\'); 
		$properties["query"] .= " LIMIT ".(string)($properties["nbRecords"]*$properties["pageOffset"]).",".$properties[\'nbRecords\'];'
		,
			'";$properties["pageOffset"]=0+(isset($_GET["page"])?$_GET["page"]:"0");
		$properties["pageAlias"]=\'page\'; 
		$properties["nbRecords"]=\$props->getProperty(\'$1\',\'$2\'); 
		$properties["query"] .= " LIMIT ".(string)($properties["nbRecords"]*$properties["pageOffset"]).",".$properties[\'nbRecords\'];'
		,
		'";$properties["pageOffset"]=0+(isset($_GET["page"])?$_GET["page"]:"0");
		$properties["pageAlias"]=\'page\'; 
		$properties["nbRecords"]=0+\'20\'; 
		$properties["query"] .= " LIMIT ".(string)($properties["nbRecords"]*$properties["pageOffset"]).",".$properties[\'nbRecords\'];')
		,$s);

		$s='<?php '.$s.' ?>';

		// set file root (from package)
		$package = $this->package;
		if ($package==''||$package=='root')
			$fileRoot=NX_CONF.'db/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES!='')
			$fileRoot=GENERIS_PACKAGES.$package.'/conf/db/';
		else
			$fileRoot=NX_CONF.'packages/'.$package.'/db/';

		$view = $this->context->getLastResource('view',false);
		if ($mode)
			$view .= '.'.$mode;

		$filePath=$fileRoot."{$view}_{$type}Record.inc";
		$this->queries[$filePath]=array($view,$s);

		$src=htmlentities($s);		
		nxltrace("found SQL QUERY: $filePath:<br/>\n$src",'listing');
	}

	function mapCode($view,$code) {
		return preg_replaceX(
			array(
				'/\{(where|group.by|sort|limit)\}/ie'),
			array('\'"\'.$this->getProperty("'.$view.'.".strtolower("$1")).\'\''),
				$code,$this);

	}

	function cleanSQLSrc($s) {
		return preg_replace(
			array(
				'/[.]["\']["\'][.]/',
				'/["\'][.]["\']/',
				'#"\."#',
				'#_>#',
				'#."";#'
				),
			array('.','','','->',';'),
			$s
			);
	}

	function saveFile($path,$src) {
		$fs=$this->props->getPlugIn('FileSys');
		$fs->saveFile($path,$src);
		$src=htmlentities($src);		
		nxltrace("SQL QUERY (CUSTOM): $path:<br/>\n$src",'listing');
	}

}

?>
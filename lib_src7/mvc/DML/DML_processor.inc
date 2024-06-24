<?PHP
define('NX_LIB_DML',NX_LIB.'mvc/DML/');

include_once('DML_parserDML.inc');
include_once('DML_fieldMap.inc');
include_once('DML_SqlMap.inc');
include_once('DML_genRecDesc.inc');
include_once('DML_genSQL.inc');


/*
DML creates SQL queries and record descriptors to be used by NxField API and records.

TYPES:
It maps high level types of fields to:
1) NX API types: each of this type requires a class NxField_[type].inc
2) SQL high level types which describe a generic layer of SQL types.
	These types are higher level than actual SQL (ex.. "file").
	They are transformed into SQL types by each DB driver (ex. MySQL).
	
FIELD NAMES:
field names exist in several forms:
- original field name: ex. account-info.
- formated name: "account_info". When part of a "group", prefixed by [group]_, as in
	user_account_info
- sql name: "_account_info" (prefix means any field name can be used in DML, 
	even "select")	
- qualified sql name: _customer._acount_info (ie. includes table name),
	used in multi table queries such as joints.
	
ALIASES:
NX API supports look ups to fields through aliases:
- actual field "alias",
- property name: using the record as a property bag
- role: (obsolete)

*/
class DML_processor
{
	var $xml_parser;
	var $tables;
	var $views;
	
	function __construct(&$props) 
	{
		$this->props=&$props;
		$this->tables=array();
		$this->views=array();
		$this->fs=$props->getPlugIn('FileSys');
	}
		
	function build($comp,$path)
	{
		if(($src=$this->file_get_contents($path))===null)
		{
			echo "cant find XML doc";
			return false;
		}
			
		return $this->processXML($src,$comp);
	}
		
	function processXML($src,$comp,$package) {	
		// component (for grouping table creation/deletion)
		$this->comp=$comp;
		
		// read DML source
		$this->parseDML($src);
		
		// complete tables and views with table field attributes
		// inherit from table fields, keys inherit from fields and table.
		$this->completeTables();
		$this->completeViews();
		
		// read DML field mapping
		$this->readFieldMaps(NX_LIB_DML.'data/fieldMaps.xml');

		// read SQL field mapping
		// NB. read sql driver name from data def or config ?
		$this->readSQLMaps(NX_LIB_DML.'data/MySQL.xml');

		// remap table fields
		$this->tables=& $this->mapTypes($this->tables);
		$this->views =& $this->mapTypes($this->views);
		
		//  SQL mapping
		// $this->tables=& $this->mapSQL($this->tables);
		// $this->views =& $this->mapSQL($this->views);
		
		// generate record descriptors
		nxltrace('DML(2): generate recDesc for tables','DML');
		$this->genRecDesc($this->tables,$package);	

		nxltrace('DML(2): generate recDesc for views','DML');
		$this->genRecDesc($this->views,$package);	

		// generate SQL
		$this->genSQL($package);
	}
	
	function toPHP() {
		var_export($this->views);
	}
	
	// obsolete?
	function readDML($path) 
	{
		$this->xml_parser = new DML_parserDML();
		$this->xml_parser->parseFile($path);

		$this->tables=$this->xml_parser->tables;
		$this->views=$this->xml_parser->views;

		$this->xml_parser->close();
		$this->xml_parser=null;		
	}

	function parseDML($src) 
	{
		$this->xml_parser = new DML_parserDML();
		$this->xml_parser->parseXML($src);

		$this->tables=$this->xml_parser->tables;
		$this->views=$this->xml_parser->views;

		$this->xml_parser->close();
		$this->xml_parser=null;		
	}
	
	/* read field type description.
	NB. DML field types are mapped onto "SQL" types and "Server" field types.
	- "SQL" types are high levels types (such as image, file, text, text-choice, etc. that
		 are later mapped onto actual SQL types
	- "Server" field types are classes provided by the nx lib in lib/data/fields/NxDataField_xxx.php, and
		they handle forms, html, string operations.
	*/
	function readFieldMaps($path) {
		$this->xml_parser = new DML_fieldMap();
		$this->xml_parser->parseFile($path);

		$this->maps=$this->xml_parser->maps;

		$this->xml_parser->close();
		$this->xml_parser=null;		
	}
	
	/* read SQL map.
		maps high level types to SQL types, depending on current handler (MYSQL or else)
	*/
	function readSQLMaps($path) {
		$this->xml_parser = new DML_SqlMap();
		$this->xml_parser->parseFile($path);

		$this->sqlMaps=$this->xml_parser->maps;

		$this->xml_parser->close();
		$this->xml_parser=null;		
	}
	
	// get type mappings and apply them to tables and views
	function & mapTypes($tables) 
	{		
		$tables2=array();
		
		// for each table
		foreach($tables as $tname=>$table)
		{		
			
			$tfields=$table['fields'];
			
			// add new list of fields
			$tables2[$tname]=array('rprops'=>$table['rprops'],'fields'=>$this->mapTypeFields($tfields));
			
			// add keys
			if (isset($table['key']))
				$tables2[$tname]['key']=
					$this->mapTypeFields($table['key'],false);
			
			// add sorting
			if (isset($table['sort']))
				$tables2[$tname]['sort']=
					$this->mapTypeFields($table['sort']);

			// add group by
			if (isset($table['group_by']))
				$tables2[$tname]['group_by']=
					$this->mapTypeFields($table['group_by']);

			// add paging
			if (isset($table['pages']))
				$tables2[$tname]['pages']=$table['pages'];
				
			// add record limit
			if (isset($table['limit']))
				$tables2[$tname]['limit']=$table['limit'];
		}
		
		return $tables2;
	}
	
	function mapTypeFields($tfields,$uniqFlds=true)
	{
		$maps=&$this->maps;
		$sqlMaps=&$this->sqlMaps;
		$fields=array();
		
		// for each field
		foreach($tfields as $field)
		{
			// get field name
			$name=$field['NAME'];
			$qname=$field['QName'];

			// remap field by adding up properties from map
			if (isset($field['TYPE']))
				$type=$field['TYPE'];
			else
				$type=null;
			
			$field2=array();
			$field2['type']=$type;
//			$field2['type']=$field['TYPE'];
			
			// check if DML map exists
			if (isset($maps[$type]))
			{
				$mapF=$maps[$type];
				
				// add up field attributes to current field
				$fserver=$mapF['server'];
				$field2['server']=$field + $fserver;
				
				// replace type by (PHP) mapped one
				if (isset($fserver['TYPE']))
					$field2['server']['TYPE']=$fserver['TYPE'];
								
				// add up sql def
				$field2['sql']=$mapF['sql'];
				
				if (!isset($mapF['sql']['TYPE']))
					$field2['sql']['TYPE']=$field2['type'];

				if (isset($field['SIZE']))
					$field2['sql']['SIZE']=$field['SIZE'];
					
				/*
				if ($field2['sql']['TYPE']=='text' && isset($field2['sql']['SIZE']))
				{
					$size=$field2['sql']['SIZE'];
					if ($size<256)
					{
						$field2['sql']['TYPE']="VARCHAR($size)";
					}
				}
				*/
			}
			else
			{
				$field2['server']=$field;
				$field2['sql']=array('TYPE'=>$type);
			}
			
			// overload SQL size by server max size
			if (isset($field2['server']['MAXSIZE']))
				$field2['sql']['SIZE']=$field2['server']['MAXSIZE'];
				
			if (isset($field2['server']['MULTIPLE']))
				$field2['sql']['MULTIPLE']=$field2['server']['MULTIPLE'];
			
			// check if SQL map exists
			$sqlType=$field2['sql']['TYPE'];
			
			// translate static size "text" to "chars"
			if ($sqlType=='text' && isset($field2['sql']['SIZE']) && 0+$field2['sql']['SIZE']<=255)
				$sqlType='chars';
				
			// remap sql type
			if (isset($sqlMaps[$sqlType]))
			{
				// found a sql map => overload SQL attributes
				$field2['sql'] = $sqlMaps[$sqlType]+$field2['sql'];
			}			
			else
			{
				$fname=$field['QName'];
				if (preg_match('/%\{[^\}]+\}/',$fname))
					nxltrace("DML(2): dynamic field [$fname] => cant apply inherited properties", 'warning');
				else
					nxltrace("DML(2): unknown SQL field type [$sqlType] for field [$fname]", 'error');
			}

			// add new field
			if ($uniqFlds)
				$fields[$qname]=$field2;
			else
				$fields[]=$field2;
		}
		
		return $fields;
	}
	
	function completeTables() 
	{
		$views=array();
		
		// add table parameters to views (inheritance)
		foreach($this->tables as $vname=>$view)
		{
			nxltrace("DML(2) complete table {$view['NAME']}",'DML');
			
			// inherit keys from field list or table
			if (isset($view['key']))
			{
				nxltrace("DML(2) - complete key",'DML');
				$view['key']=$this->completeFields($vname,$view['key'],$view['fields'],null,null,false);
			}
			
			// inherit sort from field list or table
			if (isset($view['sort']))
			{
				nxltrace("DML(2) - complete sort",'DML');
				$view['sort']=$this->completeFields($vname,$view['sort'],$view['fields'],null,null);
			}

			// inherit sort from field list or table
			if (isset($view['group_by']))
			{
				nxltrace("DML(2) - complete group_by",'DML');
				$view['group_by']=$this->completeFields($vname,$view['group_by'],$view['fields'],null,null);
			}
			
			$views[$vname]=$view;
		}
		
		$this->tables=$views;
	}
	
	function completeViews() 
	{
		$views=array();
		$tables=&$this->tables;
	
		// add table parameters to views (inheritance)
		foreach($this->views as $vname=>$view)
		{
			nxltrace("DML(2) complete view {$view['NAME']}",'DML');

			// get default table
			$dftTableName=$view['TABLE'];
				
			nxltrace("DML(2) - complete fields",'DML');
			// inherit fields from table			
			$view['fields']=$this->completeFields($vname,$view['fields'],null,$dftTableName,$tables,true,true);
			
			// inherit keys from field list or table
			if (isset($view['key']))
			{
				nxltrace("DML(2) - complete key",'DML');
				$view['key']=$this->completeFields($vname,$view['key'],$view['fields'],$dftTableName,$tables,false,false);
			}
			
			// inherit sort from field list or table
			if (isset($view['sort']))
			{
				nxltrace("DML(2) - complete sort",'DML');
				$view['sort']=$this->completeFields($vname,$view['sort'],$view['fields'],$dftTableName,$tables,false,false);
			}
			
			// inherit sort from field list or table
			if (isset($view['group_by']))
			{
				nxltrace("DML(2) - complete group_by",'DML');
				$view['group_by']=$this->completeFields($vname,$view['group_by'],$view['fields'],$dftTableName,$tables,false,false);
			}

			$views[$vname]=$view;
		}
		
		$this->views=$views;
	}
	
	/* complete collection with other field collection (table or body)*/
	function completeFields($vname,$fColl,$fields,$dftTableName=null,$tables=null,$uniqFlds=true,$needType=false)
	{
		$kfields=array();
		foreach($fColl as $field)
		{	
			/*
			if(isset($field['group']))
				DebugBreak();
			*/

			// get field name
			$name=$field['NAME'];
			$qname=$field['QName'];
			//nxltrace("DML(2) --- complete field $name",'DML');
			//foreach ($field as $p=>$v)	nxltrace("DML(2) ------ field[$p] = $v",'DML');
			
			//nxltrace("DML(2) --- fields reference",'DML');
			//foreach ($fields as $p=>$v) nxltrace("DML(2) ------ fields[$p] = $v",'DML');
				
			// complete with field set if any
			if (isset($fields[$qname]))
				$f=$field+$fields[$qname];
			elseif($dftTableName!=null && $tables!=null)
			{
				// or pick fields from tables
				
				// get field table
				// if (isset($f['server']['TABLE']))	$t=$f['server']['TABLE'];
				if (isset($field['TABLE']))	
					$t=$field['TABLE'];
				else
					$t=$dftTableName;
					
			//nxltrace("DML(2) --- tables[fields] reference",'DML');
			//foreach ($tables[$t]['fields'] as $p=>$v) 
				//nxltrace("DML(2) ------ tables[$t][fields][$p] = $v",'DML');
				
				// try to find the field in the table	
				if (isset($tables[$t]['fields'][$qname]))
				{
					$f=$field+$tables[$t]['fields'][$qname];
				}
				else 
				{
					if ($needType)
						nxltrace("DML: unknown SQL table field (assume not DB?):[$qname] in view:[{$vname}]",'DML');
						print_r($field);
					$f=$field;
				}
			}
			else
				$f=$field;
				
			if($uniqFlds)
				$kfields[$qname]=$f;
			else
				$kfields[]=$f;
			
		}
		
		return $this->postProcessField($kfields,$uniqFlds);
	}
	
	// last processing on fields: add list attribute for text-choice type if not defined
	function postProcessField($fColl,$uniqFlds=true)
	{
		$kfields=array();
		foreach($fColl as $field)
		{
			// get field name
			$qname=$field['QName'];

			// get text-choice list name, default from field name
			if (isset($field['TYPE']) && $field['TYPE']=='text-choice' && !isset($field['LIST']))
				$field['LIST']=preg_replace('/-./','_',$field['NAME']);

			// process label
			if (!isset($field['LABEL']))		
				$field['LABEL']=$field['NAME'];
				
			if ($uniqFlds)
				$kfields[$qname]=$field;
			else
				$kfields[]=$field;
		}
		
		return $kfields;
	}
	
	/*
	function & mapSQL($tables) 
	{		
		$tables2=array();
		$maps=&$this->maps;
		
		// for each table
		foreach($tables as $tname=>$table)
		{		
			$fields=array();
			
			// for each field
			foreach($table['fields'] as $name=>$field)
			{
				// remap field by adding up properties from map
				$type=$field['TYPE'];
				
				$field2=array();
				$field2['type']=$field['TYPE'];
				
				// check if map exists
				if (isset($maps[$type]))
				{
					$mapF=$maps[$type];
					
					// add up field attributes to current field
					$fserver=$mapF['server'];
					$field2['server']=$field + $fserver;
					
					// replace type by (PHP) mapped one
					if (isset($fserver['TYPE']))
						$field2['server']['TYPE']=$fserver['TYPE'];
									
					// add up sql def
					$field2['sql']=$mapF['sql'];
				}
				else
				{
					$field2['server']=$field;
					$field2['sql']=array();
				}

				// add new field	
				$fields[$name]=$field2;					
			}
			
			// add new list of fields
			$tables2[$tname]=$fields;			
		}
		
		return $tables2;
	}	
	*/
	
	// generate RecDesc
	function genRecDesc($tables,$package='')
	{
		$gen = new DML_genRecDesc();
		
		// set file root (from package)
		if ($package==''||$package=='root')
			$fileRoot=NX_CONF.'data/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$fileRoot=GENERIS_PACKAGES.$package.'/conf/data/';
		else
			$fileRoot=NX_CONF.'packages/'.$package.'/data/';

		// for each table
		foreach($tables as $tname=>$table)
		{
			$file="{$fileRoot}{$tname}_fprops.inc";
			nxltrace("DML(2): generate recDesc: $tname at $file",'DML');
			$this->saveFile($file,$gen->toPHP($tname, $table));

			$fileJs="{$fileRoot}{$tname}_fprops.js";
			if($this->fs->file_exists($fileJs))
			{
				nxltrace("DML(2): delete recDesc JSON cache: $tname at $fileJs",'DML');
				$this->fs->delete($fileJs);
			}
		}
	}
	
	// generate RecDesc
	function genSQL($package)
	{
		// set file root (from package)
		if ($package==''||$package=='root')
			$fileRoot=NX_CONF.'db/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES!='')
			$fileRoot=GENERIS_PACKAGES.$package.'/conf/db/';
		else
			$fileRoot=NX_CONF.'packages/'.$package.'/db/';

		$gen = new DML_genSQL($this->fs,$fileRoot,$this->props);
		$gen->genSQL($this->comp,$this->tables,$this->views);
	}
  
	// --------- utilities -------
	function &file_get_contents($filename) 
	{
		$fp = @fopen($filename, "r");
		if (!($fp))
			return 0;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		return $temp;
	}	
	
	function saveFile($path,$src) {
		$this->fs->saveFile($path,$src);
		$src=htmlentities($src);		
		nxltrace("RecDesc: $path:<br/>\n$src",'listing');
	}

}

?>
<?PHP

include_once('DML_parserXML.inc');

//---------- Parse DML ----------
class DML_parserDML extends DML_parserXML
{
	var $field;
	var $table;
	var $view;
	var $istable;
	var $fields;
	var $views;
	var $tables;
	var $isHelp;
	var $uniqF;
	var	$tprefix='_';
	var $fprefix='_';
	
	function __construct() {
		parent::__construct();
		$this->views=array();
		$this->tables=array();
		$this->field=null;
		$this->group=array();
		$this->groupPath='';
		$this->isHelp=false;
		$this->tableName='';
		$this->uniqF=false;
	}
	
	// event handlers
	function data_open($att) {
		$this->tprefix=$this->setDeftAtt($att,'TPREFIX','_');
		$this->fprefix=$this->setDeftAtt($att,'FPREFIX','_');
		return true;
	}
	
	function data_close() {
		return true;
	}

	function table_open($att) {
		$this->setDeftAtt($att,'TPREFIX',$this->tprefix);
		$this->setDeftAtt($att,'FPREFIX',$this->fprefix);
		
		$this->setDeftAtt($att,'NAME','');
		$this->tableName=$att['TABLE']=$att['NAME'];
        
        $this->setDeftAtt($att,'DATABASE','');
        $this->databaseName=$att['DATABASE'];
        
		$this->table=$att;
		$this->table[$this->coll='fields']=array();

		$this->groupDescs = array();

		return true;
	}

	function table_close() {
		if ($name=$this->table['NAME']);
		{
			$tableSrc=$this->getTableName($this->table['NAME']);
			$table=$this->tprefix.$tableSrc;
			
			$this->table['rprops']=array('table'=>$table,'table_src'=>$tableSrc,'name'=>$name,'tprefix'=>$this->table['TPREFIX'],'fprefix'=>$this->table['FPREFIX']);

            if (isset($this->table['DATABASE']) && $this->table['DATABASE'])
                $this->table['rprops']['database']=$this->table['DATABASE'];

	        if (isset($this->table['ASPECT']) && $this->table['ASPECT'])
	            $this->table['rprops']['aspect']=$this->table['ASPECT'];

			if (isset($this->table['HANDLER']) && $this->table['HANDLER'])
	            $this->table['rprops']['handler']=$this->table['HANDLER'];

			if (isset($this->table['CONFIG']) && $this->table['CONFIG'])
	            $this->table['rprops']['config']=$this->table['CONFIG'];

	        if (isset($this->table['CLASS']))
	            $this->table['rprops']['class']=$this->table['CLASS'];

	        if (isset($this->table['EXTENDSDWITHDS']))
	            $this->table['rprops']['extendsdWithDs']=$this->table['EXTENDSDWITHDS'];
            
			if (isset($this->groupDescs))
				$this->table['rprops']['gprops']=$this->groupDescs;

			$this->tables[$name]=$this->table;
			$this->tableName='';
		}
		return true;
	}
	
	function view_open($att) {
		if (!isset($att['NAME']))		
			return "missing view name";
        $this->setDeftAtt($att,'TABLE','');
        $this->tableName=$att['TABLE'];

        $this->setDeftAtt($att,'DATABASE','');
        $this->databaseName=$att['DATABASE'];
        
		$this->setDeftAtt($att,'TPREFIX',$this->tprefix);
		$this->setDeftAtt($att,'FPREFIX',$this->fprefix);

		$this->table=$att;
		$this->table[$this->coll='fields']=array();

		$this->groupDescs = array();
		return true;
	}

	function view_close() {
		$name=$this->table['NAME'];
//		$table=$this->table['TABLE'];
		$tableSrc=$this->getTableName($this->table['TABLE']);
		$table=$this->tprefix.$tableSrc;
		
		$this->table['rprops']=array('table'=>$table,'table_src'=>$tableSrc,'name'=>$name,'tprefix'=>$this->table['TPREFIX'],'fprefix'=>$this->table['FPREFIX']);
        if ($this->table['DATABASE'])
            $this->table['rprops']['database']=$this->table['DATABASE'];
            
        if (isset($this->table['ASPECT']))
            $this->table['rprops']['aspect']=$this->table['ASPECT'];

        if (isset($this->table['HANDLER']))
            $this->table['rprops']['handler']=$this->table['HANDLER'];

        if (isset($this->table['CLASS']))
			$this->table['rprops']['class']=$this->table['CLASS'];

		if (isset($this->table['CONFIG']) && $this->table['CONFIG'])
			$this->table['rprops']['config']=$this->table['CONFIG'];
                
		if (isset($this->table['EXTENDSDWITHDS']))
			$this->table['rprops']['extendsdWithDs']=$this->table['EXTENDSDWITHDS'];

		if (isset($this->groupDescs) && count($this->groupDescs))
		{
			$this->table['rprops']['gprops']=$this->groupDescs;
		}

		$this->views[$name]=$this->table;
		$this->tableName='';
		return true;
	}

	function fields_open($att) {
		// redirect field storage to key
		$this->coll='fields';
		$this->uniqF=true;
		return true;
	}

	function fields_close() {
		$this->uniqF=true;		
		return true;
	}

	function key_open($att) {
		// redirect field storage to key
		$this->table[$this->coll='key']=array();
		$this->uniqF=false;
		return true;
	}

	function key_close() {
		// restore field storage
		$this->coll='fields';
		$this->uniqF=true;		
		return true;
	}
	
	function sort_open($att) {
		// redirect field storage to key
		$this->table[$this->coll='sort']=array();
		$this->uniqF=false;
		return true;
	}

	function sort_close() {
		// restore field storage
		$this->coll='fields';
		$this->uniqF=true;
		return true;
	}	
	
	function group_by_open($att) {
		// redirect field storage to key
		$this->table[$this->coll='group_by']=array();
		$this->uniqF=false;
		return true;
	}

	function group_by_close() {
		// restore field storage
		$this->coll='fields';
		$this->uniqF=true;
		return true;
	}

	function pages_open($att) {
		// redirect field storage to key
		$this->table[$this->coll='pages']=$att;
		$this->uniqF=true;
		return true;
	}

	function pages_close() {
		// restore field storage
		$this->coll='fields';
		$this->uniqF=false;
		return true;
	}
	
	function group_open($att) {
		$name=preg_replace('#[-. /]#','_',$att['NAME']);
		$this->group[]=$name;
		$this->groupPath=implode('_',$this->group);
		$this->groupPathDot=implode('.',$this->group);

		$this->groupDescs[$this->groupPathDot]=array();
		if(isset($att['LABEL']))
			$this->groupDescs[$this->groupPathDot]['label']=$att['LABEL'];

		if(isset($att['LAYOUT']))
			$this->groupDescs[$this->groupPathDot]['layout']=$att['LAYOUT'];

		return true;
	}

	function group_close() {
		array_pop($this->group);
		if (count($this->group)>0)
		{
			$this->groupPath=implode('_',$this->group);
			$this->groupPathDot=implode('.',$this->group);
		}
		else
		{
			$this->groupPath='';
			$this->groupPathDot='';
		}
			
		return true;
	}

	function field_open($att) {
		// get field name
		if (!isset($att['NAME']))		
			return "missing field name";

		if (isset($att['_PMETHOD']))		
			unset($att['_PMETHOD']);
		
		// add group
		$name=preg_replace('#[-. /]#','_',$att['NAME']);
		if ($this->groupPath!='')
		{
			$n="{$this->groupPath}_$name";
			$att['ufname']=$this->fprefix.$n;
			$att['srcName']=$att['NAME']=$n;
			$att['group']=$this->groupPath;
			$att['gpath']=$this->groupPathDot;
		}
		else
		{
			$att['srcName']=$att['NAME'];
			$att['ufname']=$this->fprefix.$name;
			$att['NAME']=$name;
		}

		// process label
		/*
		if (!isset($att['LABEL']))		
			$att['LABEL']=$att['NAME'];
		*/
		
		if (isset($att['LABEL']))		
			$att['LABEL']=preg_replace("/[']/","\\'",$att['LABEL']);
		
		if (isset($att['ALIAS']))
		{
			$att['alias']=preg_replace('#[-. /]#','_',$att['ALIAS']);
			unset($att['ALIAS']);
		}

		// store field in collection (table or keys or sort)
		if (!isset($this->table[$this->coll][$att['NAME']]))
			$name=$att['NAME'];
		elseif (isset($att['alias']) && !isset($this->table[$this->coll][$att['alias']]))
			$name=$att['alias'];
		else
//			$name="{$this->coll}_{$att['NAME']}"; // bug?
			$name="{$att['NAME']}";
			
        if (isset($att['TABLE']))
            $t=$att['TABLE'];
        else
            $t=$this->tableName;

        // multibase support: if db is specified, add table attribute
        if (isset($att['DATABASE']))
            $db=$att['DATABASE'];
        else
            $db=$this->databaseName;

        if ($db)
            $this->setDeftAtt($att,'TABLE',$t);
                    
		// qualified name
		$qname="{$t}_{$name}";
        if ($db)
            $qname=$this->getQAlias($view,$db)."_{$qname}";
        
		$att['QName']=$qname;
		
		// unique field in collection?
		if($this->uniqF)
			$this->table[$this->coll][$qname]=$att;
		else
			$this->table[$this->coll][]=$att;
			
		$this->field=$qname; //$att['NAME'];
		return true;
	}
	
	function field_close() {
		$this->field=null;
		return true;
	}

	function help_open($att) {
		if (isset($this->field))
			$this->isHelp=true;
			
		return true;
	}
	
	function help_close() {
		$this->isHelp=false;
		return true;
	}

	function cdata($parser, $cdata)
	{
		if ($this->isHelp==true && $this->field!=null)
		{
			if($this->uniqF)
			{
				$f=&$this->table[$this->coll][$this->field];
			}
			else
			{
				$f=&$this->table[$this->coll][count($this->table[$this->coll])-1];
			}				

			if (isset($f['help']))
				$f['help'].=$cdata;
			else
				$f['help']=$cdata;
		}
		return true;
	}
	
	// ---- utility functions ---
	function setDeftAtt(&$att,$n,$d)
	{
		if (!isset($att[$n]))	
			$att[$n]=$d;
			
		return $att[$n];
	}
	
	function getTableName($tname)
	{
		return preg_replace('#[-. /]#','_',$tname);
	}
	
   /*
        returns a unique alias name to be used for tables and databases, for building field aliases
    */
    function & getQAlias($view,$qv)
    {
        if ($qv == null)
            return "";
        
        if (!@preg_match ('/^\%\{((?:\w|-)+):([^:|]+)([:|]([^}:]*)(:([^}:]+))?)?\}$/', $qv,$regs) &&
            !@preg_match ('/^#((?:\w|-)+):([^:|#]+)([:|]([^}#:]*)(:([^}#:]+))?)?\}$/', $qv,$regs))
            return "$qv";

        $media = strtolower($regs[1]);
        $name  = $regs[2];
        $dft   = trim($regs[4],'\'"');
        $storage = $regs[6];
                
        // translate name to field format
        $fname = strtr($name, array('-' => '_',' '=>'_','.'=>'_', '/'=>'_')); 
        $fprefix=$view['rprops']['fprefix'];
        $v = $dft;
            
        $wrap1='';
        $wrap2='';            
//        $wrap1='"\'".';
//        $wrap2='."\'"';            
        switch($media)
        {            
            case 'field': return '"'.$fprefix.preg_replace('/\./','._',$fname).'"'; // sql: table.field 
            case 'sql': return $name; // sql
            case 'record': return '$record->getFieldValue("'.$fname.'","DB","'.$dft.'")';
            case "param-field": // get or session
                $v = $wrap1.'param_'.$name.$wrap2;
                break;
            
            case "property":
                $v = $wrap1.$name.$wrap2;
                break;
            case "param": // get or session
            case "site":
            case "user":
            case "get":
            case "request":
            case "session":
            case "post":
            case "http":
            case "get-field":
            case "post-field":
            case "http-field":
            case "variable":
            case "constant":
            case "literal":
                $v = $wrap1.$media.'_'.$name.$wrap2;
                break;
            default:
                return $v = '$this->getQValue("'.$qv.'")';
                break;
        }

        $v = preg_replace('/[-. ]/','_',strtolower($v));
        return $v;
    }      
}
?>
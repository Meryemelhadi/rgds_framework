<?PHP

include_once('DML_parserXML.inc');

//---------- Parse DML ----------
class DML_SqlMap extends DML_parserXML
{
	var $maps;
	var $ftype;
	var $sql;
	
	function __construct() {
		parent::__construct();
	}
	
	function maps_open($att) {
		$this->maps=array();
		return true;
	}
	
	function field_open($att) {
		// get field name
		if (!isset($att['TYPE']))		
			return "missing field type in field map";
		
		$this->ftype=$att['TYPE'];
		$this->sql=array();
		return true;
	}

	function field_close() {
		// store field in collection (table or keys or sort)
		$this->maps[$this->ftype]=$this->sql;
		return true;
	}

	function sql_open($att) {
		$this->sql=$att;
		return true;
	}
}
?>
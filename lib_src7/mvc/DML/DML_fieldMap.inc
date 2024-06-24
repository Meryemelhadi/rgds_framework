<?PHP

include_once('DML_parserXML.inc');

//---------- Parse DML ----------
class DML_fieldMap extends DML_parserXML
{
	var $maps;
	var $sel;
	var $type;
	var $server;
	var $sql;
	
	function __construct() {
		parent::__construct();
	}
	
	// event handlers
	function data_close() {
	}

	function maps_open($att) {
		$this->sel='maps';
		$this->maps=array();
		return true;
	}

	function maps_close() {
		return true;
	}

	// sql
	function sql_open($att) {
		$this->sel='sql';
		return true;
	}

	function sql_close() {
		$this->sel='maps';
		return true;
	}

	// server
	function server_open($att) {
		$this->sel='server';
		return true;
	}

	function server_close() {
		$this->sel='maps';
		return true;
	}
	
	// field redirector
	function field_open($att) {
		$meth=$this->sel.'_field_open';
		return $this->$meth($att);
	}

	function field_close() {
		$meth=$this->sel.'_field_close';
		return $this->$meth();
	}

	// maps/field
	function maps_field_open($att) {
		// get field name
		if (!isset($att['TYPE']))		
			return "missing field type in field map";
		
		$this->ftype=$att['TYPE'];
		$this->server=array();
		$this->sql=array();
		return true;
	}

	function maps_field_close() {
		// store field in collection (table or keys or sort)
		$this->maps[$this->ftype]=array('server'=>$this->server,'sql'=>$this->sql);
		return true;
	}

	// sql/field
	function sql_field_open($att) {
		$this->sql=$att;
		return true;
	}
	function sql_field_close() {
		return true;
	}

	// server/field
	function server_field_open($att) {
		// echo "server=";var_export($att);
		$this->server=$att;
		return true;
	}
	function server_field_close() {
		return true;
	}
}
?>
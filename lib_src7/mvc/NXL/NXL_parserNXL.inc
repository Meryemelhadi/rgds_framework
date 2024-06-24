<?PHP

include_once(NX_LIB.'xml/NX_parserXML.inc');

//---------- Parse DML ----------
class NXL_parserNXL extends NX_parserXML
{
	function __construct($type="html") {
		parent::__construct(true,$type);
	}
	
	function nxl_open(&$node) {
		$node->setDeftAtt('path',$this->srcPath);
		return true;
	}
	
	//---  other instructions ---
	function _open($tag,$node) {
		return true;		
	}
	function _close($tag) {
		return true;		
	}
		
	function script_open(&$node) {
		$this->dom->setTextMode(true);
		return true;
	}
	
	function script_close() {
		$this->dom->setTextMode(false);
		return true;
	}
	
	function expression_open(&$node) {
		$this->dom->setTextMode(true);
		return true;
	}
	
	function expression_close() {
		$this->dom->setTextMode(false);
		return true;
	}
}
?>
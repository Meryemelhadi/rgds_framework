<?PHP

include_once(NX_LIB.'xml/NxTagLib_context'.NX_PHP_VER.'.inc');
include_once('NXL_parserNXL.inc');

class NXL_processor2
{	
	var $out;
	
	function __construct(&$props) 
	{
		$this->props=&$props;
	}
	
	function build($path)
	{	
		// read XML source
		$dom=&$this->genDOM($path);
		
		// $this->toPHP($dom);
			
		// generate logic modules
		return $this->execNodes($dom);
	}
	
	function &genDOM($path) 
	{
		$dom=null;
		$xml_parser = new NXL_parserNXL();
		
		if ($xml_parser->parseFile($path)) {
			$dom=&$xml_parser->getDOMDoc();
		}

		$xml_parser->close();
		$xml_parser=null;		
		return $dom;
	}
		
	// generate applications if any
	function execNodes($dom)
	{
		// generate application steps
		$gen = new NxTagLib_context($this->props,$dom,NXL_ROOTS,false);
		return $this->out=$gen->genTree($dom);
	}

	function toPHP($dom) {
		echo "<pre>\nDOM:\n";
		var_export($dom);
		echo '</pre>';
	}
	
	function buildOutput() {
		if (isset($this->out))
			return $this->out;
		else
			return 'compilation failed';
	}
	
}
?>
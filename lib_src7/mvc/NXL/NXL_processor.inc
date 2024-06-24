<?PHP

include_once(NX_LIB.'xml/NxTagLib_context'.NX_PHP_VER.'.inc');
include_once('NXL_parserNXL.inc');
//include_once('NXL_genPages.inc');
//include_once('NXL_genAppl.inc');
include_once('NXL_genSteps.inc');

class NXL_processor
{
	var $xml_parser;
	var $pages;
	var $logic;
	
	function __construct(&$props) 
	{
		$this->props=&$props;

		$this->pages=array();
		$this->logic=array();
		$this->fs=$props->getPlugIn('FileSys');
	}
	
	function genSrc($src,$type='html')
	{	
		$xml_parser = new NXL_parserNXL($type);
		
		if ($xml_parser->parse($src)) {
			$this->dom=$xml_parser->getDOMDoc();
		}

		$xml_parser->close();
		$xml_parser=null;
		
		// generate logic modules
		$this->genLogic($this->logic);
	}
	
	function build($path,$module='site')
	{	
		// read XML source
		$this->readNXL($path);
		
		//$this->toPHP();
			
		// generate pages
		// $this->genPages($this->pages);	
		
		// generate logic modules
		$this->genLogic($this->logic);
		
		// check that the module has been generated...
		// tbd.
		return true;
	}
	
	function toPHP() {
		// echo "pages:\n";
		// var_export($this->pages);
		
		// echo "\n\napplications:\n";
		// var_export($this->apps);
		
		echo "<pre>\nDOM:\n";
		var_export($this->dom);
		echo '</pre>';
	}
	
	function buildOutput() {
		if (isset($this->out))
			return $this->out;
		else
			return 'compilation failed';
	}
	
	function readNXL($path) 
	{
		$this->xml_parser = new NXL_parserNXL();
		
		if ($this->xml_parser->parseFile($path)) {
//			$this->pages=$this->xml_parser->pages;
//			$this->apps=$this->xml_parser->apps;
			$this->dom=$this->xml_parser->getDOMDoc();
		}

		$this->xml_parser->close();
		$this->xml_parser=null;		
	}
	
	// generate pages if any
	function genPages($pages)
	{
		$gen = new NXL_genPages($this->fs,NX_DOC_ROOT);
		$gen->genPages($this->pages);
	}
	
	// generate applications if any
	function genLogic()
	{
		// generate application steps
		$gen = new NXL_genSteps($this->props,$this->dom, NX_GEN.'logic/');
		//$gen->genSteps($this->apps);
		$this->out=$gen->genTree($this->dom);
		
		// generate application state machine
		/*
		if (isset($this->apps))
		{
			$gen = new NXL_genAppl($this->fs,NX_GEN.'logic/');
			$gen->genLogic($this->apps);
		}
		*/
	}
}
?>
<?php

define('NX_TL_IGNORE_CONTENT','ignore_content');

class NxTagLib
{
	var $context;
	var $props;
	var $xpath;
	var $inc_paths=array();
	var $uri=null;
	
	function __construct(&$context) {
		$this->context=&$context;
		$this->xpath=&$this->context->xpath;
		$this->props=&$context->props;
	}
	
	// where to find related includes
	function addIncludePath($path) {
		$this->inc_paths[$path]=$path;
	}
	function getIncludePaths() {
		return $this->inc_paths;
	}
	
	function onLoad() 
	{
		$this->context->addResource('lib',get_class($this));
	}
		
	// reset indent level
	function setIndentLevel($level=0)
	{
		$this->context->level=0;
	}

	// check if parent is an instruction block
	function isInBlock($node,$parent=null)
	{
		// instruction or expression ?
		if ($parent==null)
			$parent=$this->context->getParentElement($node);
			
		if ($parent==null || $parent->nodeType!=ELEMENT_NODE)
			return false;
			
		return $this->isBlock($parent);
	}

	function isBlock($node)
	{
		if ($node->baseURI == 'http://www.w3.org/1999/XSL/Transform')
		{
			return $this->isInBlock($node);
		}
			
		if ($node->getAttribute('_isblock','false')=='true')
			return true;
		else
			return false;
	}
		
	// acess to properties (NB> shared by all tag libs)
	function getProperty($name,$deft=null,$cascade=true)
	{
		return $this->props->getProperty($name,$deft,$cascade);
	}
	function setProperty($name,$v)
	{
		return $this->props->setProperty($name,$v);
	}

	// pass the content through
	function passThrough($node,$content,$pad='',$parseAtts=false)
	{
		$name=$node->tagName;
		$dom=&$this->getDOM();
		$atts='';
		if ($name=='#text')
			return $node->data;
			
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($parseAtts)
				$av=$this->parseVal($av,false);
				
			$atts .=" $an=\"$av\"";
		}
		
		if ($content!='')
			return "\n{$pad}<{$name}{$atts}>$content\n{$pad}</$name>";
		else
			return "\n{$pad}<{$name}{$atts}/>";
	}

	// add a resource to be used by top tags (example: imports, etc.)
	
	function addResource($val,$type='includes',$key=null)
	{
		$this->context->addResource($val,$type,$key);
	}
	
	// remove a resource
	function removeResource($type='includes')
	{
		$this->context->removeResource($type);
	}
	
	// return array of resources
	function getResources($type='includes',$sepflatten=null,$removeKey=false)
	{
		return $this->context->getResources($type,$sepflatten,$removeKey);
	}
	
	// return a resources by key
	function getResource($type,$key,$removeKey=false)
	{
		return $this->context->getResource($type,$key,$removeKey);
	}

	// return a resources by key
	function getLastResource($type,$removeKey=false)
	{
		return $this->context->getLastResource($type,$removeKey);
	}

	// error management
	function fatal($msg)
	{
		return $this->context->libFatalError($msg);
	}
	
	// error management
	function warning($msg)
	{
		return $this->context->libWarning($msg);
	}
	
	// ========= DOM =============
	
	function &getDOM() {
		return $this->context->getDOM();
	}
	
	function &getParentElement($node)
	{
		$dom=&$this->context->getDOM();
		return $dom->getParentElement($node);
	}
	
	// =========== attribute value parsing =============
	
	// get node attribute and parse it, wrap with single quotes if requested
	/**
	* @return bool, true if attribute exists or has a default value
	* @param string $var
	* @param DOMNode $node
	* @param string $name
	* @param string $dft
	* @param bool $parse
	* @param bool $wrapQuotes
	* @param bool $wrapPhp wrap value with php <? php ? >
	* @desc returns a node attribute value. The value can be parsed (translating NXL #xx:# expressions)
	* and wrapped with single quotes and/or php script markers.
	*/
	function getAttribute(&$var,$node,$name,$dft=null,$parse=true,$wrapQuotes=false,$wrapPhp=false)
	{
		if(($node->nodeType!=ELEMENT_NODE) || (($var=$node->getAttribute($name,$dft))===null))
			return false;
			
		if ($parse)
			$var=$this->parseVal($var,$wrapQuotes,$wrapPhp,$dft);
		return true;
	}
	
	// get boolean from string (true, yes, no , false)
	function getBool($v) {
		if($v===true ||$v===false)
			return $v;

		$v=strtolower($v);
		if ($v=='yes' || $v=='true' || $v==='1')
			return true;
			
		if ($v=='no' || $v=='false'|| $v==='0')
			return false;

		// nxError('unknown value for NXL page attribute:'.$v);	
		return false;
	}
	
	//  parsing attributes
	function parseQvalue($v,$quotes=false,$wrapPhp=false)
	{
		if (preg_match('/#(?:[a-z_]+:|\$)[a-z.-_]+#/i',$v)==0)
		{
			$v="#$v#";
		}
		
		return $this->parseVal($v,$quotes,$wrapPhp);
	}
	
	function parseVal($v,$quotes=false,$wrapPhp=false,$dft='',$prop='$this')
	{
//		if ($quotes && !preg_match('/\$[a-zA-Z_][a-zA-Z_0-9]*/',$v))
		if ($quotes)
		{
			$v=trim($v,'\'');
			$v="'$v'";
		}
			
		if ($dft!='')
			$dft="|$dft";
			
		$v=preg_replace('/#{([^}#]+)}#/','#xo:xpath($1)#',$v);
					
		if (preg_match("/^\s*'.+'\s*$/",$v))
		{
			// string
			$res=preg_replaceX(
				array(
					'/[\\\\]#/',
					'/#this:([^#]+)#/i',
					'/#error:msg#/i',
					'/#(\w+):([^:(|#]+)#/',
					'/#(?:var|param):([^:|#]+)#/i',
					'/#(?:string):([^#]+)#/i',
					'/#\$([^:|#]+)#/i',
					'/#(?:var|param):(?:([^:|#]+)(?:[:|]\'?([^#:\']*)\'?))#/i',
					'/#(?:global):(?:([^:|#]+)(?:[:|]\'?([^#:\']*)\'?))#/i',
					'/#(?:user):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:user)[.]([^:]+):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:site):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:prop(?:erty)?|=):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:@prop(?:erty)?|=):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/ie',
					'/#(?:get):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:server):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:post):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:req(?:uest)?):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:session):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:const(?:ant)?):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#([a-z]+,[a-z,]+):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/ie',
					'/#([a-zA-Z_]+)::?([^#]+)#/e',					
					"/(^''[.])|([.]''$)/",
					"/\.''\./",
					'/ZAZAZ/'
				),
				array(
					'ZAZAZ',
					'\'.'.$prop.'->$1.\'',
					'\'.$this->error->getErrorMsg().\'',
					'#$1:$2'.$dft.'#',
					'\'.$$1.\'',
					'\'.'.$prop.'->getString("$1").\'',
					'\'.$$1.\'',
					'\'.(isset($$1)?$$1:\'$2\').\'',
					'\'.(isset($GLOBALS["$1"])?$GLOBALS["$1"]:"$2").\'',
					'\'.$GLOBALS["_NX_user"]->getProperty("$1","$2").\'',
					'\'.$GLOBALS["_NX_$1"]->getProperty("$2","$3").\'',
					'\'.$GLOBALS["_NX_site"]->getProperty("$1","$2").\'',
					'\'.'.$prop.'->getProperty("$1","$2").\'',
					'$this->props->getProperty("$1","$2")',
					'\'.(isset($_GET["$1"])?$_GET["$1"]:"$2").\'',
					'\'.(isset($_SERVER["$1"])?$_SERVER["$1"]:"$2").\'',
					'\'.(isset($_POST["$1"])?$_POST["$1"]:"$2").\'',
					'\'.(isset($_REQUEST["$1"])?$_REQUEST["$1"]:"$2").\'',
					'\'.(isset($_SESSION["$1"])?$_SESSION["$1"]:"$2").\'',
					'\'.constant("$1").\'',
					"NxTagLib::getValList('\$1','\$2','\$3',true)",
					"\$this->evalFunc('\$1','\$2',true,null)",
					'','.',
					'#'
				),
				$v,$this);		
		}
		else
		{
			// other
			$res=preg_replaceX(
				($regs=array(
					'/[\\\\]#/',
					'/#this:([^#]+)#/i',
					'/#error:msg#/i',
					'/#(\w+):([^:|(#]+)#/',
					'/#(?:var|param):([^:|#]+)#/i',
					'/#(?:string):([^#]+)#/i',
					'/#\$([^:|#]+)#/i',
					'/#(?:var|param):(?:([^:#|]+)(?:[|:]\'?([^#:]*)\'?))#/i',
					'/#(?:global):(?:([^:#|]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:user):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:user)[.]([^:]+):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:site):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:prop(?:erty)?|=):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:@prop(?:erty)?|=):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/ie',
					'/#(?:get):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:server):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:post):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:req(?:uest)?):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:session):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:const(?:ant)?):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#([a-z]+,[a-z,]+):(?:([^:|#]+)(?:[|:]\'?([^#:\']*)\'?)?)#/ie',
					'/#([a-zA-Z_]+)::?([^#]+)#/e',					
					'/ZAZAZ/',
					//'/(?:^[{]|[^%][{])([^{}]+)[}]/e',
				)),
				array(
					'ZAZAZ',
					''.$prop.'->$1',
					$prop.'->error->getErrorMsg()',
					'#$1:$2'.$dft.'#',
					'$$1',
					$prop.'->getString("$1")',
					'$$1',
					'(isset($$1)?$$1:\'$2\')',
					'(isset($GLOBALS["$1"])?$GLOBALS["$1"]:"$2")',
					'$GLOBALS["_NX_user"]->getProperty("$1","$2")',
					'$GLOBALS["_NX_$1"]->getProperty("$2","$3")',
					'$GLOBALS["_NX_site"]->getProperty("$1","$2")',
					$prop.'->getProperty("$1","$2")',
					'$this->props->getProperty("$1","$2")',					
					'(isset($_GET["$1"])?$_GET["$1"]:"$2")',
					'(isset($_SERVER["$1"])?$_SERVER["$1"]:"$2")',
					'(isset($_POST["$1"])?$_POST["$1"]:"$2")',
					'(isset($_REQUEST["$1"])?$_REQUEST["$1"]:"$2")',
					'(isset($_SESSION["$1"])?$_SESSION["$1"]:"$2")',
					'constant("$1")',
					"NxTagLib::getValList('\$1','\$2','\$3',false)",
					"\$this->evalFunc('\$1','\$2',false,null)",
					//"\$this->evalFunc('xsl','xpath(\$1)',false,'http://www.w3.org/1999/XSL/Transform')",
					'#'
				),
				$v, $this);
		}
		
        if (!isset($res))
		{
			nxltrace("cant transform: '$v'",'error');
			nxError("cant transform: $v",'compil');
            $res=$v;
		}
            
		if ($wrapPhp)
			return '<?'."php echo $res; ?".'>';
		
		return $res;
	}

	static function parseValStatic($v,$quotes=false,$wrapPhp=false,$dft='',$prop='$this')
	{
//		if ($quotes && !preg_match('/\$[a-zA-Z_][a-zA-Z_0-9]*/',$v))
		if ($quotes)
		{
			$v=trim($v,'\'');
			$v="'$v'";
		}
			
		if ($dft!='')
			$dft="|$dft";
			
		$v=preg_replace('/#{([^}#]+)}#/','#xo:xpath($1)#',$v);
					
		if (preg_match("/^\s*'.+'\s*$/",$v))
		{
			// string
			$res=preg_replaceX(
				array(
					'/[\\\\]#/',
					'/#this:([^#]+)#/i',
					'/#error:msg#/i',
					'/#(\w+):([^:(|#]+)#/',
					'/#(?:var|param):([^:|#]+)#/i',
					'/#(?:string):([^#]+)#/i',
					'/#\$([^:|#]+)#/i',
					'/#(?:var|param):(?:([^:|#]+)(?:[:|]\'?([^#:\']*)\'?))#/i',
					'/#(?:global):(?:([^:|#]+)(?:[:|]\'?([^#:\']*)\'?))#/i',
					'/#(?:user):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:user)[.]([^:]+):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:site):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:prop(?:erty)?|=):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:@prop(?:erty)?|=):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/ie',
					'/#(?:get):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:server):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:post):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:req(?:uest)?):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:session):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:const(?:ant)?):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#([a-z]+,[a-z,]+):(?:([^:#|]+)(?:[:|]\'?([^#:]*)\'?)?)#/ie',
					'/#([a-zA-Z_]+)::?([^#]+)#/e',					
					"/(^''[.])|([.]''$)/",
					"/\.''\./",
					'/ZAZAZ/'
				),
				array(
					'ZAZAZ',
					'\'.'.$prop.'->$1.\'',
					'\'.$this->error->getErrorMsg().\'',
					'#$1:$2'.$dft.'#',
					'\'.$$1.\'',
					'\'.'.$prop.'->getString("$1").\'',
					'\'.$$1.\'',
					'\'.(isset($$1)?$$1:\'$2\').\'',
					'\'.(isset($GLOBALS["$1"])?$GLOBALS["$1"]:"$2").\'',
					'\'.$GLOBALS["_NX_user"]->getProperty("$1","$2").\'',
					'\'.$GLOBALS["_NX_$1"]->getProperty("$2","$3").\'',
					'\'.$GLOBALS["_NX_site"]->getProperty("$1","$2").\'',
					'\'.'.$prop.'->getProperty("$1","$2").\'',
					'$this->props->getProperty("$1","$2")',
					'\'.(isset($_GET["$1"])?$_GET["$1"]:"$2").\'',
					'\'.(isset($_SERVER["$1"])?$_SERVER["$1"]:"$2").\'',
					'\'.(isset($_POST["$1"])?$_POST["$1"]:"$2").\'',
					'\'.(isset($_REQUEST["$1"])?$_REQUEST["$1"]:"$2").\'',
					'\'.(isset($_SESSION["$1"])?$_SESSION["$1"]:"$2").\'',
					'\'.constant("$1").\'',
					"NxTagLib::getValList('\$1','\$2','\$3',true)",
					"\$this->evalFunc('\$1','\$2',true,null)",
					'','.',
					'#'
				),
				$v);		
		}
		else
		{
			// other
			$res=preg_replaceX(
				($regs=array(
					'/[\\\\]#/',
					'/#this:([^#]+)#/i',
					'/#error:msg#/i',
					'/#(\w+):([^:|(#]+)#/',
					'/#(?:var|param):([^:|#]+)#/i',
					'/#(?:string):([^#]+)#/i',
					'/#\$([^:|#]+)#/i',
					'/#(?:var|param):(?:([^:#|]+)(?:[|:]\'?([^#:]*)\'?))#/i',
					'/#(?:global):(?:([^:#|]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:user):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:user)[.]([^:]+):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:site):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:prop(?:erty)?|=):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:@prop(?:erty)?|=):(?:([^|:#]+)(?:[:|]\'?([^#:]*)\'?)?)#/ie',
					'/#(?:get):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:server):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:post):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:req(?:uest)?):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#(?:session):(?:([^:|#]+)(?:[:|]\'?([^#:]*)\'?)?)#/i',
					'/#(?:const(?:ant)?):(?:([^:|#]+)(?:[|:]\'?([^#:]*)\'?)?)#/i',
					'/#([a-z]+,[a-z,]+):(?:([^:|#]+)(?:[|:]\'?([^#:\']*)\'?)?)#/ie',
					'/#([a-zA-Z_]+)::?([^#]+)#/e',					
					'/ZAZAZ/',
					//'/(?:^[{]|[^%][{])([^{}]+)[}]/e',
				)),
				array(
					'ZAZAZ',
					''.$prop.'->$1',
					$prop.'->error->getErrorMsg()',
					'#$1:$2'.$dft.'#',
					'$$1',
					$prop.'->getString("$1")',
					'$$1',
					'(isset($$1)?$$1:\'$2\')',
					'(isset($GLOBALS["$1"])?$GLOBALS["$1"]:"$2")',
					'$GLOBALS["_NX_user"]->getProperty("$1","$2")',
					'$GLOBALS["_NX_$1"]->getProperty("$2","$3")',
					'$GLOBALS["_NX_site"]->getProperty("$1","$2")',
					$prop.'->getProperty("$1","$2")',
					'$this->props->getProperty("$1","$2")',					
					'(isset($_GET["$1"])?$_GET["$1"]:"$2")',
					'(isset($_SERVER["$1"])?$_SERVER["$1"]:"$2")',
					'(isset($_POST["$1"])?$_POST["$1"]:"$2")',
					'(isset($_REQUEST["$1"])?$_REQUEST["$1"]:"$2")',
					'(isset($_SESSION["$1"])?$_SESSION["$1"]:"$2")',
					'constant("$1")',
					"NxTagLib::getValList('\$1','\$2','\$3',false)",
					"\$this->evalFunc('\$1','\$2',false,null)",
					//"\$this->evalFunc('xsl','xpath(\$1)',false,'http://www.w3.org/1999/XSL/Transform')",
					'#'
				),
				$v);
		}
		
        if (!isset($res))
		{
			nxltrace("cant transform: '$v'",'error');
			nxError("cant transform: $v",'compil');
            $res=$v;
		}
            
		if ($wrapPhp)
			return '<?'."php echo $res; ?".'>';
		
		return $res;
	}

	static function getValList($list,$n,$dft,$wrap)
	{
		$s='';
		$e='';
		$sels=explode(',',strtolower($list));
		foreach ($sels as $sel)
		{
			$var=strtoupper($sel);
			switch($sel)
			{
				case 'server':
					$n=strtoupper($n);
					if ($n=='FROM')
						$n='HTTP_REFERER';
				case 'get':
				case 'post':
				case 'session':
				case 'request':
					$s.="(isset(\$_{$var}['$n'])?\$_{$var}['$n']:(";
					$e.='))';
					break;
				case 'param':
					$s.="(isset(\${$n})?\${$n}:(";
					$e.='))';
					break;
				case 'user':
					$s.="\$GLOBALS['_NX_user']->getProperty('$n',";
					$e.=')';
					break;
				case 'prop':
				case 'property':
					$s.="\$this->getProperty('$n',";
					$e.=')';
					break;
			}
		}
		if ($wrap)
		{
			$s="'.$s";
			$e="$e.'";
		}
		
		return "{$s}'{$dft}'{$e}";
	}
	
	// get lib and pass the expression to it
	function evalFunc($prefix,$expr,$wrapQuotes,$uri=null)
	{
 		// nxLog("parse expression prefix=[$prefix], expr=[$expr]".($wrapQuotes?',quotes':''),'TAG_LIB');
		// exec lib function
		$ret=$this->context->evalFunc($prefix,$expr,$uri);
		
		if($wrapQuotes && preg_match('/\$[a-zA-Z_]|constant\(/',$ret))
		{
			$ret="'.$ret.'";
		}
		
		nxltrace("calling function: $expr =[$ret]",'walker',0,true);

		return $ret;
	}
	
	// get rid of '.' string sequences like: 'aa'.'bb' replaced by 'aabb' 
	function simplifyExpr($expr){
		return preg_replace("/'[.]'/",'',$expr);
	}
	
	// creates an identifier with name prefix
	function createVarID($name='var')
	{
		static $id=0;
		return "_nx_$name".($id++);
	}
	
	// ========== default handlers =====================
	function __process_tag($node,$content,$level,$pad,$prefix,$tag,$mode)
	{
		if ($this->getAttribute($debug,$node,'debug') && ($debug=='yes'||$debug=='true'))
			DebugBreak();

		$method=$mode.$this->getHandlerName($tag);		
		if (method_exists($this,$method))
		{
			// backup attributes before parsing
			// if($mode=='_')	$node->_refreshNode();

			$res = $this->$method($node,$content,$level,$pad);

			// restore attributes
			// if($mode=='') $node->_restoreNode();

			return $res;
		}
		else		
		{
			$method="{$mode}tag_default";
			return $this->$method($node,$content,$level,$pad,$prefix,$tag);
		}
	}
	
	// map tag name to its handler
	// php symbols maped to symbol_, #name to name_, a-name to a_name
	function getHandlerName($tag) {
		return preg_replace(
			array('/[_#](comment|text)$/','/(comment|text)[_#]$/','/^[#](.*)$/','/[# .-]/',
				'/^(include|include_once|list|unset|require|namespace|echo|script|foreach|while|break|continue|if|else|class|object|function|return|exit|array|extends|and|or|not|echo)$/'),
			array('$1_','$1_','$1_','_','$1_'),
			$tag);
	}

	// expressions ex. xsl::value(@name)
	function _expression_($node,$expr,$baseURI,&$doc,$prefix) 
	{	
		if(preg_match('/^\s*([^(]+)(?:[(](.*)[)])?\s*$/',$expr,$matches))
		{
			$f=trim($matches[1]);
			$argList=$matches[2];
			$class=get_class($this);
			
			// 1) try first with function like format
			// ex. #xsl::xpath(/html/body)#			
			// handler of the form 
			// 		function _func_count($node,$args,$level=0)
			$f2="_func_$f";

			if (method_exists($this,$f2))
			{
				nxltrace("calling function:$prefix:$f2($argList) URI=$baseURI from lib [$class], node={$node->id}",'debug');
				$args=explode(',',$argList);
				$ret=$this->$f2($node,$args);
				nxltrace("calling function:$prefix:$f2($argList)=[$ret]",'debug');
				return $ret;
			}
			
			nxltrace("calling function as node:$prefix:$f($argList) URI=$baseURI from lib [$class], node={$node->id}",'debug');
			
			// 2) try calling a node handler directly, by creating a pseudo node and calling handler
			$atts=array();
			if (preg_match_all('|([^:=]+)[:=]\'([^\']*)\'|',$argList,$amatches,PREG_SET_ORDER) ||
				preg_match_all('|([^:=]+)[:=]$([^ ]+)|',$argList,$amatches,PREG_SET_ORDER))
			{
				// attribute like format
				//ex. #tpl::field(name='image' format='html' options='url')#
				foreach ($amatches as $att)
				{
					$atts[trim($att[1])]=$att[2];
				}
			}
			else if (($argList=trim($argList,', '))!='')
			{
				// function like params: create implicit attribute names p1, p2, etc.
				$args=explode(',',$argList);
				foreach ($args as $i=>$arg)
				{
					$atts["p{$i}"]=$arg;
				}
			}
				
			// create a pseudo node
			$fnode=$doc->createElementNS($baseURI,"$prefix:{$matches[1]}",$atts);
			$fnode->_attachToParent($node);
			
			// call the function dealing with this "tag" or default handler
			if (!method_exists($this,$f))
				$f='tag_default';
			
			// start with opening tag function
			$pre_f="_$f";
			if (method_exists($this,$pre_f))
				if ($this->$pre_f($fnode)===NX_TL_IGNORE_CONTENT)
				{
					nxltrace("error calling function $prefix::$f",'error');
					return "/*error calling function $prefix::$f*/";
				}
				
			// get content from tag function
			$ret=$this->$f($fnode,'',0,'');
			
			nxltrace("calling node as function:$prefix:$f($argList)=[$ret]",'debug');
			return $ret;
		}
		
		// not a function, call "expression" handler
		return $this->expression_($node,$expr,$baseURI,$doc,$prefix);
	}
	
	/* ignore xml content (a super comment tag..) */
	function _ignore($node,$level,$pad)
	{
		return NX_TL_IGNORE_CONTENT;
	}	
	function ignore($node,$content,$level,$pad)
	{
		return '';
	}	
	
	function expression_($node,$expr,$baseURI,&$doc,$prefix)
	{
		return $expr;
	}
	
	function _comment_($node)
	{	
		$this->keepAllText($node,true);
	}
	
	/* NB. comment are converted to _comment element */
	function comment_($node,$content,$level,$pad)
	{	
		return '';
	}
	
	function _pi_($node)
	{	
		$this->keepAllText($node,true);
	}
	
	/* NB. comment are converted to _comment element */
	function pi_($node,$content,$level,$pad)
	{	
		$this->getAttribute($type,$node,'type','');
		return '<?'.$type.$content.'?>';
	}	
	
	// text node: returns text
	// NB. get rid of blank text except if parent requests to keep it by 
	// setting attribute _keepAllText to "true".
	function _text_() {
	}
	
	function text_($node)
	{	
		if (trim($node->data)==='')
		{
			$parent=$this->context->getParentElement($node);
			if (isset($parent->nodeType) && $parent->nodeType==ELEMENT_NODE && $parent->getAttribute('_keepAllText')=='true')
				return $node->data;
			else
				return '';
		}
		
		return $node->data;
	}
	
	// use it to keep blank text (removed by default by text handler)
	function keepAllText($node,$keep) {
		if ($keep==false)
			return;
			
		$node->setAttribute('_keepAllText','true');
	}
	
	function trace($node,$content,$level,$pad)
	{
		$this->getAttribute($cl,$node,'class','trace',false,true);
		$content=nl2br(htmlentities($content));
		nxltrace($content,$cl);	
		return '';	
	}

	function _include_()
	{
	}
	
	function include_($node,$content,$level,$pad)
	{
		$package='';
		if ($this->getAttribute($uri,$node,'uri',null,true))
		{
			$fpath=$this->context->getPathByURI($uri);
			if (!$fpath)
				$this->fatal('cant find file for given uri '.$uri);
		}
        elseif($this->getAttribute($fpath,$node,'file',null,true,true))
        {
        }
        elseif (!$this->getAttribute($fpath,$node,'path',null,true,true))
        {
            DebugBreak();
            nxError("<include> uses path instead of file attribute : ".$fpath);
        }
        else
        {
            return $this->fatal('no file or uri attribute');
        }

		if($this->getAttribute($xpath,$node,'xpath','',true))
			$this->context->addResource($xpath,'xpath');	
			
		if ($this->getAttribute($is_once,$node,'mode','')&& $is_once=='once')
			$forceReload=false;
		else
			$forceReload=true;
			
		if($fpath)
			eval('$fpath='.$fpath.';');
		else
            return $this->fatal('no file or uri attribute');

		$res=$this->loadSiteFile($fpath,$xpath,$forceReload);

		if ($res!==null)
			return $res;
		else
			return $this->fatal("include file doesn't exist:[$fpath]");	
	}

	function _include_once()
	{
	}
	
	function include_once_($node,$content,$level,$pad)
	{
		$package='';
		if ($this->getAttribute($uri,$node,'uri',null,true))
		{
			$fpath=$this->context->getPathByURI($uri);
			if (!$fpath)
				$this->fatal('cant find file for given uri '.$uri);
		}
		elseif (!$this->getAttribute($fpath,$node,'file',null,true,true))
		{
			$this->fatal('no file attribute');
		}

		if($this->getAttribute($xpath,$node,'xpath','',true))
			$this->context->addResource($xpath,'xpath');	
			
		$forceReload=false;
			
		// eval('$fpath='.$fpath.';');
        $fpath=trim($fpath,"' \"");        
		$res=$this->loadSiteFile($fpath,$xpath,$forceReload);

		if ($res!==null)
			return $res;
		else
        {
            // DebugBreak();
			return $this->fatal("include file doesn't exist:[$fpath]");	
        }
	}
	
	function processXML($node,$content,$level,$pad)
	{
		// get source to compile
		$this->getAttribute($src,$node,'value',$content,false,false);	

		// selected nodes within source?
		if($this->getAttribute($xpath,$node,'xpath','/',true))
			$this->context->addResource($xpath,'xpath');	
				
		// element to apply ?
		if ($this->getAttribute($name,$node,'element',null))
		{
			// what is its uri
			if ($this->getAttribute($uri,$node,'uri',null))
				$uri=" xmlns=\"{$uri}\"";
			else
				$uri='';	
				
			// add up attributes to pass to the tag
			$atts='';
			foreach ($node->_getAttributes($node) as $an=>$av)
			{
				switch($an)
				{
					case 'element':
					case 'value':
					case 'uri':
						break;
					default:		
						$av=$this->parseVal($av,false);
						$atts .=" $an=\"$av\"";
						break;
				}
			}
			
			// wrap source with element
			$src="\n{$pad}<{$name}{$atts}{$uri}>$src\n{$pad}</$name>";
		}

		// 
		nxltrace('SOURCE TO PROCESS:[START]<br/>'.nl2br(htmlentities($src)).'<br/>[END SOURCE]','frag_source');
		
		// process it
		return $this->context->processFragmentSource($src,$xpath,$level-1);
	}	
	
	function get($node,$content,$level,$pad)
	{	
		/* value */
		if (($value=$node->getAttribute('value'))!==null
			||
			($value=$node->getAttribute('default'))!==null)
		{
			$value = $this->parseVal($value,true);
		}
		else
		{
			$value=null;
		}

		/* name */
		if (!$this->getAttribute($name,$node,'name'))
		{
			if ($value===null)
				return $this->fatal('error in nxl:get : no "name" or value attribute');
			else
				return $value;
		}
			
		$an=explode(':',$name);
		if (isset($an[1]))
		{
			$name=$an[1];

			// get service, ex. user.subscriber:perms
			$an2=explode('.',$an[0]);
			$sel=$an2[0];
			if (isset($an2[1]))
			{
				// qualified selector with service
				$service=$an2[1];
			}
			else
				$service='';
		}
		else
		{
			$sel='property';
			$service='';
		}

		if ($value===null)
			$value="''";
		
		switch($sel)
		{
			case 'server':
				$name=strtoupper($name);
				if ($name=='FROM')
					$name='HTTP_REFERER';
			case 'request':
			case 'post':
			case 'cookie':
			case 'get':
				$var='_'.strtoupper($sel)."[\"$name\"]";
				return "\n{$pad}(isset(\${$var})?\${$var}:$value)";
				break;
			case 'global':
				$var="GLOBALS[\"$name\"]";
				return "\n{$pad}(isset(\${$var})?\${$var}:$value)";
			case 'prop':
			case 'property':
				return "\n{$pad}\$this->getProperty(\"{$name}\",$value)";
			case 'var':
			case 'param':
			case 'parameter':
				return "\n{$pad}\${$name}";		
			case 'site':
				$var="GLOBALS['_NX_site']";
				return "\n{$pad}(isset(\${$var})?\${$var}->getProperty(\"$name\",$value):$value)";
			case 'user':
				if ($service=='')
					$service='user';
				$var="GLOBALS[\"_NX_$service\"]";
				return "\n{$pad}(isset(\${$var})?\${$var}->getProperty(\"$name\",$value):$value)";
			default:
				return "null /*get : unknown variable $sel:$name*/";
		}
	}
	
	function _set($node,$level)
	{
		// $this->context->declareBlock($node,false);		
	}

	function set($node,$content,$level,$pad)
	{	
		/* name */
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('error in nxl:get : no "name" attribute');
			
		$an=explode(':',$name);
		if (isset($an[1]))
		{
			$name=$an[1];

			// get service, ex. user.subscriber:perms
			$an2=explode('.',$an[0]);
			$sel=$an2[0];
			if (isset($an2[1]))
			{
				// qualified selector with service
				$service=$an2[1];
			}
			else
				$service='';
		}
		else
		{
			$sel='property';
			$service='';
		}
				
		/* value */
		if (($value=$node->getAttribute('value'))!==null)
		{
			if ($value!='null')
				$value = $this->parseVal($value,true);
		}
		else
		{
			$value=$content;
		}

		/* set operator */
        $this->getAttribute($op,$node,'operator','=',true,false);
		
		// instruction or expression ?
		if (true && $this->context->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
			
		switch(strtolower($sel))
		{
			case 'request':
			case 'post':
			case 'session':
			case 'get':
				$var='_'.strtoupper($sel)."[\"$name\"]";
				return "\n{$pad}\${$var}$op$value;";				
			case 'cookie':
				/* duration for cookie */
				if ($this->getAttribute($validdate,$node,'until',null,true,true))
				{
					$validdate="strtotime($validdate)";
				}
				else
				{
					// default one day 
					$validdate='strtotime($this->getProperty("cookie.duration",(string)(24*3600)))';
				}
				return "\n{$pad}setcookie($name,$value,$validdate);";
				
			case 'global':
				$var="GLOBALS[\"$name\"]";
				return "\n{$pad}\${$var}$op$value;";				
			case 'prop':
			case 'property':
				return "\n{$pad}\$this->setProperty(\"{$name}\",$value);";
			case 'var':
			case 'param':
			case 'parameter':
				return "\n{$pad}\${$name}$op{$value};";		
			case 'site':
				$var="\$GLOBALS['_NX_site']";
				return "\n{$pad}{$var}->setProperty(\"{$name}\",$value);";
			case 'user':
				if ($service=='')
					$service='user';
				$var="\$GLOBALS[\"_NX_$service\"]";
				return "\n{$pad}{$var}->setProperty(\"{$name}\",$value);";

			default:
				return "\n{$pad}/*set var: unknown variable $sel:$name*/";
		}
	}
	function unset_($node,$content,$level,$pad)
	{	
		/* name */
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('error in nxl:get : no "name" attribute');
			
		$an=explode(':',$name);
		if (isset($an[1]))
		{
			$name=$an[1];

			// get service, ex. user.subscriber:perms
			$an2=explode('.',$an[0]);
			$sel=$an2[0];
			if (isset($an2[1]))
			{
				// qualified selector with service
				$service=$an2[1];
			}
			else
				$service='';
		}
		else
		{
			$sel='property';
			$service='';
		}
			
		switch(strtolower($sel))
		{
			case 'request':
			case 'post':
			case 'session':
			case 'get':
				$var='_'.strtoupper($sel)."[\"$name\"]";
				return "\n{$pad}if(isset(\${$var}))unset(\${$var});";		
			case 'cookie':
				/* duration for cookie */
				if ($this->getAttribute($validdate,$node,'until',null,true,true))
				{
					$validdate="strtotime($validdate)";
				}
				else
				{
					// default one day 
					$validdate='strtotime($this->getProperty("cookie.duration",(string)(24*3600)))';
				}
				return "\n{$pad}setcookie($name,null,$validdate)";
				
			case 'global':
				$var="GLOBALS[\"$name\"]";
				return "\n{$pad}if(isset(\${$var}))unset(\${$var});";		
			case 'prop':
			case 'property':
				return "\n{$pad}\$this->setProperty(\"{$name}\",null);";
			case 'var':
			case 'param':
			case 'parameter':
				return "\n{$pad}if(isset(\${$name}))unset(\${$name});";		
			case 'site':
				$var="\$GLOBALS['_NX_site']";
				return "\n{$pad}{$var}->setProperty(\"{$name}\",null);";
			case 'user':
				if ($service=='')
					$service='user';
				$var="\$GLOBALS[\"_NX_$service\"]";
				return "\n{$pad}{$var}->setProperty(\"{$name}\",null);";

			default:
				return "\n{$pad}/*unset var: unknown variable $sel:$name*/";
		}
	}
		
	// default handlers
	function _tag_default($node,$level)
	{
		return true;
	}
	
	// by default we pass through unknown elements
	function tag_default($node,$content,$level,$pad,$ns,$op)
	{
		nxltrace("no handler for instruction: {$ns}:{$op}",'warning');
		
		if ($node->nodeType!=ELEMENT_NODE)
			return '';
			
		if ($ns)
			$name="$ns:$op";
		else
			$name=$op;
			
		$atts='';
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($an=='uri')
				continue;
				
			$av=$this->parseVal($av,false);			
			$atts .=" $an=\"$av\"";
		}


		// copy ns
		if ($node->nsdef)
			foreach ($node->nsdef as $ns)
			{
				if ($ns['prefix'])
					$atts .=" xmlns:{$ns['prefix']}=\"{$ns['uri']}\"";
				else
					$atts .=" xmlns=\"{$ns['uri']}\"";
			}
		
		if ($content!='')
			$res= "<{$name}{$atts}>$content\n{$pad}</$name>";
		else
			$res= "<{$name}{$atts}/>";
			
		nxltrace("passing through:".htmlentities($res),'warning');
		return $res;
	}
	
	// when debugging, if breakpoint needed, put it in these functions
	function _breakpoint_($node) 
	{
	}
	function breakpoint_($node) 
	{
	}
	
	// load a nxl file from the site root directory. Useful for generating code from NXL that is dependent on site.
	function loadConfig($node)
	{
		if ($this->getAttribute($config,$node,'preload',null,true,true))
		{
			eval('$config='.$config.';');
			$aconf=explode(';',preg_replace('/[;, ]+/',';',$config));			
			foreach ($aconf as $config)
			{
				$this->loadSiteFile("{$config}.nxl",'/',false);		
			}
			
			return true;
		}
	}
	
	// load a nxl file from the site root directory. Useful for generating code from NXL that is dependent on site.
	function loadSiteFile($file,$xpath='/',$forceReload=true)
	{	
		// get site id from url
		global $NX_DOMAIN;
		$site_id=preg_replace('#^(?:[^.]+)[.]([^/.]+)[.](?:[^/]+)#','$1',$NX_DOMAIN);
		$xpath='/';
		$level=0;
		$libPath = $this->context->tagLibPath;

			
		// get fpath and package
		$_fpath = explode('@',$file);
		if(count($_fpath)>1)
		{
			$fpath=$_fpath[0];
			$package=$_fpath[1];
			if(defined('GENERIS_PACKAGES'))
				array_unshift($libPath,GENERIS_PACKAGES.$package.'/nxl/',GENERIS_PACKAGES.$package.'/nxl/components/');
		}
		else
		{
			$fpath=$file;
			$package='';
		}

		
		// search first in current directories
        $wd=$this->context->getWorkingDir();
		if ($fpath[0]!='/' && $fpath[1]!=':')
		{
			nxltrace("loading site file=\"$fpath\" (relative) from site $site_id",'compil');
			// relative path: add working dirs in stack	
			$libPath[]=$wd;
			$libPath[]=$this->context->getCurrentDir();
			$doc_uri=$this->context->getLastResource('doc_uri',false);
			if (isset($doc_uri))
				$libPath[]=dirname($doc_uri).'/';
		}
		else
		{
			nxltrace("loading site file=\"$fpath\" (absolute path) from site $site_id",'compil');
            $p=preg_replace('%(^.*/packages/[^/]+/).*$%','$1',$wd);
            if($p!=$wd)
            {
                array_unshift($libPath,$p.'nxl/');
                array_unshift($libPath,$p.'nxl/components/');
            }
		}

		foreach ($libPath as $rootPath)
		{
			$path=$rootPath.ltrim($fpath,'/\\');
			if (!$forceReload && isset($this->context->files["{$path}_{$xpath}"]))
				return '';

			nxltrace("trying path =\"$path\" ",'compil');
			if (is_file($path))
			{					
				// get included source	
				nxltrace("<b>&lt;include file=\"$path\" xpath=\"$xpath\"&gt;</b>",'compil');
				$this->context->files["{$path}_{$xpath}"]=true;
				$res=$this->context->processFragment($path,$xpath,$level,$rootPath);
					
				nxltrace("<b>&lt;/include file=\"$path\"&gt;</b>",'compil');
				return $res;	
			}
		}
				
		// try first in site directory
		$fpath="$site_id/{$file}";							
		foreach ($libPath as $rootPath)
		{
			$path=$rootPath.$fpath;
			if (!$forceReload && isset($this->context->files["{$path}_{$xpath}"]))
				return '';
				
			nxltrace("trying path =\"$path\" ",'compil');
			if (is_file($path))
			{
				// get included source
				nxltrace("<b>&lt;load site file=\"$path\" &gt;</b>",'compil');
				$res=$this->context->processFragment($path,$xpath,$level,$rootPath,false);
							
				nxltrace("<b>&lt;/load site file=\"$path\"&gt;</b>",'compil');
				$this->context->files["{$path}_{$xpath}"]=true;
				return $res;
			}	
		}	
	
		// then try in nxfrontier site directory
		$fpath="nxfrontier/{$file}";
		foreach ($libPath as $rootPath)
		{
			$path=$rootPath.$fpath;
			if (!$forceReload && isset($this->context->files["{$path}_{$xpath}"]))
				return '';
				
			nxltrace("trying path =\"$path\" ",'compil');
			if (is_file($path))
			{
				// get included source
				nxltrace("<b>&lt;load common file=\"$path\" &gt;</b>",'compil');
				$res=$this->context->processFragment($path,$xpath,$level,$rootPath,false);
							
				nxltrace("<b>&lt;/load common file=\"$path\"&gt;</b>",'compil');
				$this->context->files["{$path}_{$xpath}"]=true;
				return $res;
			}	
		}

		nxltrace("couldnt find site file=\"$fpath\" from site $site_id",'compil');		
		return null;
	}
	
	function get_src_dir($node,$content,$level,$pad)
	{
		return $this->context->getWorkingDir();	
	}		
	
	function get_lib_info($node,$content,$level,$pad){
		if (!$this->getAttribute($sel,$node,'property'))
			$this->getAttribute($sel,$node,'p0','uri');
			
		switch($sel)
		{
			case 'uri':
				return $this->uri;
			case 'class':
				return get_class($this);
		}
		
		return '';
	}
	
	function nxltrace($node,$content,$level,$pad)
	{
		$this->getAttribute($value,$node,'value',$content);
		$this->getAttribute($level,$node,'level','trace');
		nxltrace($value,$level);
		return '';
	}

	function log($node,$content,$level,$pad)
	{
		if (!($this->getAttribute($v,$node,'value',null,true,false) ||
			  $this->getAttribute($v,$node,'p0',$content,true)))
			return $this->fatal('no value attribute or content');
			
		if (preg_match('/\$\w/',$v))
			$str=$v;
		else
			$str="'".trim($v,"'")."'";	
			
		$this->getAttribute($level,$node,'level','1',true,true);
		$this->getAttribute($loc,$node,'location','trace',true,true);
		
		return "\n{$pad}nxLog($str,$loc,$level);";
	}
	
	// =========== PACKAGE MANAGEMENT ==========
	function decodePackage($node,$default='inherit',$name_att='name',$update_name=true)
	{
		$package=null;

		// get package from name if exists
		if ($this->getAttribute($name,$node,$name_att))
		{
			// get name@$root
			$_name = explode('@',$name);
			if(count($_name)>1)
			{
				$name=$_name[0];
				$package=trim($_name[1]);
				if ($package=='')
				{
					// "name@" => start from conf root
					$package='root';
				}
				
				if ($update_name)
					$node->setAttribute($name_att,$name);
			}
		}

		if ($package==null)
		{
			$this->getAttribute($package,$node,'package',$default);		
			if ($package=='inherit')
				// inherit packahe from parent nodes
				$package=$this->context->getLastResource('package',false);
			elseif ($package=='root'||$package=='main'||$package=='default')
				// set it to root conf
				$package='root';
		}
		
		nxltrace("decodePackage = $package");
		
		if ($package)
			return strtolower($package);
		else
			return 'root';
	}


	// set property helping finding conf resources in local packages.
	// packages are group of conf descriptors bundled together fro helping
	// debugging and deployment of new sites.
	function setPackage($node)
	{
		if (!($this->getAttribute($v,$node,'value',null,true,false) ||
			  $this->getAttribute($v,$node,'p0',$content,true)))
		
			return "\n{$pad}$var->setProperty(\"package\",$v)c;";
		
		return "\n{$pad}$var->setProperty(\"package\",\"\");";
	}

	function no_tag($node,$content,$level,$pad){
		return $content;
	}
	
} ?>
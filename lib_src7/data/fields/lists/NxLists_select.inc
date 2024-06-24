<?php

// handler for building select lists from list of words.
// read words and create codes by using a dictionary.
// returns an array of the form code => word
class NxLists_select
{		
	var $index;
	var $seq;
	var $prefix;
	var $list;
	
	function __construct(&$props)
	{
		$this->props=&$props;
	}
	
		// ============= TPL MAPPING FUNCTIONS =============
	
	/**
	 * @return code
	 * @param prop property name
	 * @param dft default value
	 * @param static is executed at build time or runtime
	 * @param inPHP results to copy inside PHP section or html
	 * @desc remap a property from TPL to php
	 */
	function mapProp($p,$isDeft,$dft,$static,$inPHP)
	{
		nxLog("map property:[$p], ".($static?"static":"runtime"),"DS:XDOC");
		//$dft = addslashes($dft);
		$prop = trim($p);
		
		// mask properties starting with db (protecting db params)
		if (strstr($prop, 'db.')===0)
			return '';

		if ($static)
		{
			if ($isDeft=='=')
			{
				$v=$this->expression($dft,$static,true);
				$this->setProperty($prop,$v);
				nxLog("setting up property:[$prop]=[$v]","DS:XDOC");
				return '';
			}
			elseif ($isDeft=='|')
			{
				$v=$this->expression($dft,$static,true);
				$res=$this->getProperty($prop,$v);
			}
			else
				$res=$this->getProperty($prop,'');
		}
		else
		{
			// remove " delimiters
			$dft=rtrim(ltrim($dft,'"'),'"');
			
			$res='#q#.#$#this->getProperty("'.$prop.'","'.$dft.'").#q#';
		}

		nxLog("map property:[$p] res=[$res]","DS:XDOC");
		return $res;
	}

	function mapSystemProp($selector,$prop,$dft,$staticChar,$inPHP)
	{
		$dft = addslashes($dft);
		$prop = trim($prop);
		
		$varN='_NX_'.$selector;
		if ($staticChar=='@')
		{
			$res=$GLOBALS[$varN]->props->getProperty($prop,$dft);
		}
		else
		{
			$res='#q#.#$#GLOBALS[\''.$varN.'\']->getProperty("'.$prop.'","'.$dft.'").#q#';
		}

		nxLog("map property:{$varN}::{$prop},".($staticChar=="@"?"static":"runtime")." res=$res"	,"TPL:mapProc");
			
		return $res;
	}
	
	function expression($e,$static,$inPHP=false)
	{
		return $e;
	}
	
	function mapString($str,$pack,$static,$inPHP)
	{
		if ($static=="@")
		{
			return $this->getString($str,null,$pack);
		}
		else
		{
			return '\#q#.#$#this->getString("'.$str.'",null,"'.$pack.'").#q#';
		}
	}
	
	function &parseTPL($src)
	{
		// extract head and returns body	
		$patterns = array(
			// remove simple quotes
			"/'/",
		
			// ====  transform macros into php =====
		
			// run time variables
			'/{(@)?(?:prop(?:erty):|=)([^}|:=]+)(?:([|=])([^}]*))?}/e',
			
			// system properties {@user:user.companyName|not loggged}
			'/{(@)?(user|page|site):([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?}/e',

			// localised string
			'/{(@)?(?:string:|_)"([^"]*)"(?:[:]"?([^"}]+)"?)?}/e',
			);
			
		$replace = array(
			'&acute;',
			'$this->mapProp("$2","$3","$4","$1",false)',
			'$this->mapSystemProp("$2","$3","$4","$1",false)',	
			'$this->mapString("$2","$3","$1",false)',
			);

		// extract head, title and body code
		return  preg_replaceX($patterns, $replace,$src,$this);
	}	

	// ===================  PARSER ======================
	function &parse($path)
	{
		// read file
		$src=NxDS_Source::file_get_contents($path);
		if ($src===0)
			return false;
			
		// parse TPL
		$src = $this->parseTPL($src);
		
		// parse doc
		return $this->parseDoc($src);
	}
	
	// parse html links from file and return a record collection
	function &parseDoc($src)
	{
		nxLog("list:[$src]",'Choice:list/select');
		
		// extract options
		preg_replaceX('%<select[^>]*>(.*)</select>%sie','$this->select[]="$1";',$src,$this);
		
		if (count($this->select) == 0)
			// not html => try it as php
			return null;
			
		// get first select of the doc
		$src=$this->select[0];
		
		// tags to lowercase
		$src=preg_replace('#</option>#i','</option>',$src);
		
		// split up options
		$optionsStr = explode('</option>',$src);

		// pattern/replace arrays
		$patterns = array(
			'%value="([^"]+)"%sie',
			'%>(.*)$%sie',
			);
			
		$replace = array(
			'$this->setOptProp("node.value","$1")',
			'$this->setOptProp("node.text","$1")',
			);
		
		// get links properties: link.href, link.key, link.title, link.text
		$this->options=array();
		
		// parse each option
		foreach ($optionsStr as $optionstr)
		{
			$this->option=array();
					
			// extract option properties
			preg_replaceX($patterns, $replace,$optionstr,$this);
			
			// add option to collection as value=>text
			if (isset($this->option['node.text']))
			{
				$text=$this->option['node.text'];
				
				if (isset($this->option['node.value']))					
					$key=$this->option['node.value'];
				else
					// no value=>reformat text as key
					$key='?-'.preg_replace('#[ -]#','_',$text);

				$this->options[$key]=$text;
			}
		}

		return $this->options;
	}	
	
	function setOptProp($p,$v)
	{
		nxLog("HTML content , node[$p]=$v","DS::LIST");
		$v=str_replace('#$#','$',$v);		
		$this->option[$p]=$v;
		return '';
	}	
}

?>

<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 	transform a jsx file into a js file.
 	
 	Supports the same directive set as HTML templates (tpl) but in the form of @directive syntax.
 	ex: 
 		@property(prop) or @prop
 		@if(cond) css_action @elseif(cond) action @else action @endif
 		@include(page.othercss.css)
 		@set:variable v = �value�;
*/
class NxPlug_Template_jsx extends NxPlug_Template
{
	function __construct(&$props)
	{
		parent::__construct($props);
		$this->ext='js';
		$this->t_ext='jsx';
		$this->d_ext='jsx.php';
		$this->pathOffset='res/';
	}


	// preprocess includes: read files and insert them in source
	// NB. accept tpl and css2 type of syntax
	function processIncludes($src,$ext='',$src_path=null)
	{
		// supports the inclusion of tpl files (ex. for loading variables)
		if ($ext == 'tpl')
		{
			return parent::parse($src);
		}

		// @include directives
		$patterns = array(
			//'/{--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{!--(((.|\n)(?!--}))*)(.)?--}/s',
			'/{@?include:([^=:}]+)(:([^:{}=]+))?}/e',
			'/@include\(([^=:,)]+)(,([^,{}()=]+))?\)/e'
			);
			
		$replace = array(
			// '',
			'<!--\\1-->',
			'$this->mapInclude("$1","$2")',
			'$this->mapInclude("$1","$2")',
			);
			
		return preg_replaceX($patterns, $replace, $src,$this); 		
	}
	
	function parse($str,$ext='')
	{
		$patterns = array(
			'/<?php([^?]+)\?\>/si',
			'/@(property\()([^})|=]+)([|=]([^)]+))?\)/e',
			'/@(get|post|request|cookie|param|user|session)\(([^)}|:]+)([|:]([^)}]+))?\)/e',
			'/@(constant)[(]([^()}{|:=]+)(([|])([^()}{]+))?\)/e',
			'/@(global)[(]([^)}|:=]+)(([:=|])([^()}{]+))?\)/e',
			'/(@)(?:string\()[%"\']([^\'"]*)["\'%](,"?([^":]+)"?)?\)/e',
			'/@var(iable)?[(]([^)}|:=]+)(([:=|])([^(){}]+))?\)/e',
			'/@require[(]([^=):.}]+)[.]([^ :}{()=.]+)(:([^:(){}=]+)(:([^(){};=]+))?)?\)/e',
			'/@if[(]([^=():}]+)([:]([^:}]*)(:([^)}]+))?)?\)/e',
			'/@elseif[(]([^=:})]+)\s*([:]([^:})]*)(:([^})]+))?)?\)/e',
			'|@else|',
			'|@endif|',
			);
			
		$replace = array(
			'$0', // '$this->mapPHP("$1")',
			'$this->mapProp("$2","$4",false,false)',
			'$this->mapQProp("$1","$2","$4",false,false)',
			'$this->mapK("$2","$4","$5",false,false)',
			'$this->mapGlobal("$2","$4","$5",false,false)',
			'$this->mapString("$2","$4","$1",false)',
			'$this->mapLocal("$2","$4","$5",false,false)',
			'$this->mapReq("$1","$2","$4","$6")',
			'$this->mapIf("$1","$3","$5")',
			'"<?php }else ?>".$this->mapIf("$1","$3","$5")',
			'<?php 
		} /*if*/
		else
		{ 
		?>',
			'<?php 
		} /*if*/ 
		?>',
			);
			
		return $this->cleanup(preg_replaceX($patterns, $replace,$str,$this)); 	
	}
	
	function cleanup($s)
	{
		$matches=array(
			'/<\?=\s*(.*[^\s]+)\s*\?>/',
			'/\?>(\s*)<\?(php)?([^=])/'
			);
		
		$replace=array(
			'<?php echo \\1; ?>',
			'\\1\\3'
			);
		
		return preg_replace($matches, 
					$replace,
					$s);			
	}
}


?>
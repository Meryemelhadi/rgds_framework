<?PHP

// tag lib for the aspect : data record. Generates the class handling the data record aspect,
// in /nx/conf/aspects/records/
// uri : xmlns:tpl="http://www.nxfrontier.com/aspects/records"

class taglib_nxfrontier_aspects_records extends NxTagLib
{		
	function _aspect(&$node) {
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		

		global $NX_LANG;
		global $_NX_site;
		$this->getAttribute($name,$node,'name','default',true,false);
		$name=strtr($name,'.','/');
		
		$this->context->addResource("aspect_{$name}@$package",'aspect_name');
		
		nxltrace("Create aspect : record {$name}@$package","trace");
		
		$this->context->addResources($functions,"aspect_{$name}@$package");
	}
	
	function aspect($node,&$content,$level,$pad)
	{		
		global $NX_LANG;
		global $_NX_site;
		
		$aspect_name=$this->context->getLastResource('aspect_name',true);
		$this->getAttribute($name,$node,'name','default',true,false);
		$name=strtr($name,'.','/');
		$className=strtr($name,'/','_');
		$path=strtr($name,'.','/');

		// get root directory for application (and pop package)
		$package=$this->context->getLastResource('package',true);
		if ($package=='root')
			$rootPath=NX_CONF.'aspects/';
		else
			$rootPath=NX_CONF."packages/{$package}/aspects/";
		
		// get strings
		$functions=$this->context->getResources($aspect_name);

		// sort($functions);
		$count=count($functions);
		if ($count==0)
			nxltrace("no aspect handler found... using default functions","trace");	
		
		// create content source
		$content=<<<EOH

  class aspects_records_$className extends NxModule {
	  var \$record;

  	  function __construct(&\$record,&\$module){
  	  	  parent::__construct(\$module->props,\$module->parent);
  	  	  \$this->attachTo(\$module);
  	  	  \$this->record = &\$record;
	  }
	  function onNew(&\$errorCB) {
	  	\$record = &\$this->record;

EOH;

if (isset($functions['onnew']))
	$content .= $functions['onnew'];
else
	$content .= <<<EOH
	  	  nxLog("onNew",'aspects');
EOH;
unset($functions['onnew']);

$content .= <<<EOH
		return true;

	  }

	  function onBeforeSave(\$isInsert,&\$errorCB) {
	  	\$record = &\$this->record;
	  
EOH;

if (isset($functions['onbeforesave']))
	$content .= $functions['onbeforesave'];
else
	$content .= <<<EOH
	  	  nxLog("onBeforeSave : ",'aspects');
EOH;
unset($functions['onbeforesave']);

$content .= <<<EOH
		return true;

	  }
	  function onSave(\$isInsert,&\$errorCB) {
	  	\$record = &\$this->record;
	  
EOH;

if (isset($functions['onsave']))
	$content .= $functions['onsave'];
else
	$content .= <<<EOH
	  	  nxLog("onSave",'aspects');
EOH;
unset($functions['onsave']);

$content .= <<<EOH

	  }

	  function onBeforeDelete(&\$errorCB) {
	  	\$record = &\$this->record;
	  
EOH;

if (isset($functions['onbeforedelete']))
	$content .= $functions['onbeforedelete'];
else
	$content .= <<<EOH
  	  nxLog("onBeforeDelete",'aspects');
EOH;
unset($functions['onbeforedelete']);

$content .= <<<EOH
		return true;
	  
	  }
	  function onDelete(&\$errorCB) {
	  	\$record = &\$this->record;
	  
EOH;

if (isset($functions['ondelete']))
	$content .= $functions['ondelete'];
else
	$content .= <<<EOH
	  	  nxLog("onDelete",'aspects');
EOH;
unset($functions['ondelete']);
$content .= <<<EOH

	  }

EOH;

foreach ($functions as $f=>$code)
{
$content .= <<<EOH
	  function $f($params) {
		\$record = &\$this->record;

		$code

	  }

EOH;
}


$content .= <<<EOH

}

EOH;
		
		nxltrace("Added aspect : {$name}@$package=$content","listing");

		// add library headers
		$imports=array_unique($this->context->getResources('import',null,true));
		$src="\n";
		
		foreach ($imports as $import)
		{
			$src .= $import."\n";
		}

		$content = '<?php '.$src.$content.'?>';
		
		// save class
		$this->context->saveSrc($rootPath."records/{$path}.inc",$content);
	}
	
	function _tag_default(&$node,&$level)
	{		
		// reset the automatic node level used for indentation
		$level=1;

		$this->context->declareBlock($node,true);
		nxltrace("<$id>","code");
		
	}

	function tag_default($node,&$content,$level,$pad)
	{		
		// get id and value
		$id=strtolower($node->localName);

		// get root directory for application (and pop package)
		$functions_name=$this->context->getLastResource('aspect_name',false);
		$this->context->addResource($content,$functions_name,$id);

		nxltrace("$content</$id>","code");
		$this->context->declareBlock($node,false);
	}
}

?>
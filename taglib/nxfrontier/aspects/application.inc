<?PHP

// tag lib for the aspect : data record. Generates the class handling the data record aspect,
// in /nx/conf/aspects/records/
// uri : xmlns:tpl="http://www.nxfrontier.com/aspects/records"

class taglib_nxfrontier_aspects_application extends NxTagLib
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
		
		nxltrace("Create aspect : application {$name}@$package","trace");
		
		$this->context->addResources($functions,"aspect_{$name}@$package");
	}
	
	function aspect($node,&$content,$level,$pad)
	{		
		global $NX_LANG;
		global $_NX_site;
		
		$aspect_name=$this->context->getLastResource('aspect_name',true);
		$this->getAttribute($name,$node,'name','default',true,false);
		$name=strtr($name,'.','/');
		$className=strtr($name,array('/'=>'_','.'=>'_'));
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
		
  class aspects_application_$className extends NxModule {
	  var \$record;

  	  function aspects_application_$className(&\$application){
  	  	  \$this->NxModule(\$application->props,\$application->parent);
  	  	  \$this->attachTo(\$application);
  	  	  \$this->application=&\$application;
	  }
	  function onStartOperation() {
	  
EOH;

if (isset($functions['onstartoperation']))
	$content .= $functions['onstartoperation'];
else
	$content .= <<<EOH
	  	  nxLog("onStartOperation",'aspects');
EOH;

$content .= <<<EOH
		return true;

	  }

	  function onFinishOperation()
	  {
	  	  $application=$this->application;
EOH;

if (isset($functions['onfinishoperation']))
	$content .= $functions['onfinishoperation'];
else
	$content .= <<<EOH
	  	  nxLog("finish operation {\$application->appName}::{\$application->operation}.{\$application->step}  @{\$application->package}",'aspect');
EOH;

$content .= <<<EOH
	  }

	  function onBeforeRun(\$name,\$module)
	  {
	  	  $application=$this->application;	  
EOH;

if (isset($functions['onbeforerun']))
	$content .= $functions['onbeforerun'];
else
	$content .= <<<EOH
	  	  nxLog("{\$application->appName}::{\$application->operation}.{\$application->step} @{\$application->package} : onBeforeRun \$name",'aspect');
EOH;

$content .= <<<EOH
	  	  return true;

	  }

	  function onRun(\$name,\$module)
	  {
	  	  $application=$this->application;
EOH;

if (isset($functions['onrun']))
	$content .= $functions['onrun'];
else
	$content .= <<<EOH
	  	  nxLog("{$application->appName}::{$application->operation}.{$application->step} @{$application->package} : onRun $name",'aspect');
EOH;

$content .= <<<EOH
	  
	  }
  }
EOH;
		
		nxltrace("Added application aspect : {$name}@$package=$content","listing");

		// add library headers
		$imports=array_unique($this->context->getResources('import',null,true));
		$src="\n";
		foreach ($imports as $import)
		{
			$src .= $import."\n";
		}

		$content = '<?php '.$src.$content.'?>';
		
		// save class
		$this->context->saveSrc($rootPath."application/{$path}.inc",$content);
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
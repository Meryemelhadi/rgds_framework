<?PHP

define('NXL_COMMENT',false);

class NXL_genSteps extends NxTagLib_context
{
	function __construct(&$props,&$dom,$path)
	{
		parent::__construct($props,$dom,NXL_ROOTS,NXL_COMMENT);
		$this->path=$path;
	}
	
	/* gen actual processing logic
	   ====================================================== */
	function genSteps($apps)
	{
		foreach ($apps as $pname=>$app)
		{					
			foreach($app['operations'] as $op)
			{
				foreach($op['steps'] as $sname=>$step)
				{
					$this->genStep($step);
				}
			}
			foreach($app['modules'] as $module)
			{
				$this->genStep($module);
			}
		}
	}
	
	function genStep($step)
	{
		$fullpath=trim($step['path'],'/').'/'.$step['name'];
		$packagepath=str_replace('/','.',$fullpath);
		$sname=$step['name'];
				
		$this->fs->saveFile($this->path.$fullpath.'.php',
							$this->genStepSrc($step));
	}
	
	function genStepSrc($step)
	{
		$fullpath=trim($step['path'],'/').'/'.$step['name'];
		$class_name=str_replace('/','_',$fullpath);	
		$packagepath=str_replace('/','.',$fullpath);
		//$oname=$op['name'];		
		$sname=$step['name'];

		$code = $this->processChildren($step['prog'],0);
		
		$src='<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.\'module/NxPage.inc\');
';

		// add library headers
		foreach ($this->lib as $lib)
		{
			if (method_exists($lib,'header'))
				$src .= $lib->header()."\n";
		}
		
		$src.=
"class $class_name extends NxPage
{
	function $class_name".'($props,&$parentDesc)
	{
		parent::__construct($props,$parentDesc);
	}

	function run()
	{';

	$src .= $code;
	
	$src .='
		return true;
	}
}
?>';
		return $src;
	}
}
?>
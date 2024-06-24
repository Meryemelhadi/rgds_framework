<?php

include_once('application_base.inc');

class application_angular_hp extends application_base
{
	function generate($node,$parent) {
		$lib = $this->lib;
		$model = $this;

		// decode package (and remove package suffix on "logic" param)
		$package=$lib->decodePackage($node,'inherit','logic',true);	
		
		// get cache mode
		$iscache=$lib->getBool($lib->getLastResource('page_cache',true));
		
		if(!$parent)
			$parent=$lib->context->getParentElement($node);

		// get params
		$params=$lib->context->getResources('params',"\n");
		
		// get path
		$path=$lib->getLastResource('path',true);
		
		// get name
		$lib->getAttribute($name,$node,'name','index');				

		$lib->getAttribute($page,$parent,'page',$name);
		$lib->getAttribute($application,$node,'application',$name);
		$lib->getAttribute($table,$node,'table',$application);
		$lib->getAttribute($base_data,$node,'base_data',$table);
        $lib->getAttribute($strings,$node,'strings',$application);
        $lib->getAttribute($skin,$node,'skin','default');
        $lib->getAttribute($string_base,$node,'string_base','record');
        
        $lib->getAttribute($default_op,$node,'default_operation','list');

        $lib->getAttribute($role,$node,'role','default');
        $lib->getAttribute($isCreator,$node,'isCreator',false);
		$isCreator=($isCreator!='false')&&($isCreator!='no');

        $lib->getAttribute($isValidator,$node,'isValidator',false);
		$isValidator=($isValidator!='false')&&($isValidator!='no');

		// prepare string mapping for model->clone copy
		$lib->getAttribute($string_mapping,$node,'string_mapping','');
		$map = array('/recruitment3/'=>$package,'/crm/'=>$name);

        $lib->getAttribute($defaultModule,$node,'default_module','');
		if($defaultModule)
			$map["|{$defaultModule}|"]= $defaultModule;

		$stringsMap = explode('|',$string_mapping);
		foreach($stringsMap as $stringMap)
		{
			$a = explode('=',$stringMap);
			if(count($a)>1)
			{
				$map['|'.$a[0].'|']=$a[1];
			}
		}

		$this->deployAndMap($package,$application,$map);
		$this->genNxlAll($package,$page);
	}
}

?>
<?PHP

class NXL_build
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}


	function build($module,$package='')
	{
		// get paths
		$srcPath = $this->getSrcPath($module,$package);
		$exePath = $this->getExePath($module,$package);

		// up to date ?
		if (is_file($exePath))
			// deployed once
			if (NX_CHECK_CONTENT=='once')
				return $exePath;
			// build refresh
			else if (NX_CHECK_CONTENT!='compile' && // build
					(is_file($srcPath) &&
					($ftimeSrc = @filemtime($srcPath)) && 
					($ftimeExe = @filemtime($exePath)) && 
					($ftimeExe-$ftimeSrc)>0 && 
					!isset($_GET["nocache"])))
				return $exePath;
						
		// compile application logic
		$this->compile($module,$srcPath,$exePath);
					
		return $exePath;
	}	

	function getSrcPath($module,$package='')
	{
		$basepath=str_replace('.','/',$module);

		if ($package)
		{
			$root="{$package}/logic/";
			if (// module
				is_file($f=NX_CONF."{$root}$basepath.php") ||
				is_file($f=NX_CONF."{$root}$module.nxl") ||
				is_file($f=NX_DIS_CONF."{$root}$basepath.php") ||
				is_file($f=NX_DIS_CONF."{$root}$module.nxl")
				)
				return $f;
		}

		$root="logic/";	
		
		if (// module
			is_file($f=NX_CONF."{$root}$basepath.php") ||
			is_file($f=NX_CONF."{$root}$module.nxl") ||
			is_file($f=NX_DIS_CONF."{$root}$basepath.php") ||
			is_file($f=NX_DIS_CONF."{$root}$module.nxl")
			)
			return $f;
			
		nxError("cant find logic module: $module");
		return null;
	}

	function getExePath($module,$package)
	{
		// set file root (from package)
		if ($package==''||$package=='root')
			$fileRoot=NX_CONF;
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$fileRoot=GENERIS_PACKAGES.$package.'/conf/';
		else
			$fileRoot=NX_CONF.'packages/'.$package.'/';

			
//		return NX_GEN."{$root}logic/".str_replace('.','/',$module).'.php';
//		return NX_CONF."{$root}logic/".str_replace('.','/',$module).'.php';
		return $fileRoot."logic/".str_replace('.','/',$module).'.php';
	}
	
	function compile($module,$srcPath,$exePath)
	{
		if (preg_match('/[.]nxl$/',$srcPath))
		{
			// nxl
			include(NX_LIB.'mvc/NXL/NXL_processor.inc');	
			$xml_parser = new NXL_processor($this->props);
			if ($xml_parser->build($module,$srcPath) && is_file($exePath))
				return $exePath;
		}
		else
		{
			//php
			$fs=$this->props->getPlugIn('FileSys');
			if ($fs->copy($srcPath,$exePath))
				return $exePath;
		}
		return null;
	}
}
?>
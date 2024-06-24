<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_FileSys
{
	var $isWin;
	
	function __construct($props=null)
	{
		// $this->props=&$props;
		if (substr(php_uname(), 0, 7) == "Windows")
			$this->isWin=true;
		else
			$this->isWin=false;
	}

	// reformat path separators
	function formatPath($path) {
	
		if ($this->isWin==true)
		{
			$path2=iconv('UTF-8', 'ISO-8859-1//TRANSLIT', $path);
			if(!$path2)
				$path2 = utf8_decode($path);
				
			return str_replace(
				array('//','/','\\/'),array('/','\\','\\'),$path2);
		}
		else
			return str_replace('//','/',$path);
	}

	function encodeToFs($path) {
	
		if ($this->isWin==true)
			return iconv('UTF-8', 'ISO-8859-1//TRANSLIT',$path);
		else
			return $path;
	}
	function decodeFromFs($path) {
	
		if ($this->isWin==true)
			return iconv('ISO-8859-1//TRANSLIT','UTF-8',$path);
		else
			return $path;
	}
	
	// =============== SYSTEM UTILITIES ===================
	function &file_get_contents($filename,$attr="r") 
	{
		$fp = @fopen(NxPlug_FileSys::formatPath($filename), $attr);
		if (!($fp))
			return 0;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		fclose($fp);
		return $temp;
	}

	function pathinfo($path,$options=null)
	{
		 $path = NxPlug_FileSys::formatPath($path);
         if($options===null)         
            return @pathinfo($path);
            
         return @pathinfo($path,$options);
	}
	function is_file($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 return @is_file($path);
	}
	function file_exists($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 return @file_exists($path);
	}
	function is_dir($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 return @is_dir($path);
	}
	function filemtime($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 return @filemtime($path);
	}
	function md5_file($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 return @md5_file($path);
	}
	function stat($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 return @stat($path);
	}
	function realpath($path)
	{
		 $path = NxPlug_FileSys::formatPath($path);
		 $pathOs = @realpath($path);
		 if($pathOs)
			 return $this->decodeFromFs($pathOs);

		 return null;
	}

	// ========= Tools =========
	//! directory/file functions
	function createTree($path,$mode=0777,$asOs=false)
	{
		// to be fixed...
		 nxLog("create path tree:[$path]",'TPL',6);

		if ($this->isWin==true)
			$sep='\\';
		else
			$sep='/';
		 
		 if(!$asOs)
			$path = NxPlug_FileSys::formatPath($path);

		 $basepath = NxPlug_FileSys::formatPath(NX_HOST_ROOT);
		 
		 // check if path part of host dir tree
		 if (strstr(strtolower($path),strtolower($basepath))===false)
		 {
			nxError("can't create path tree:[$path] because it is not part of tree root:$basepath",'TPL');
			return null;
		 }
		 	
	 	 $relPath=substr($path,strlen($basepath));
		 
		 $dirs = explode("/",strtr($relPath,'\\','/'));
		 $path = rtrim($basepath,'/\\');

		 $oldumask = umask(0);
		 nxLog("umask:[$oldumask]",'TPL');
		 
		 for($i = 0;$i < count($dirs);$i++) 
		 {
		 	if (!empty($dirs[$i]))
		 	{
				$path .= $sep.$dirs[$i];
				if(!is_dir($path))
				{
					 nxLog("creating directory :[$path]",'TPL');

					if (!@mkdir($path,$mode))
					{
						nxError("Error, can't create path:".$path." mask:$mode, umask:".umask(),"TPL");
						return null;
					}
					else
						nxLog("created dir $path","NxPlug_Template",NX_COMPILE_EVENT);
				}
		 	}
		 }

		 umask($oldumask);
		 return $path.$sep;
	}	
	
	//! delete file
	function delete($f,$asOs=false){	
		if(!$asOs)
			$f = NxPlug_FileSys::formatPath($f);
						
		if (!is_file($f))
			return false;
		
		if (@unlink($f))
			return true;

		$this->chmod($f,0777);
		if (@unlink($f))
			return true;
			
		return false;
	}	
	
	//! cleanup dir content
	function cleanDir($dir) {	
		$dir = NxPlug_FileSys::formatPath($dir);
			
		if (!is_dir($dir))
			return false;
			
		$this->chmod($dir,0777);

	    $handle = opendir($dir); 
	    while($filename = readdir($handle))
	       if ($filename != "." && $filename != "..")
	         unlink("{$dir}/{$filename}");
	     closedir($handle);
	     return true;
	}

	//! deep removal of dir
	function rmDir($dirname,$deep=true,$asOs=false){    
		
		if(!$asOs)
			$dirname = NxPlug_FileSys::formatPath($dirname);

		// Simple delete for a file    
		if (is_file($dirname)) 
			return $this->delete($dirname,true);    

		if (!is_dir($dirname))
			return false;
			
		$this->chmod($dirname,0777);
		
		// Loop through the folder    
		$dir = opendir($dirname);
		if($dir===false)
		{
			nxError('rmDir failed for [$dirname] : check perms?');
			return false;
		}

		$n = 0;
		while (false !== $entry = readdir($dir)) 
		{
			// Skip pointers        
			if ($entry == '.' || $entry == '..')
				continue;        
       
			$n++; 

			// Deep delete directories              
			if($deep)
			{
				if (is_dir("$dirname/$entry"))
					$this->rmDir("$dirname/$entry",$deep,true);
				elseif(is_file("$dirname/$entry"))
					$this->delete("$dirname/$entry",true);
			}
		}
		
		// Clean up    
		closedir($dir);

		if($deep || $n==0)
			return rmdir($dirname);
		else
			return false;
	}
	
	function copy($f,$dest,$asOs=false)
	{
		if(!$asOs)
		{
			$f = NxPlug_FileSys::formatPath($f);
			$dest = NxPlug_FileSys::formatPath($dest);
		}

		$d = dirname($dest);
		
		if(is_dir($d))
			if (@copy($f, $dest))
				return true;
			
		// failed : try to change permissions (parent dir)
		if (!is_dir($d) && !$this->createTree($d,0777,true))
			return false;
			
		$this->chmod($d,0777);
		if (is_file($dest))
			$this->chmod($dest,0666);
			
		return @copy($f,$dest);
	}

	// rename : move file to another location and name. $dest is a file name
    function rename($f,$dest,$asOs=false)
    {
		if(!$asOs) 
		{
			$f = NxPlug_FileSys::formatPath($f);
			$dest = NxPlug_FileSys::formatPath($dest);
		}
        
        if (is_dir($d=dirname($dest)))
			if (@rename($f, $dest))
				return true;
            
        // failed : try to change permissions (parent dir)
        if (!is_dir($d=dirname($dest)) && !$this->createTree($d,0777,true))
            return false;
            
        $this->chmod($d,0777);
        if (is_file($dest))
            $this->chmod($dest,0666);

        return @rename($f,$dest);
    }
    
	// rename : move file to another directory. $dest is a directory name
	function move($f,$dest)
	{
		$f = NxPlug_FileSys::formatPath($f);
		$dest = NxPlug_FileSys::formatPath($dest);
		if (($e=substr($dest,-1))=='/' || $e='\\')
			$dest .= basename($f);
            
        return $this->rename($f,$dest);
	}

	//! deep copy of dir
	function copyDir($dirname,$toDir,$mode=0777,$overload=false)
	{    	
		$dirnameOs = NxPlug_FileSys::formatPath($dirname);
		$toDirOs = NxPlug_FileSys::formatPath($toDir);

		// Simple delete for a file    
		if (is_file($dirnameOs)) 
			return $this->copy($dirname,$toDir);

		if (!is_dir($dirnameOs))
			return false;
			
		$this->createTree($toDir,$mode);
		
		// Loop through the folder    
		$dir = opendir($dirnameOs);
		while (false !== $entry = readdir($dir)) 
		{
			// Skip pointers        
			if ($entry == '.' || $entry == '..')
				continue;        
       
			// Deep copy directories              
			if (is_dir("$dirnameOs/$entry"))
				$this->copyDir("$dirname/$entry","$toDir/$entry",$mode);
			elseif(is_file("$dirname/$entry"))
				if ($overload || !is_file("$toDirOs/$entry"))
					$this->copy("$dirname/$entry","$toDir/$entry");
		}
		
		// Clean up    
		closedir($dir);    
		return true;
	}	
	
	function fopen($path,$mode,$createTree=false)
	{
		$path = NxPlug_FileSys::formatPath($path);
		
		if (is_file($path)&&($fd=@fopen($path,$mode)))
			return $fd;
			
		// failed : try to create the directory
		if (!is_dir($d=dirname($path)))
		{
			if (strstr($mode,'w')===FALSE || !$createTree)
				return false;
		
			if ($this->createTree(dirname($path))==null)
				return false;
		}
			
		$this->chmod($d,0777);
		if (is_file($path))
			$this->chmod($path,0666);
			
		return fopen($path,$mode);
	}
	
	function chmod($f,$p)
	{
		if (substr(php_uname(), 0, 7) == 'Windows')
			return;
		
		if (system('/bin/chmod '.decoct($p)." $f")===false)
		{
			nxError("operation failed: chmod on file $f",'TPL');
			return false;
		}
		else
		{
			nxLog("chmod on file $f succeeded",'TPL');
			return true;
		}
	}
	
	function saveFile($path,$src,$flush=true,$mode='wb')
	{
		$path = NxPlug_FileSys::formatPath($path);		
		
		// check if output directory exist
		if (!is_dir($d=dirname($path)))
			if ($this->createTree($d,0777,true)==null)
			{
				nxError('cant create directory :'.$d);
				return false;
			}
			
		// transform it and write to file
		nxLog("creating file $path with content length=".strlen($src),'TPL');
		
		if (!$fp = fopen($path,$mode))
		{
			$this->delete($path,true);
			$fp = fopen($path,$mode);
		}

		if ($fp)
		{
			// parse and copy to file
			if (fputs($fp,$src)===false)
				nxError('write error to file :'.$path);
				
			if ($flush)
				fflush($fp);
			
			fclose($fp);			
			return true;
		}
		else 
		{
			nxError('cant save content executable :'.$path);
			return false;
		}		
	}
	
	function listFiles($path,$levels=50,$files=true,$dirs=false)
	{
		$pathOs = NxPlug_FileSys::formatPath($path);		

		$list=array();
		if (!is_dir($pathOs))
			return $list;

		$handle=@opendir($pathOs);
		if ($handle === false)
		{
			nxError("can't list file, unknown path:$pathOs",'FS');
			return $list;
		}

		while($a=@readdir($handle))
		{
			if(!preg_match('/^\./',$a))
			{
				$a = $this->decodeFromFs($a);

				$full_pathOs="$pathOs$a";
				$full_path="$path$a";
				if(is_file($full_pathOs))
				{
					if($files)
						$list[]=$full_path; 
				}
				elseif(is_dir($full_pathOs))
				{
					if($dirs)
						$list[]=$full_path.'/';
                        
                    if($levels>1) 
                    {
					    $recursive=$this->listFiles("{$full_path}/",$levels-1,$files,$dirs);
					    $list = array_merge($list,$recursive);
                    }
				}
			}
		}

		closedir($handle);
		return $list;
	}

	function listFilesRegEx($path,$re,$levels=50,$files=true,$dirs=false)
	{
		$list=array();
        $lf=$this->listFiles($path,$levels,$files,$dirs);
		foreach ($lf as $f)
		{
			if ($res=preg_match($re,$f))
				$list[]=$f;

			if ($res === false) 
			 {
			   $error = error_get_last();
			   DebugBreak();
			   nxError("listFilesRegEx : re=$re");
			 }
		}
		return $list;
	}
}

?>
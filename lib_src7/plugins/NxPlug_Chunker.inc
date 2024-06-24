<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_Chunker
{
	function  NxPlug_Chunker(&$props)
	{
		$this->props=&$props;
	}
	
	// =============== SYSTEM UTILITIES ===================
	function &file_get_contents($filename) 
	{
		$fp = @fopen($filename, "r");
		if (!($fp))
			return 0;
			
		$temp='';
		while (!feof($fp))
		{
			$temp .= fread($fp, 4096);
		}

		return $temp;
	}

	// ========= Tools =========
	// directory/file functions
	function createTree($path,$mode = 0777)
	{
		// to be fixed...
		 nxLog("create path tree:[$path]",'TPL',6);
		 
		 $path = strtr($path,"\\","/");
		 $basepath = strtr(NX_HOST_ROOT,"\\","/");
		 
		 // check if path part of host dir tree
		 if (strstr($path,$basepath)===false)
		 {
			nxError("can't create path tree:[$path] because it is not part of tree root:$basepath",'TPL');
			return null;
		 }
		 	
	 	 $relPath=substr($path,strlen($basepath));
		 
		 $dirs = explode("/",$relPath);
		 $path = rtrim($basepath,'/');

		 $oldumask = umask(0);
		 nxLog("umask:[$oldumask]",'TPL');
		 
		 for($i = 0;$i < count($dirs);$i++) 
		 {
		 	if (!empty($dirs[$i]))
		 	{
				$path .= '/'.$dirs[$i];
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
		 return $path."/";
	}	
	
	function copy($f,$dest)
	{
		if (@copy($f, $dest))
			return true;
			
		// failed : try to change permissions (parent dir)
		if (!is_dir($d=dirname($dest)) && !$this->createTree($d))
			return false;
			
		$this->chmod($d,0777);
		if (is_file($dest))
			$this->chmod($dest,0666);
			
		return copy($f,$dest);
	}
	
	// cleanup dir content
	function delete($f) {					
		if (!is_file($f))
			return false;
		
		if (@unlink($f))
			return true;

		$this->chmod($f,0777);
		if (@unlink($f))
			return true;
			
		return false;
	}	
	
	// cleanup dir content
	function cleanDir($dir) {		
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

	// deep removal of dir
	function rmDir($dirname){    
		// Simple delete for a file    
		if (is_file($dirname)) 
			return unlink($dirname);    

		if (!is_dir($dirname))
			return false;
			
		$this->chmod($dirname,0777);
		
		// Loop through the folder    
		$dir = opendir($dirname);
		while (false !== $entry = readdir($dir)) 
		{
			// Skip pointers        
			if ($entry == '.' || $entry == '..')
				continue;        
       
			// Deep delete directories              
			if (is_dir("$dirname/$entry"))
				$this->rmDir("$dirname/$entry");        
			else            
				unlink("$dirname/$entry");			    
		}
		
		// Clean up    
		closedir($dir);    
		return rmdir($dirname);
	}
	
	function fopen($path,$mode,$createTree=false)
	{
		if ($fd=@fopen($path,$mode))
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
	
	function saveFile($path,$src,$flush=true)
	{
		// check if output directory exist
		if (!is_dir($d=dirname($path)))
			if ($this->createTree($d)==null)
			{
				nxError('cant create directory :'.$d);
				return false;
			}
			
		// transform it and write to file
		nxLog("creating file $path with content length=".strlen($src),'TPL');
		
		if ($fp = fopen($path,'w'))
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
	
	function chunk($path_file,$to_dir,$basefilename,$ext='',&$log_str,$chunksize=20,$perms=0777) 
	{
		$log_str='';
		
		$to_dir=rtrim($to_dir,'/').'/';
		
		//initialize vars
		$begin=time(); // script start time
		$start = time(); // last gate time
		$interval=time(); // current gate time
		$minutes=1; // intervals for gates
		$filenum = 1; // start chunk file number at 1
		$recordnum = 0; // start at record 1
	
		//start processing
		$log_str.="Processing ({$path_file})\n";
		$suffix=str_pad("$filenum",3,"0",0);
		$exportfile = "{$to_dir}{$basefilename}-{$suffix}.{$ext}";
		
		$handle = @fopen($path_file,"r");
		
		if ($handle) 
		{
			while (!feof($handle)) 
			{	
				$recordnum++;
				$buffer = fgets($handle, 4096);
					
				//write line to chunk file
				$output .= $buffer;
			
				// if chunk limit reached then start to 
				// close the file with well formed xml
				if ($recordnum>$chunksize) {
			
					// save chunk and resetr output buffer
					$suffix=str_pad("$filenum",4,"0",0);
					$exportfile = "{$to_dir}{$basefilename}-{$suffix}.{$ext}";
					$this->saveFile($exportfile,$output,true);
					$this->chmod($exportfile,$perms);

					$output='';
					
					// and increment file number to start new log file chunk
					//reset record counter number for new chunk file
					$recordnum=0;
					$filenum++;
			
					//update export file name
					$suffix=str_pad("$filenum",4,"0",0);
					$exportfile = "{$to_dir}{$basefilename}-{$suffix}.{$ext}";
			
					//echo status report to STDOUT
					$log_str.="Segment $filenum. Record ".($chunksize*$filenum).".\n";
				}
				
				//put in a catch so that script doesn't run riot and
				//will die after X number of cycles
				if ($filenum>5000) {
					die();
				}
				
				if (($interval-$start)>60) {
					$minutes++;
					$log_str.= $minutes." Minutes so far.\n";
					$start=time();
				} else {
					$interval = time();
				}
			}
			fclose($handle);
			
			if ($recordnum>0){
				$this->saveFile($exportfile,$output,true);
				$this->chmod($exportfile,$perms);
			}

			$procend = time();
			$log_str.= "\n####\n";
			$log_str.= "Split Complete (".floor((($procend-$begin)/60))." Minutes)\n";
			return true;
		} 
		else 
		{
			$log_str.="Unable to open file! ({$path_file})\n";
			return false;
		}
	}
	
	function listFiles($path,$levels=50)
	{
		 $list=array();
		 if (!is_dir($path))
		 	return $list;
		 	
	     $handle=@opendir($path);
	     if ($handle === false)
	     {
	     	nxError("can't list file, unknown path:$path",'FS');
	     	return $list;
	     }
	     	
	     while($a=@readdir($handle)){
	         if(!preg_match('/^\./',$a)){
	               $full_path="$path$a";
	               if(is_file($full_path))
	               { 
	               		$list[]=$full_path; 
	               }
	               elseif(is_dir($full_path) && $levels>1)
	               {
	                   $recursive=$this->listFiles("{$full_path}/",$levels-1);
	                   $list = array_merge($list,$recursive);
	               }
	         }
	     }
	     closedir($handle);
	     sort($list);
	     return $list;
	}
	
	function listFilesRegEx($path,$re,$levels=50,$max=0,$cache=true)
	{
		static $fcache = array();
		
		if ($cache && isset($fcache[$path]))
		{
			$files=$fcache[$path];
		}
		else
		{
			$files=$fcache[$path]=$this->listFiles($path,$levels);			
		}
		
		$list=array();
		$i=0;
		foreach ($files as $f)
		{
			if (preg_match($re,$f))
			{
				$list[]=$f;
				++$i;
				if ($max>0 && $i>=$max)
					break;
			}
		}
		return $list;
	}
}

?>
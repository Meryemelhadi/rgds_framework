<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// portal user file space
class NxPlug_UserSpace
{
	var $app;
	function __construct(&$props)
	{
		$this->prop =& $props;
		// $this->usersRoot = $this->prop->getProperty("users.space.root",rtrim(NX_PUBLIC_USERS,"/"))."/";
		$this->usersRoot = NX_PUBLIC_USERS;
	}
	
	function getUserSpaceRoot($userId)
	{
		return  $this->usersRoot.$userId;
	}
	
	function createUserSpace($userId)
	{
		$userSpaceRoot = $this->usersRoot. $userId;
		$this->createTree($userSpaceRoot,0775);
	}
	
	function deleteUserSpace($userId)
	{
		$this->deleteTree($this->usersRoot.$userId);
	}

	function getUserDiskSpace($userId)
	{
		return $this->dirSize($this->usersRoot.$userId);
	}
	
	function realpath($path) {
		 // $path = strtr(realpath($path),"\\","/");
		 return $path;
	}
	
	// directory/file functions
	function createTree($path,$mode = 0775)
	{
		// to be fixed...
		 $path = $this->realpath($path);
		 $dirs = explode("/",$path);
		 $path = $dirs[0];

		 $oldumask = umask(0);
		 
		 for($i = 1;$i < count($dirs);$i++) 
		 {
		 	if (!empty($dirs[$i]))
		 	{
				$path .= "/".$dirs[$i];
				if(!is_dir($path))
					if (!@mkdir($path,$mode))
					{
						nxLog("Error, can't create path:".$path." mask:$mode, umask:".umask(),"UserSpace");
					}
		 	}
		 }

		 umask($oldumask);
		 return $path."/";
	}
	
	function deleteTree($file) 
	{
		if (file_exists($file)) 
		{
			chmod($file,0777);
			if (is_dir($file))
			{
				$handle = opendir($file);
				while($filename = readdir($handle)) 
				{
					if ($filename != "." && $filename != "..")
						delete($file."/".$filename);
				}
				closedir($handle);
				rmdir($file);
			}
			else
				unlink($file);
		}
	}
	
	function dirSize($directory)
	{
		if (!is_dir($directory)) return -1;
		$size = 0;
		if ($DIR = opendir($directory))
		{
			while (($dirfile = readdir($DIR)) !== false)
			{
				if (is_link($directory . '/' . $dirfile) || $dirfile == '.' || $dirfile == '..')
					continue;
					
				if (is_file($directory . '/' . $dirfile))
				$size += filesize($directory . '/' . $dirfile);
				else if (is_dir($directory . '/' . $dirfile))
				{
					$dirSize = dirSize($directory . '/' . $dirfile);
					if ($dirSize >= 0) $size += $dirSize;
					else return -1;
				}
			}
			closedir($DIR);
		}
		return $size;
	}

	function formatFileSize($rawSize)
	{
		if ($rawSize / 1048576 > 1)
		return round($rawSize/1048576, 1) . 'MB';
		else if ($rawSize / 1024 > 1)
		return round($rawSize/1024, 1) . 'KB';
		else
		return round($rawSize, 1) . 'bytes';
	}
	
}

?>

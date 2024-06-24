<?php

require_once('NxLock_Lock.inc');

class NxLock_counter
{
	function __construct($name,$path=null) {
		$this->fs = new NxPlug_FileSys();
		$this->name = $name;
		if(!$path)
		{
			global $NX_DOMAIN;
			$path = NX_CONF .'locks/'.$NX_DOMAIN.'/';
		}
		else
			$path = rtrim($path,'/\\').'/';

		if(!is_dir($path))
		{
			if(!$this->fs->createTree($path))
			{
				throw new Exception('cant create path for lock '.$path.$name);
			}
		}

		$this->fullPath = $path.$name.'.txt';
		$this->lock = new NxLock_Lock($name,$path);
	}

	function __destruct() {
	}

	function inc() {
		$this->lock->getlock();
			if(is_file($this->fullPath))
				$src=$this->fs->file_get_contents($this->fullPath);
			else
				$src='0';

			$i=(int)$src;
			$i++;
			$this->fs->saveFile($this->fullPath,$i);
		$this->lock->unlock();

		return $i;
	}
}

?>
<?php

require_once(NX_LIB.'plugins/NxPlug_FileSys.inc');

class NxLock_Lock
{
	var $path;
	var $lockfp;
	var $isLock;

	function __construct($name,$path=null) {
		$this->name = $name;
		if(!$path)
		{
			$path = NX_GEN .'locks/';
		}
		else
			$path = rtrim($path,'/\\').'/';

		if(!is_dir($path))
		{
			$fs = new NxPlug_FileSys();
			if(!$fs->createTree($path))
			{
				throw new Exception('cant create path for lock '.$path.$name);
			}
		}

		$this->path = $path;
	}

	function __destruct() {
		$this->unlock($this->name);
	}

	function getLock() {
		$name=$this->name;
		$lockfp="fp_$name";
		$isLock = "lock_{$name}";
		if (!isset($this->$isLock) || !$this->$isLock)
		{
			$this->$lockfp = fopen($this->path."lock_{$name}.txt", "w+");
			$this->$isLock = flock($this->$lockfp, LOCK_EX);
		}
	}

	function unlock() {
		$name=$this->name;
		$lockfp="fp_$name";
		$isLock = "lock_{$name}";
		if (isset($this->$isLock) && $this->$isLock)
		{
			flock($this->$lockfp, LOCK_UN); // lib?re le verrou
			fclose($this->$lockfp);
			$this->$isLock = false;
			$this->$lockfp = null;
		}
	}
}

?>
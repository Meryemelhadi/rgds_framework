<?php

	class NxLock {
		function __construct($name='alock') {
			$this->name=$name;
			$this->fname="lock_{$name}.txt";
			$this->fp=null;
			$this->lock=null;
		}

		function __destruct() {
			$this->release();
		}

		// pose le verrou
		function get() {
			nxLog('REQUEST LOCK for '.$this->name,'LOCK');

			if ($this->lock)
				return true;

			$this->fp = fopen($this->fname, "w+");
			$this->lock = flock($this->fp, LOCK_EX);

			nxLog('GOT THE LOCK for '.$this->name,'LOCK');
			return true;
		}

		// libere le verrou
		function release() {
			if ($this->lock)
			{
				nxLog('RELEASING LOCK for '.$this->name,'LOCK');
				flock($this->fp, LOCK_UN); 
				fclose($this->fp);
				$this->lock=null;
				nxLog('LOCK RELEASED OK for '.$this->name,'LOCK');
				return true;
			}
		}
	}

?>
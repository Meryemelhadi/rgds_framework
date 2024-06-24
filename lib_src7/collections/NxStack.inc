<?php
class NxStackIter
{
	function __construct(&$stack,$cur)
	{
		$this->stack=&$stack;
		$this->cur=$cur;
	}
	
	function &current()
	{
		return $this->stack[$this->cur];
	}

	function &previous($moveCur=true)
	{
		$ret=&$this->stack[$this->cur-1];
		
		if ($moveCur)
			$this->cur--;
		
		return $ret;	
	}
}

class NxStack
{
	var $stack=array();
	var $cur=-1;
	
	function push($v) {
		$this->stack[]=&$v;
		$this->cur++;
	}

	function pop() {
		$ret=&$this->stack[$this->cur--];
		array_pop($this->stack);
		return $ret;
	}
	
	function & current()	{
		return $this->stack[$this->cur];
	}
	
	function iter() {
		return new NxStackIter($this->stack,$this->cur);
	}
	
	function getAll($flatten=null,$flush=false) 
	{
		if ($flatten!==null)
			$ret=implode($flatten,$this->stack);
		else
			$ret=$this->stack;
			
		if ($flush)
			$this->stack=array();
			
		return $ret;
	}
}
?>
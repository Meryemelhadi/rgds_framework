<?php

class NxTime_Timer
{
	private $ticks = array();
	private $times = array();
	private $sums = array();

	function __construct() {
	}

	function start($event='tick')
	{
		$this->ticks[$event][]=microtime(true);
	}

	function stop($event='tick')
	{
		$stop = microtime(true);
		if(!$this->ticks[$event])
		{
			nxError("cant stop timer not started ".$event);
			return;
		}

		$start = $this->ticks[$event][count($this->ticks[$event])-1];
		$this->times[$event][]=$duration=$stop - $start;
		if (!isset($this->sums[$event]))
			$this->sums[$event]=$duration;
		else
			$this->sums[$event]+=$duration;

		return $duration;
	}
	function add($timer)
	{
		foreach($timer->getAllSums() as $ev=>$sum)
		{
			if (!isset($this->sums[$ev]))
				$this->sums[$ev]=$sum;
			else
				$this->sums[$ev]+=$sum;
		}
	}

	function get($event='tick')
	{
		return $this->times[$event];
	}

	function getAllEvents()
	{
		return $this->times;
	}
	function getAllSums()
	{
		return $this->sums;
	}

	function all()
	{
		/*
		$s='';
		foreach($this->times as $k=>$events)
			foreach($events as $d)
				$s[]="ev[$k] = $d s";
		*/

		$total = 0;
		foreach($this->sums as $k=>$sum)
		{
			$total+=$sum;
			$s[]=sprintf("sum[%s] = %.2f s",$k,$sum);
		}

		$s[]=sprintf("TOTAL = %.2f s",$total);
		
		return $s;
	}

}

?>
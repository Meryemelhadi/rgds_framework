<?PHP

class NXL_genAppl
{
	function __construct(&$fs,$path)
	{
		$this->fs=&$fs;
		$this->path=$path;
	}
	
	function genLogic($apps) 
	{
		foreach ($apps as $pname=>$app)
		{					
			// create appli
			$this->genAppli($app);
		}
	}
	
	/* gen state machine for the application
	  ============================================================= */
	function genAppli($app)
	{
		$fullpath=trim($app['path'],"/").'/'.str_replace('.','/',$app['name']);
		$class_name=str_replace('/','_',$fullpath);
		$packagepath=str_replace('/','.',$fullpath);
				
		$src='<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.\'module/NxApplication.inc\');
';

		$src.=
"class $class_name extends NxApplication
{
	function $class_name".'($props,&$parentDesc)
	{
		parent::__construct('."'$packagepath'".',$props,$parentDesc);
	}

	function run()
	{
		switch ($this->operation)
		{
';

	$ops = $this->reorderOperations($app['operations']);
	foreach($ops as $oname=>$op)
	{
		$src .= $this->getOpSrc($op);
	}

	$src .='
		}		
		$this->saveState();
		return true;
	}
}
?>';
	
		$this->fs->saveFile($this->path.$fullpath.'.php',$src);
	}
	
	function getOpSrc($op)
	{
		$oname=$op['name'];
		$src = "    // operation: $oname\n    case '$oname':";
		
		if (isset($op['default']) && $op['default']=='yes')
		{
			$src.='
	default:
		$this->operation=\''.$oname.'\';
';
		}
			
		$src .='
		
		switch($this->step)
		{
		';
			
		foreach($this->reorderSteps($op['steps']) as $st)
		{
			$src .= $this->getStepSrc($st,$op);
		}
		
		$src .="		}
        break;\n";
		
		return $src;
	}
	
	function getStepSrc($step,$op)
	{
		$fullpath=trim($step['path'],'/').'/'.$step['name'];
		$packagepath=str_replace('/','.',$fullpath);
		$oname=$op['name'];		
		$sname=$step['name'];
		$trans = $step['trans'];
		
		$src  = "
         // step $sname    
		 case '$sname':";
		
		if (isset($step['default']) && $step['default']=='yes')
		{
			$src.='
		 default:
			$this->operation=\''.$oname.'\';
			$this->step=\''.$sname.'\';
			$this->startOp();
';
		}
		
		// define step properties
		$props=$this->getStepProps($step);
		if (count($props))
		{
			$propSrc=',
				array(';
			foreach ($props as $n=>$prop)
			{
				$propSrc.="
					'$n'=>$prop,";
			}
			$propSrc.=')';
		}
		else
			$propSrc='';
		
			
		// add step logic
		$src.="
			if (\$this->runLogic('$packagepath'".$propSrc."))
			{";
		
		if (isset($step['end']) && $step['end']=='yes')
		{
			if (isset($trans['redirect']))
			{
				$url=$this->getStepUrl($trans['redirect']);
				// $src.='
				// $this->redirectToOperation('.$url.');';
				$src.="
				header('Location:'.\$this->getProperty('site.url.noprefix','').{$url});";
				
			}
						
			$src.='
				$this->finish();
';
		}
		else
			$src.="\n";
				
		$src .="			}
            break;\n";
		return $src;
	}
	
	// get properties for step logic
	function getStepProps($step)
	{
		$props=array();
		
		$trans = $step['trans'];
		if (isset($trans['back']))
		{
			$url=$this->getStepUrl($trans['back']);
			if ($url)
			{
				$props['back.url']='$backUrl='.$url;
				$props['cancel.url']='$backUrl';
			}
			//else
				//$url='$this->getPropery(\'back.url\')';			
		}
		
		if (isset($trans['next']))
		{
			$url=$this->getStepUrl($trans['next']);
			if ($url)
			{
				$props['next.url']='$nextUrl='.$url;
				$props['nextStep']='$nextUrl';
			}
		}
		
		return $props;
	}
	
	function getStepUrl($nav)
	{
		if (isset($nav['url']) && trim($nav['url'])!='')
			return "'{$nav['url']}'";
	
		if (isset($nav['page']) && trim($nav['page'])!='')
			$page="'{$nav['page']}'";
		else
			$page='null';
		
		$op='null';
		$step='null';
		if (isset($nav['operation']) && trim($nav['operation'])!='')
		{
			$op="'{$nav['operation']}'";
		}
		
		if (isset($nav['step']) && trim($nav['step'])!='')
		{
			$loc=explode('.',$nav['step']);
			if (count($loc)>1)
			{
				// op.step
				$op="'{$loc[0]}'";
				$step="'{$loc[1]}'";
				// return "\$this->getUrl('{$loc[1]}','{$loc[0]}')";
			}
			else if (count($loc)==1)
			{
				$step="'{$loc[0]}'";
				// step
				//return "\$this->getUrl('{$loc[0]}')";
			}
			else 
			{
				nxError("wrong step transition:{$nav['step']}");
			}
		}
		else
		{
			if ($op!=='null')
				$step="''";
		}
		
		if ($op=='null' && $step=='null' && $page='null')
		{
			return null;
		}
		
		return "\$this->getUrl({$step},{$op},{$page})";
	}
	
	function stepLogic($step)
	{
		return '';
	}
	
	// change order of steps for switch-case
	function reorderOperations($ops)
	{
		$dft=null;
		$end=null;
		$ops2=array();
		
		// get default and end operations
		foreach ($ops as $n=>$op)
		{
			if (isset($op['default']) && $op['default']=='yes')
				$dft=$op;
			else if (isset($op['end']) && $op['end']=='yes')
				$end=$op;
			else
				$ops2[$n]=$op;
		}
				
		// check for default op/step and put it at the end
		if ($dft===null)
		{
			// not found => assumes the first one is the default (start)
			$dft=array_shift($ops2);
			$dft['default']='yes';						
		}	
		
		// check for last operation/step
		if ($end===null)
		{
			if (count($ops2)>0)
			{
				$end=array_pop($ops2);
				$end['end']='yes';						
			}
			else if ($dft)
			{
				$dft['end']='yes';									
			}
		}
		
		// add last op/step
		if ($end)
			$ops2[$end['name']]=$end;
			
		// add default op/step
		$ops2[$dft['name']]=$dft;
			
		return $ops2;
	}
	
	function reorderSteps($steps)
	{
		return $this->reorderOperations($steps);
	}
}

?>
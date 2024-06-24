<?PHP

// all data format operations
class taglib_nxfrontier_nxl_crypt extends NxTagLib
{		
	function genPassword($node,&$content,$level,$pad)
	{		
		$this->getAttribute($len,$node,'length','8',true,false);
		$plugVar=$this->createVarID('pwd_plug');
		
		return "\n{$pad}(\${$plugVar}=\$this->getPlugIn('Password'))!=null?\${$plugVar}->createEasyPassword($len):''";
	}

	function genHashCode($node,&$content,$level,$pad)
	{		
		$this->getAttribute($val,$node,'value',$content,true,true);
		if ($this->getAttribute($date,$node,'type',null,true,false) && $type=='date')
		{
			$val .= '.time()';
		}
		
		$val .= '.(($sr=srand((double)microtime()*1000000))?rand():0)';

		return "\n{$pad}md5($val)";
	}

	function genPassCode($node,&$content,$level,$pad)
	{		
		if (!$this->getAttribute($key,$node,'key',null,true,true))
			return $this->fatal('no key attribute');
			
		if (!$this->getAttribute($user,$node,'user',null,true,true))
			return $this->fatal('no user attribute');
			
		if (!$this->getAttribute($ticket,$node,'ticket',null,true,true))
			return $this->fatal('no ticket attribute');
			
		return "md5($key.$user.$ticket)";
		
	}
}

?>
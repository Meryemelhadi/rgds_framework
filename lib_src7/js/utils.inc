<?php

	function _PHP2Js2($o,$ak='') 
	{
		$s='';
		if (is_array($o))
		{
			ksort($o);
			foreach ($o as $key=>$val)
			{
				$s.=_PHP2Js2($val,"{$ak}['{$key}']");
			}
			return  rtrim($s,',');
		}
		elseif (is_string($o))
		{
			$o=preg_replace("/\"/",'&quot;',$o);
			return "{$ak}=\"$o\";";
		}
		elseif (is_integer($o))
			return "{$ak}=$o;";
		else if(is_object($o))
		{
			return _PHP2Js2(get_object_vars($o),$ak);
		}
	}
	
	function _PHP2Js($o) 
	{
		$s='';
		if (is_array($o))
		{
            if(util_is_assoc($o)) 
            {
				foreach ($o as $key=>$val)
				{
					$s.="{$key}:"._PHP2Js($val).',';
				}
				return  '{'.rtrim($s,',').'}';
            }
            else 
            {
				foreach ($o as $key=>$val)
				{
					$s.=_PHP2Js($val).',';
				}
				return  '['.rtrim($s,',').']';
            }

		}
		elseif (is_string($o))
		{
			$o=preg_replace("/\"/",'&quot;',$o);
			
			// detected a js object do not wrap it as string
			if ($o!='' && $o[0]=='{')
				return "$o";

			return "\"$o\"";
		}
		elseif (is_integer($o))
			return $o;
		else if(is_object($o))
		{
			return _PHP2Js(get_object_vars($o));
		}
		
		return 'null';
	}
	
    function util_is_assoc($array) {
    	if (count($array)==0)
    		return false; 
    		
        krsort($array, SORT_STRING); 
        return !is_numeric(key($array));          
    } 	
	
	function PHP2Js($o,$name='',$mode='literal',$array_map='as-object') 
	{
		if ($mode=='literal')
		{
			$res=_PHP2Js($o,$array_map);
			if ($name!='')
				return "$name=$res;";
			else
				return $res;
		}
		else
			return _PHP2Js2($o,$name,$array_map);
	}

?>
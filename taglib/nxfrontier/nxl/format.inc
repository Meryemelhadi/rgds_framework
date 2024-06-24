<?PHP

// all data format operations
class taglib_nxfrontier_nxl_format extends NxTagLib
{		
	function toIdentifier($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "trim(preg_replace(array('/&[^;]+;/','/\\W+/','/_\$/'),array('','_',''),strtolower(trim(strtr($v,'àâéèêëçîïùüûô','aaeeeeciiuuuo'),'\''))),'_')";
		}
		else
		{
			return trim(preg_replace(array('/&[^;]+;/','/\W+/','/_$/'),array('','_',''),strtolower(trim(strtr($v,'àâéèêëçîïùüûô','aaeeeeciiuuuo'),'\''))),'_');
		}
	}
	function toLatin($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "(\$GLOBALS['NX_CHARSET']=='utf-8'?utf8_decode($v):($v))";
		}
		else
		{
			return $GLOBALS['NX_CHARSET']=='utf-8'?utf8_decode($v):($v);
		}
	}
	
	// 
	function explode($node,&$content,$level,$pad)
	{			
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			$this->getAttribute($sep,$node,'separator','\n',true,true);
		}
		else
		{
			$this->getAttribute($sep,$node,'separator','\n',true,false);
		}
		
		return "explode($sep,$v)";
	}

	// 
	function implode($node,&$content,$level,$pad)
	{			
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			$this->getAttribute($sep,$node,'separator','\n',true,true);
		}
		else
		{
			$this->getAttribute($sep,$node,'separator','\n',true,false);
		}
		return "implode($sep,$v)";
	}
	
	// 
	function urlEncode($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "htmlentities(urlencode($v))";
		}
		else
		{
			return htmlentities(urlencode($v));
		}
	}
	
	// 
	function urlDecode($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "urldecode($v)";
		}
		else
		{
			return urldecode($v);
		}
	}
	
	// 
	function trim($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,false);

		if ($this->getAttribute($chars,$node,'chars',null,true,true))
		{
			if (preg_match('/\$\w/',$v))
			{
				return "trim($v,$chars)";
			}
			else
			{
				return trim($v,$chars);
			}
		}
		else 
		{
			if (preg_match('/\$\w/',$v))
			{
				return "trim($v)";
			}
			else
			{
				return trim($v);
			}
		}
	}
	
	//	<format:trimAfterLast pattern="-" value="#$path#" />
	function trimAfterLast($node,&$content,$level,$pad)
	{		
		$this->getAttribute($v,$node,'value',$content,true,true);
		$this->getAttribute($dft,$node,'default',$v);

		if (!$this->getAttribute($pattern,$node,'pattern',null,true,true))
			return $this->fatal('no pattern attribute');

		if (preg_match('/\$\w/',$v) || preg_match('/\$\w/',$dft))
		{
			return "((\$tmp=$v)&&((\$pos=strrpos(\$tmp,$pattern))!==false)?substr(\$tmp, 0, \$pos):$dft)";
		}
		else
		{			
			return ((($pos=strrpos($v,$pattern))!==false)?substr($v, 0, $pos):$dft);
		}
	}
		
	// <format:replace from="," to="-" value="?-#property:nav.path#" />
	// replace a set of chars by others
	function replace($node,&$content,$level,$pad)
	{	
		if(!$this->getAttribute($from,$node,'from',null,true,true) || 
			!$this->getAttribute($to,$node,'to',null,true,true))
			return $this->fatal('missing "from" or "to" attribute');
			
		if (preg_match('/\$\w/',$content))
			$this->getAttribute($v,$node,'value',$content,true,false);
		else
			$this->getAttribute($v,$node,'value',"'$content'",true,false);
			
		return "\n{$pad}str_replace($from,$to,$v)";
	}
	
	// <format:reg_replace re="/,/" to="-" value="?-#property:nav.path#" />
	// apply regular expression changes
	function reg_replace($node,&$content,$level,$pad)
	{	
		if(!$this->getAttribute($from,$node,'re',null,true,true) || 
			!$this->getAttribute($to,$node,'to',null,true,true))
			return $this->fatal('missing "re" or "to" attribute');
			
		if (preg_match('/\$\w/',$content))
			$this->getAttribute($v,$node,'value',$content,true,false);
		else
			$this->getAttribute($v,$node,'value',"'$content'",true,false);
			
		return "\n{$pad}preg_replace($from,$to,$v)";
	}

	// <format:pad value="#$v#" char="_" side="left" len="10" />
	// pad value with characters, on a left|right side, length len
	function pad($node,&$content,$level,$pad)
	{	
		if(!$this->getAttribute($char,$node,'char',null,true,true) ||
			(!$this->getAttribute($n,$node,'len',null,true,false) && 
			!$this->getAttribute($n,$node,'length',null,true,false)))
			return $this->fatal('missing "char" or "length"(= "len") attribute');
			
		if (preg_match('/\$\w/',$content))
			$this->getAttribute($v,$node,'value',$content,true,false);
		else
			$this->getAttribute($v,$node,'value',$content,true,true);

		$this->getAttribute($side,$node,'side',"left",true);
		switch(strtolower($side))
		{
			case 'left':$side='STR_PAD_LEFT'; break;
			case 'right':$side='STR_PAD_RIGHT'; break;
			default:
				return $this->fatal('unknown "side":"'.$side.'", try "left" or "right"');
		}

		return "\n{$pad}str_pad($v,$n,$char,$side)";
	}

	// <format:replace from="," to="-" value="?-#property:nav.path#"/>
	// replace a set of chars by others
	function quotes2HTML($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "preg_replace(array('/\\'/','/[\"]/'),array('&#146;','&quot;'),$v)";
		}
		else
		{
			return preg_replace(array('/\'/','/["]/'),array('&#146;','&quot;'),$v);
		}
	}
	
	// <format:replace from="," to="-" value="?-#property:nav.path#"/>
	// replace a set of chars by others
	function simpleQuotes2HTML($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "preg_replace(array('/\\'/'),array('&#146;'),$v)";
		}
		else
		{
			return preg_replace(array('/\'/'),array('&#146;'),$v);
		}
	}

	// <format:stringBlock>string with html </format:stringBlock>
	// wrap html text as a string
	function stringBlock($node,&$content,$level,$pad)
	{
		return "<<<EOH\n{$content}\nEOH\n";
	}

	// <format:parseTPL>string with html </format:parseTPL>
	// parse TPL to string
	function parseHTML($node,&$content,$level,$pad)
	{
		$patterns = array(
			'#(src|href|background)\s+=\s+#ie',
			'#((?:src|href|background|value)=")[^">]*[/]default[/]res[/]([^"]+")#i',
			'#((?:src|href|background|value)=")[^">]*[/]res[/]([^"]+")#ie',
			);
			
		$replace = array(
			'$1=',
			'$1{global:NX_DEFT_SKIN_URL}$2',
			'$this->mapResPath("$1","$2")',
			);
		
		$v=preg_replaceX($patterns, $replace,$content,$this);
		
		return "<<<EOH\n{$v}\nEOH\n";
	}

	function mapResPath($attr,$path)
	{
		$p="{$attr}".'{global:NX_SKIN_URL}'.$path;
		nxLog("map path: {$attr}{$path} to {$p}","NXL_FORMAT");
		return $p;
	}	
	
	
	// <format:lowerCase value="XXX"/>
	function lowerCase($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "strtolower($v)";
		}
		else
		{
			return strtolower($v);
		}
	}
	// <format:lowerCase value="XXX"/>
	function upperCase($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "strtoupper($v)";
		}
		else
		{
			return strtoupper($v);
		}
	}

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
		
		return "\n{$pad}md5($val)";
	}
	
	
	// <format:stripTags value="#property:section.cache#" />
	// strip html tags and returns text
	function stripTags($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "(\$fmtPlug=&\$this->getPlugin('format'))!=null?\$fmtPlug->stripTags({$v},'UTF-8'):''";
		}
		else
		{	
			// add ws around tags, strip tem, remove ws
			$fmt=&$this->props->getPlugin('format');
			return $fmt->stripTags($v,'UTF-8');
		}
	}
	
	function parseTPL($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "\n{$pad}preg_replaceX(array('/{@(?:=|property:)([^}]+)}/ie',),
					{$pad}	array('\$this->getProperty(\"$1\")'),
					{$pad}	{$v},\$this)";
		}
		else
		{	
			return preg_replaceX(
					array(
						'/{@(?:=|property:)([^}]+)}/ie',
						'/{@(?:=|user:)([^}]+)}/ie',
					),
					array(
						'\$this->getProperty(\"$1\")',
						'\$GLOBALS["_NX_user"]->getProperty(\"$1\")'
					),
					$v,$this);
		}
	}
	
	// transform a numeric index to alpha so that can be alpha sorted..
	// works for indexes between 0 and 63
	function num2alphaSort($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		if (preg_match('/\$\w/',$v))
		{
			return "\n{$pad}(((\$n=$v)<10)?\$n:((\$n<37)?chr(ord('a')+(\$n-10)):(chr(ord('A')+(\$n-37)))))";
		}
		else
		{	
			// convert during compile time
			return ((($n=$v)<10)?$n:(($n<37)?chr(ord('a')+($n-10)):(chr(ord('A')+($n-37)))));
		}
	}
	
	function html2text($node,&$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);

		if (preg_match('/\$\w/',$v))
		{
			// runtime transformation
			return "\n{$pad}((class_exists('html2text')||false!==include_once(NX_LIB.'tools/class.html2text.inc'))&&
{$pad}	(\$parser=new html2text($v))!=null)?\$parser->get_text():''";
		}
		else
		{	
			// compile time transformation
			if (!class_exists('html2text'))
				require_once(NX_LIB.'tools/class.html2text.inc');
				
			$parser=new html2text($v);
			return $parser->get_text();
		}
	}
}

?>
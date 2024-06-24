<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_rich_text_default
{
	function __construct()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm(&$value,&$desc)
	{
		$fname = $f->getAlias();
		// $F_ = $desc->name;

		// transform HTML to enriched text (ie with some patterns)
		// - 2 \n for paragraph
		// - one \n for line feed
		// -  "-" char for lists
		// - *word* for bold
		// [url content] for links
		$patterns = array(
			'/(&quot;|&#39;)/',
			'%<p[^>]*>(.*)</p>%i',
			'%<br[/]?>%i',
			'%<li[^>]*>([^\n]*)<li>%i',
			'%<li[^>]*>([^\n]*)</li>\n?%i',
			'%</(ul|ol)>%i',
			'%<(?:b|strong)>(.*)</(?:b|strong)>%i',
			'%<a\s+href="([^"]+)"[^>]*>(.*)</a>%i',
			'%<[^>]*>%',
			);
			
		$replace = array(
			"\'",
			'$1'."\n\n",
			"\n",
			"\n-".'$1<li>',
			"\n-".'$1',
			"\n",
			'*$1*',
			'[$1 $2]',
			'',
			);

		// extract head, title and body code
		$inputVal = preg_replace($patterns, $replace,$value);
		
		// $inputVal = $value;

		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$rowsMaxBox= intval($desc->getProperty("rowsMaxBox",5));

		$size=intval($desc->getProperty("size",$colsMaxBox));
		
		if ($maxLength = intval($desc->getProperty("maxLength",null)) != null)
			$filterChars = ' onkeypress="keypress_textarea(event,'.$maxLength.');" onpaste="keypress_textarea(event,'.$maxLength.');"';
		else
			$filterChars = '';

		// size larger than 2 lines => textarea with checking of number of chars
		$nbRows =  $size / $colsMaxBox;
		if ($nbRows <= $rowsMaxBox)
			$rows = $nbRows;
		else
			$rows = $rowsMaxBox;

		$style = $desc->getStyle("input","textarea");
		$res = "<textarea class=\"text\" type=\"text\" name=\"".$fname."\"".$style.
			 " cols=\"".$colsMaxBox."\" rows=\"".$rows."\"".
			 $filterChars.">".$inputVal."</textarea>";
		
		return $res;
	}

	function readForm(&$recData,&$desc)
	{
		// $fname = $desc->name;
		$fname = $f->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=="")
			return null;

		// transform enriched text (ie with some patterns) to HTML
		// - 2 \n for paragraph
		// - one \n for line feed
		// -  "-" char for lists
		// - *word* for bold
		// [url content] for links
		$patterns = array(
			'%<[^>]*>%',
			"/'/",'<','>',
			'%\n-([^\n]*)%i',
			'%(.*)\n\n%i',
			'%[*]([^*]+)[*]%',
			'%\[([^ ])+\s+([^\]]*)\]%',
			'%\n%',
			);
			
		$replace = array(
			'',
			"&#39;", '&lt;','&gt;',
			'<li>$1</li>',
			'<p>$1</p>',
			'<b>$1</b>',
			'<a href="$1">$2</a>',
			'<br>',
			);

		// extract head, title and body code
		$value = preg_replace($patterns, $replace,$value);
			
		return $value;  
	}
}

?>
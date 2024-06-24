<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/**
 * @return void
 * @param string $msg message to display
 * @param string $level type of display (compil,listing,debug, walker)
 * @param unknown $indent indentation level
 * @desc displays a message in the compil trace window.
*/
$GLOBALS['nxl_trace']='';
$GLOBALS['nxl_indent']=0;
$GLOBALS['nxl_pad']='';
$GLOBALS['nxl_error_number']=0;
//$_GET['debug']=0;

function nxltrace($msg,$level='compil',$indent=0,$encode=false)
{
	if (!defined("NX_UI_DEBUG") || !NX_UI_DEBUG)
		return;

	global $nxl_trace;
	$s = '';

	if ($msg==''
		||($level=='debug' && !isset($_GET['debug']))
		||((($level=='walker')||($level=='taglibx')) && !isset($_GET['walker']))
		)
	 return;
	
	if ($level=='error')
	{
		if ($GLOBALS['nxl_error_number']>0)
			$s.= "<a href=#error_{$GLOBALS['nxl_error_number']}>[prev error]</a>";
		$GLOBALS['nxl_error_number']++;
		$s.= "<a name=error_{$GLOBALS['nxl_error_number']}></a>";
		$s.= "<a href=#error_".(1+$GLOBALS['nxl_error_number']).">[next error]</a>";
	}
	
	if ($encode)
		$msg=htmlentities($msg);
		
	global $nxl_indent,$nxl_pad;
	if ($indent!=0)
	{
		$nxl_indent+=$indent;
		if ($nxl_indent<0)
			$nxl_indent=0;
		$nxl_pad=str_repeat('&nbsp;',$nxl_indent);
	}
	
	// $this->res['compil'][]=$msg;
	$s .="<div class=$level>$nxl_pad$msg</div>\n";
	if(NX_UI_DEBUG == 'nxltrace')
		nxLog($s,'nxltrace');
	$nxl_trace = $s;
}
	
class NxPlug_TplMap_tpml
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}

	function translate($src,&$tpl)
	{
		return $src;
	}	
		
	function transformNXL($src,$src_path)
	{	
		include_once(NX_LIB.'mvc/NXL/NXL_processor.inc');

		$GLOBALS['nxl_indent']=0;
		$GLOBALS['nxl_pad']='';
		$GLOBALS['nxl_error_number']=0;
		
		// dont process resources
		$GLOBALS['NXL_PROCESS_RESOURCES']=false;
				
		$xml_parser = new NXL_processor($this->props);
		$src='<no_tag  xmlns="http://www.w3.org/1999/xhtml">'.$src.'</no_tag>';
		$xml_parser->genSrc($src);
		$src=$xml_parser->buildOutput();
		
		// remove <![CDATA[<?php echo "xxx";  ?">]]>
		$content='((.|\n)(?!\]\]))';
		$src = preg_replace(
			array('/<[!]\[CDATA\[<\?php('.$content.'*)\?(\"?)>\]\]>/s',
				'/[&]amp;amp;/'),
			array('<?php \\1 ?>',
				'&'.'amp;'), $src); 
		                   
// 		nxLog("' code after [$src] '");

		return $src;
	}

}

?>
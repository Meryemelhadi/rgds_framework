<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

if (!defined("NX_DWT_MAP_TITLE"))
	define("NX_DWT_MAP_TITLE","title");
	
/* manage template files with html extension. Manage head and body tags in an html page. */
class NxPlug_TplMap_xhtml
{
	function __construct(&$props)
	{
		$this->props=&$props;
	}

	function translate($src,&$tpl)
	{
		$drmWvr=$this->props->getPlugin('TplMap_tpml','NxPlug_','mvc/TPL');
		
		// transform template
		$src = $drmWvr->translate($src,$tpl);
		
		if ($tpl->isPage)
			return $src;
		
		$this->title=null;
		$this->head=null;
		$this->body=null;
		
		// extract head and returns body	
		$patterns = array(
			'/<!--!((((.)(?!-->))*)(.)?)-->/s',
			'%<title>(.*)</title>%ise',
			'%<head>((?:.*))</head>%ise',
			'%<body([^>]*)>((?:.*))</body>%ise'
			);
			
		$replace = array(
			'$1',
			'$this->getTitle("$1")',
			'$this->getHead("$1")',
			'$this->getBody("$1","$2")',
			);

		// extract head, title and body code
		$res = preg_replaceX($patterns, $replace,$src,$this);
		
		// if something in head, add it to head collection to be inserted in final page
		if ($this->head != null && ($this->head=trim($this->head))!="")
		{
			$heads =& $this->props->root->getProperty("page.modules.heads",null,false);
			if ($heads==null)
				$this->props->root->setProperty("page.modules.heads",array($this->head));
			else
				$heads[] = $this->head;
		}
		
		// if body not empty, returns body, else returns what's left in page
		if ($this->body != null && ($this->body=trim($this->body))!="")
			return $this->body;
		else
			return $res;
	}

	function transformNXL($src)
	{
		include_once(NX_LIB.'mvc/NXL/NXL_processor.inc');

		$GLOBALS['nxl_indent']=0;
		$GLOBALS['nxl_pad']='';
		$GLOBALS['nxl_error_number']=0;
		
		// dont process resources
		$GLOBALS['NXL_PROCESS_RESOURCES']=false;
				
		$xml_parser = new NXL_processor($this->props);
		$xml_parser->genSrc($src);
		$src=$xml_parser->buildOutput();
		
		// remove <![CDATA[<?php echo "xxx";  ?">]]>
		$content='((.|\n)(?!\]\]))';
		$src1 = preg_replace(
			array('/<[!]\[CDATA\[<\?php('.$content.'*)\?(\"?)>\]\]>/s',
				'/[&]amp;amp;/'),
			array('<?php \\1 ?>',
				'&'.'amp;'), $src); 
		                   
// 		nxLog("' code after [$src] '");
        if($src1)
            return $src1;
    
        $src1 = preg_replace(
            array(
                '/\<[!]\[CDATA\[<\?php/',
                '/\?(\"?)>\]\]>/',
                '/[&]amp;amp;/'),
            array(
                '<?php',
                '?>',
                '&'.'amp;'), $src); 

        if($src1)
            return $src1;
            
		return $src;
	}

	function getTitle($s)
	{
		// nxLog("HTML content template, got title:[$s]","HTML_TPL");
		$this->title=$s;
		//nxLog("HTML content template - end","HTML_TPL");
		return '';
	}
	
	function getHead($s)
	{
		nxLog("HTML content template, got head:[$s]","HTML_TPL");
		// NB. preg_replace has added slashes before ' chars => remove them
		$this->head=str_replace("\'","'",$s);
		return '';
	}
	
	function getBody($attr,$s)
	{
		nxLog("HTML content template, got body:[$s]","HTML_TPL");
		
		/*
		nxLog(
		"+++++++++++++++\nHTML TRANSLATION : GET BODY\n++++++++++++++\n".
		$s .
		"+++++++++++++++\nHTML TRANSLATION : END++++++++++++++\n","HTML TPL"); 
		*/
		
		$this->body=str_replace("\'","'",$s);
		
		/*
		nxLog(
		"+++++++++++++++\nHTML TRANSLATION : TRANSLATED BODY\n++++++++++++++\n".
		$this->body .
		"+++++++++++++++\nHTML TRANSLATION : END++++++++++++++\n","HTML TPL"); 
		*/
		
		return '';
	}
}

?>
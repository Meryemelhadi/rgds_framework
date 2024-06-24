<?php
set_time_limit(30000);

// initialise the nx API (internal constants and logs)
if (!defined('NX_BUILD_NXL'))
	require_once(NX_DIS.'NxInit.inc');

// init site properties
$siteDesc = & NxSite_init(array(		
	// upload settings
	"pathUpload" => NX_DOC_ROOT."files",
	"baseUrlUpload" => NX_BASE_URL."files/",
	"maxFileUploadSize" => 2500000,

	// regional settings
	"lang"	=> "en",
	"dateFormatShort" => "%d/%m/%y",		
	));
?>

<html>
<head>
<title> NXL Compilation : <?php echo (isset($_GET['url'])?$_GET['url']:''); ?></title>
<style>
body,p,div,li { font-family:arial; }
.error { color:red; }
.ok { color:green; }
.resource {color:green;}
.trace { color:#333333;font-style:italic;}
.tag { color:#666699; }
.tag_content { margin-left:15px; border:solid 1px #666699; color:#999999;}
.frag_source {color:#222255; font-family:courier;font-size:smaller; background-color:#ddddee;border:solid 1px black; }
.listing,.page {color:#222222; font-family:courier;font-size:smaller; background-color:#eeeeee;border:solid 1px black; }
.debug {color:brown;font-family:courier;font-size:x-small; margin-left:20px;}
.walker {color:brown;font-family:courier;font-size:xx-small; margin-left:20px;}
.article {
	display:block; 
	border:solid 1px black; 
	background:#999999; 
	color:#000000;
	font-weight:bold; 
	font-size:1.2em;
	text-align:right;
}

.taglib {
	color:magenta;font-family:courier;font-size:x-small; margin-left:20px;
}

</style>
</head>
<body>

<?php
$GLOBALS['nxl_indent']=0;
$GLOBALS['nxl_pad']='';
$GLOBALS['nxl_error_number']=0;
/**
 * @return void
 * @param string $msg message to display
 * @param string $level type of display (compil,listing,debug, walker)
 * @param unknown $indent indentation level
 * @desc displays a message in the compil trace window.
*/
function nxltrace($msg,$level='compil',$indent=0,$encode=false)
{
	if ($msg==''
		||($level=='debug' && !isset($_GET['debug']))
		||((($level=='walker')||($level=='taglib')) && !isset($_GET['walker']))
		)
	 return;
	
	if ($level=='error')
	{
		if ($GLOBALS['nxl_error_number']>0)
			echo "<a href=#error_{$GLOBALS['nxl_error_number']}>[prev error]</a>";
		$GLOBALS['nxl_error_number']++;
		echo "<a name=error_{$GLOBALS['nxl_error_number']}></a>";
		echo "<a href=#error_".(1+$GLOBALS['nxl_error_number']).">[next error]</a>";
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
	echo "<div class=$level>$nxl_pad$msg</div>\n";
}

function nxltrace_report()
{
	if ($GLOBALS['nxl_error_number']>0)
	{
		echo "<a name=#error_".(1+$GLOBALS['nxl_error_number']).">[end]</a>";
		echo "<div class=error><strong>TOTAL ERRORS: {$GLOBALS['nxl_error_number']}</strong></div>\n";	
		echo "<div class=error><a href=#error_1>[first error]</a></div>";
		echo "<div class=error><a href=#error_{$GLOBALS['nxl_error_number']}>[last error]</a></div>";
	}
	else
		echo "<div class=ok><strong>TOTAL ERRORS: {$GLOBALS['nxl_error_number']}</strong></div>\n";	

	echo "<div class=error><a href={$_SERVER['PHP_SELF']}?{$_SERVER['QUERY_STRING']}&debug=yes&walker=yes>[debug mode]</a></div>";
	echo "<div class=error><a href={$_SERVER['PHP_SELF']}?{$_SERVER['QUERY_STRING']}&walker=yes>[detailed trace mode]</a></div>";

}

$siteDesc->runPage(array(
	// page properties
		'page.logic' => 'handler.nxl',
		'page.skin' => 'default',
		'page.strings' => 'nxl',
		'page.view'=>'sys.nxl.compil',
));

nxltrace_report();
?>

</body>
</html>
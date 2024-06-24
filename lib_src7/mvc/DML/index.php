<pre>
<?
define('NX_LIB','./');
define('NX_DIS_CONF','./');

function nxError($msg) {
	echo "Error:$msg<br/>\n";
}

include('DML_processor.inc');

$xml_parser = new DML_processor();
$xml_parser->build('C:\webs\openclinical\nx\conf\content\data\openclinical.xml');
$xml_parser->toPHP();

?>
</pre>
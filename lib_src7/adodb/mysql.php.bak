<?
include("adodb.inc.php");
 $db = NewADOConnection('mssql');
 $db->Connect("192.168.1.202,51867", "sa", "SikoBiko22", "PYXINET");
 $result = $db->Execute("SELECT * FROM dbo._user");
 if ($result === false) die("failed");  
 while (!$result->EOF) {
	for ($i=0, $max=$result->FieldCount(); $i < $max; $i++)
		   print $result->fields[$i].' ';
	$result->MoveNext();
	print "<br>\n";
 } 
?>
<html>
<head>
<title>Login error</title>
<?php

echo "<link rel=\"stylesheet\" href=\"".NX_SKIN_URL."display.css\">\n";
echo "<link rel=\"stylesheet\" href=\"".NX_SKIN_URL."edit.css\">\n";
echo "<script src=\"". NX_JS ."checkForm.js\" ></script>\n";

?>
</head>

<body leftmargin="0" topmargin="0" scroll="auto">
<div align="center">

<?php
echo "<div".$ui->getStyle("style:box","text-align:center").">"

echo "<table><tr><td colspan=\"2\"".$ui->getStyle("style:msg").">";
echo $this->getParam("msg","&nbsp;");
?>
</td></tr>
<tr><td align="center">
<?php
	echo $ui->getButton("button");
?>
</td>
</tr>
</table>
</div>
</body>
</html>

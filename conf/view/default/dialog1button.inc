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

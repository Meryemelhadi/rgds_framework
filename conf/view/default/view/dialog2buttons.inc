<?php
echo "<div".$ui->getStyle("box","text-align:center").">";

echo "<table><tr><td colspan=\"2\"".$ui->getStyle("msg").">";

if (isset($properties["msg"]))
	echo $properties["msg"];
else
	echo $properties["&nbsp;"];
?>
</td></tr>

<tr><td align="center">
<?php
	echo $this->getButton("b1");
?>
</td><td align="center">
<?php
	echo $this->getButton("b2");
?>
</td></tr>
</table>
</div>
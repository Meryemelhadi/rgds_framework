<?php
	echo "<div".$this->getStyle("box","text-align:center").">";
?>
<table><tr>
<?php
	echo "<td colspan=\"2\"".$this->getStyle("message","text-align:center").">";
	echo $this->getParam("msg","&nbsp;");
?>
</td></tr>
<tr><td align="center">
<?php
	echo $this->getButton("button");
?>
</td>
</tr>
</table>
</div>

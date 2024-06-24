<div align="center">
<table><tr><td colspan="2" align="center">
<?php
if (isset($properties["msg"]))
{
	echo $properties["msg"];
}
?>
</td></tr>

<tr><td align="center">
<?php
if (isset($properties["ok.url"]))
{
	if (!isset($properties["ok.label"]))
		$label = "Ok";
	else
		$label = $properties["ok.label"];

	echo "<a href=\"".$properties["ok.url"]."\">";
	if (!isset($properties["ok.image"]))
		echo($label);
	else
		echo("<img src=\"".$properties["ok.image"]."\" alt=\"".$label."\" border=\"0\" />");
	echo "</a>";
}
?>
</td><td align="center">
<?php
if (isset($properties["cancel.url"]))
{
	if (!isset($properties["cancel.label"]))
		$label = "Cancel";
	else
		$label = $properties["cancel.label"];

	echo "<a href=\"".$properties["cancel.url"]."\">";
	if (!isset($properties["cancel.image"]))
		echo($label);
	else
		echo("<img src=\"".$properties["cancel.image"]."\" alt=\"".$label."\" border=\"0\" />");
	echo "</a>";
}
?>
</td></tr>
</table>
</div>
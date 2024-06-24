<div align="center">
<?php
echo "<div>Admin console for table\"". $properties["table"]. "\"</div>";
?>

<?php


if (isset($properties["create.url"]))
{
	if (!isset($properties["create.label"]))
		$label = "Create table";
	else
		$label = $properties["create.label"];

	echo "<div align=\"center\">";
	echo "<a href=\"".$properties["create.url"]."\">";
	echo "<span style=\"border:1px solid #333333;background-color:#eeeeee\">";
	echo $label;
	echo "</span></a></div>";
}

if (isset($properties["delete.url"]))
{
	if (!isset($properties["delete.label"]))
		$label = "Delete table";
	else
		$label = $properties["delete.label"];

	echo "<div align=\"center\">";
	echo "<a href=\"".$properties["delete.url"]."\">";
	echo "<span style=\"border:1px solid #333333;background-color:#eeeeee\">";
	echo $label;
	echo "</span></a></div>";
}
?>
</div>
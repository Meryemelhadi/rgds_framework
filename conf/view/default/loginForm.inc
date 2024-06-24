<html>
<head>
<title>Login error</title>
<?php
	echo $this->getSyleSheets("login.head.css");
	echo $this->getScripts("login.head.scripts");
?>
</head>

<body leftmargin="0" topmargin="0" scroll="auto">
<div align="center">
<?php

if (isset($properties["msg"]))
{
	echo "<div class=\"msg\">".$properties["msg"] ."</div>";
}
?>

<div>
<?php
	$records = $properties["records"];
	if (!isset($properties["fmethod"]))
		$fmethod = "toHTML";
	else
		$fmethod  = $properties["fmethod"];
		
	for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();
		
		if (isset($properties["formHeader"]))
			echo $properties["formHeader"];
			
		echo "<table class=\"record_edit\">";
		$fieldIt = $record->getFieldIterator();
		$hiddenFields = "";
		for ($fieldIt->reset(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			if (!$field->isHidden())
			{
				echo "<tr>";
				echo "<td>".$field->getLabel()."</td>";
				echo "<td>".$field->$fmethod()."</td>";
				echo "</tr>";
			}
			else if ($fmethod != "toHTML")
			{
				$hiddenFields .= $field->$fmethod();
			}
				
		}
		
		if (isset($properties["submitButton"]) || isset($properties["cancelButton"]))
		{
			echo "<tr><td>";
			if (isset($properties["cancelButton"]))
				echo $properties["cancelButton"];

			echo "</td><td>";
				
			if (isset($properties["submitButton"]))
				echo $properties["submitButton"];
			else
				echo "&nbsp;";
			echo "</td></tr>";
		}			
		echo "</table>";
		
		if (isset($properties["formFooter"]))
		{
			echo $hiddenFields;
			echo $properties["formFooter"];
		}
	}
?>
</table>
</div>
</body>
</html>
<div align="center">
<table>
<?php
	$records = $properties["records"];
	$fmethod  = $properties["fmethod"];
	if (!isset($fmethod))
		$fmethod = "toHTML";
		
	for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();
		$fieldIt = $record->getFieldIterator();
		echo "<tr>";
		for ($fieldIt->reset(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			echo "<td>".$field->$fmethod()."</td>";
		}
		echo "</tr>";
	}
?>
</table>
</div>

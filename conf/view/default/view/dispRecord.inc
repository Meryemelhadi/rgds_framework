<div align="record_display">
<table class="record_display">
<?php
	$records = $properties["records"];
	$fmethod  = $properties["fmethod"];
	if (!isset($fmethod))
		$fmethod = "toHTML";
		
	for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();
		$fieldIt = $record->getFieldIterator();
		for ($fieldIt->reset(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			if (!$field->isHidden())
			{
				echo "<tr>";
				echo "<th>".$field->getLabel()."</th>";
				echo "<td>".$field->$fmethod()."</td>";
				echo "</tr>";
			}
		}
	}
	
	echo "<tr><td colspan=\"2\">".$this->getButton("button")."</td></tr>";
?>
</table>
</div>

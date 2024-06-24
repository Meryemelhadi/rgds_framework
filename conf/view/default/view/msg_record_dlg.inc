<?php

if (isset($properties["msg"]))
{
	echo "<div class=\"msg\">".$properties["msg"] ."</div>";
}
?>

<div>
<?php
	$records = $properties["records"];
	$fmethod  = $properties["fmethod"];
	if (!isset($fmethod))
		$fmethod = "toHTML";
		
	for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();
		
		if (isset($properties["formHeader"]))
			echo $properties["formHeader"];
			
		echo "<table class=\"record_edit\">";
		$fieldIt = $record->getFieldIterator();
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
		}
		
		if (isset($properties["submitButton"]) || isset($properties["cancelButton"]))
		{
			echo "<tr><td>";
			echo $this->getButton($properties["cancelButton"]);
			echo "</td><td>";				
			echo $this->getButton($properties["submitButton"]);
			echo "</td></tr>";
		}			
		echo "</table>";
		
		if (isset($properties["formFooter"]))
		{
			echo $properties["formFooter"];
		}
	}
?>
</table>
</div>
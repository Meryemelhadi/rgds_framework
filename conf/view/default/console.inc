<?php
	echo $this->getButton("home");
	echo $this->getButton("newRecord");
	echo $this->getButton("list");
	echo "<span class=\"msg\">" . $this->getProperty("Error.navigation","")."</span>";
?>

<?php
$records = & $this->getParam("records");

if ($records && $records->count() > 0)
{
	echo "<table class=\"console\" cellspacing=\"1\">";
	$fmethod  = $this->getParam("fmethod","toHTML");
	$recIt = $records->getRecordIterator();
	$record = & $recIt->getCurrent();
	
	echo "<tr>";
	for ($fieldIt = $record->getFieldIterator(); $fieldIt->isValid(); $fieldIt->next())
	{
		$field = & $fieldIt->getCurrent();
		if (!$field->isHidden())
			echo "<th>".$field->getLabel()."</th>";
		
	}
	echo "<th colspan=\"3\">operations</th>";
	echo "</tr>";		
	
	for (; $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();
		echo "<tr>";
		$paramKey = "";
		for ($fieldIt = $record->getFieldIterator(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			if (!$field->isHidden())
				echo "<td>".$field->$fmethod()."</td>";

			if ($field->isKey())
				$paramKey .= "&amp;".$field->toUrlParam();
		}
		echo "<td>".$this->getButton("viewRecord",null,$paramKey)."</td>";
		echo "<td>".$this->getButton("editRecord",null,$paramKey)."</td>";
		echo "<td>".$this->getButton("deleteRecord",null,$paramKey)."</td>";
		echo "</tr>";
	}
	echo "</table>";
}
else
{
	echo "<br/>".$module->getString("Empty table");
}
?>

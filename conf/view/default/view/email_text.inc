<?php
	echo "subject: ".$properties["email.subject"]."\n";
	
	$records = $properties["records"];
	$fmethod  = $properties["fmethod"];
	if (!isset($fmethod))
		$fmethod = "toString";
		
	for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();

		$fieldIt = $record->getFieldIterator();
		$hiddenFields = "";
		for ($fieldIt->reset(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			if (!$field->isHidden())
			{
				echo $field->getLabel().": ".$field->$fmethod()."\n";
			}
		}
	}
?>
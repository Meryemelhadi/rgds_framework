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
		
	$isHelp=false;
	if ($fmethod=="toForm")
	{
		$isForm = true;
	}
	else
	{
		$isForm = false;
	}
		
	for ($recIt = $records->getRecordIterator(); $recIt->isValid(); $recIt->next())
	{
		$record = & $recIt->getCurrent();
		
		if (isset($properties["formHeader"]))
			echo $properties["formHeader"];
			
		if ($isForm)
			echo "<table class=\"formBox\"><tr><td>";
			
		echo "<table class=\"record_edit\">";
		$fieldIt = $record->getFieldIterator();
		$hiddenFields = "";
		for ($fieldIt->reset(); $fieldIt->isValid(); $fieldIt->next())
		{
			$field = & $fieldIt->getCurrent();
			if (!$field->isHidden())
			{
				if ($isForm && ($h=$field->getProperty("help",null,false))!=null)
				{
					$hi="&nbsp;<img onclick='helpPlease(this,\"".$h."\",\"".$field->getLabel()."\")' src=\"".NX_IMAGES."help.gif\"/>";
					$isHelp=true;
					echo "<tr><td>";
				}
				else
				{
					$hi = "";
					echo "<tr><td>";
				}
					
				echo $field->getLabel();
				if ($isForm)
				{
					if ($field->isRequired())
						echo "<span class=\"required\">*<span>";
					echo "</td><td>";
					echo $field->$fmethod().$hi."</td>";
					echo "</tr>";
				}
				else
				{
					echo "<td>".$field->$fmethod()."</td>";
					echo "</tr>";
				}
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
		if ($isForm)
			if ($isHelp)
				echo "</td><td id=\"helpBox\" class=\"helpBox\"><script>writeHelpBox();</script></td></tr></table>";
			else
				echo "</td><td id=\"helpBox\" class=\"helpBox\">&nbsp;</td></tr></table>";
		
		if (isset($properties["formFooter"]))
		{
			echo $hiddenFields;
			echo $properties["formFooter"];
		}
	}
?>
</div>
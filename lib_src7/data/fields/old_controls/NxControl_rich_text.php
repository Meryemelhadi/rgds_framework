<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_rich_text
{
	function NxControl_rich_text()
	{
	}
	
	function toHTML()
	{
	}
	
	function toForm(&$value,&$desc)
	{
		// $F_ = $desc->name;
		$fname = $desc->getAlias();
	
		// $inputVal = eregi_replace("\"", "&quot;", $this->value);
		$inputVal = $value;
		
		if ($desc->isHidden())
			return "<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
	
		if (!$desc->isEdit())
			return $this->toHTML()."<input type=\"hidden\" name=\"".$fname."\" value='".$inputVal."' />";
			
		$style = $desc->getStyle("richText","text");
		$colsMaxBox= intval($desc->getProperty("colsMaxBox",30));
		$rowsMaxBox= intval($desc->getProperty("rowsMaxBox",5));
		
		$res = 
'<table UNSELECTABLE="on" class="richText" '.$style.' cellspacing="0" cellpadding="0">
<tbody UNSELECTABLE="on" ><tr UNSELECTABLE="on"><td UNSELECTABLE="on" width="25">&nbsp;</td>
<td><table UNSELECTABLE="on" class="toolbar" id="tb_horiz" cellspacing="1" cellpadding="0"><tr UNSELECTABLE="on">';
	
		$op_horiz = array(
				"Cut","Copy","Paste",
				"|","Undo","Redo",
				"|","Bold","Italic","Underline","StrikeThrough","SuperScript","SubScript",
				"|","CreateLink");
				
		$op_ext=array(
		);

		if (defined('NX_HTML_CLEANUP') && NX_HTML_CLEANUP===true)
		{
			$op_ext[]=array('op'=>'RT_cleanHTML','label'=>'cleanup imported HTML');
		}
		
		if (defined('NX_IMAGE_LIB') && NX_IMAGE_LIB===true)
		{
			$op_ext[]=array('op'=>'RT_insertImg','label'=>'insert an image');
		}
				
		$op_vert = array(
			"InsertOrderedList","InsertUnorderedList","Indent","Outdent",
			"JustifyLeft","JustifyCenter","JustifyRight"
		);

		//$NX_IMAGES = NX_BASE_URL."nx/images/";
		$NX_IMAGES = NX_DEFT_SKIN_URL."images/";
		
		foreach ($op_horiz as $op)
		{
			if ($op != "|")
			{
				$res .= "<td UNSELECTABLE='on'><img UNSELECTABLE='on' src=\"".$NX_IMAGES.$op.".gif\" title=\"".$op.
					"\" onclick=\"RT_execCommand('".$op."');\"/></td>";
			}
			else
			{
				$res .= "<td UNSELECTABLE='on' class=\"separator\"><img UNSELECTABLE='on' width=\"10\" height=\"22\" src=\"".$NX_IMAGES."Separator.gif\" /></td>";
			}
		}
		
		foreach ($op_ext as $opd)
		{
		$res .= "<td UNSELECTABLE='on'><img UNSELECTABLE='on' src=\"".$NX_IMAGES.$opd['op'].".gif\" title=\"".$opd['label']."\" onclick=\"RT_execCommand('".$opd['op']."');\"/></td>";
		}
			
		$res .= "</tr></table></td></tr>";
		
		$res .= "<tr UNSELECTABLE='on' style=\"height:".$rowsMaxBox."em;width:20px;\" valign=\"top\"><td UNSELECTABLE='on' width=\"25\"><table UNSELECTABLE='on' class=\"toolbar\"  id=\"tb_vert\" cellspacing=\"1\" cellpadding=\"0\">";
		foreach ($op_vert as $op)
		{
			if ($op != "|")
			{
				$res .= "<tr UNSELECTABLE='on'><td UNSELECTABLE='on' width=\"24\"><img UNSELECTABLE='on' width=\"23\" height=\"22\" src=\"".$NX_IMAGES.$op.".gif\" title=\"".$op.
					"\" onclick=\"RT_execCommand('".$op."');\"/></td></tr>";
			}
		}
		
		$res .= "</table></td><td>";
		$res .= "<div UNSELECTABLE='off' id=\"edit_".$fname."\" style=\"height:100%;overflow:auto;\" CONTENTEDITABLE class=\"rich_text\">".$value."</div>";
		$res .= "<input id=\"".$fname."\" name=\"".$fname."\" type=\"hidden\" value='".$inputVal."' />";
		$res .= "</td></tr></tbody></table>";
		
		return $res;
	}

	function readForm(&$recData,&$desc)
	{
		//$fname = $desc->name;
		$fname = $desc->getAlias();
		
		if (!isset($recData[$fname]) || ($value = $recData[$fname])=="")
			return null;

		return
			preg_replace(
				array(
					'/<([\w]+) (?:style|class)=([^>]*)([^>]*)/i',
					'/<\\?\??xml[^>]*>/i',
					'/<script[^>]*>[^<]*<\/script>/i',
					'/<\/?\w+:[^>]*>/i',
					'/<\/?font[^>]*>/i',
					
					"/'/",
					"/[<]\?/",
					"/<script/i",
					),
				array(
					'<$1$3',
					'',
					'',
					'',
					'',
					
					"&#39;",
					"&lt;&#3f;",
					"&lt;script",
					),
					$recData[$fname]
				);
								
/*				
//				ereg_replace("\"","&quot;",
					ereg_replace("'","&#39;",
						ereg_replace( "[<]\?","&lt;&#3f;", 
							ereg_replace( "<script","&lt;script",
								$recData[$fname]
							)
						)
//					)
				);
*/
	}
}

?>
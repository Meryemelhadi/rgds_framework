<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/** Data field : choice list */
class NxDataField_text_choice_sub_req extends NxDataField_text_choice
{
	function __construct(&$desc)
	{
		$this->desc =& $desc;
		$this->desc->sep=$desc->getProperty('csv_sep','|',false);
	}
	
	function getDS($attribute,&$media,&$view) 
	{			
		// get ds name
		if (($dsn=$this->desc->getProperty($attribute,null,false))==null)
			$dsn = $attribute;
			
		// get ds settings
		if ($dsn!='')
		{
			$ads=explode(':',$dsn);
			switch(count($ads))
			{
				case 2:
					list($media,$view)=$ads;
					break;
				case 1:
					list($view)=$ads;
					$media='db';
					break;
				default:
					return null;
			}
		}
		else
		{
			return null;
		}

		// get db data source plugin		
		$ds =& $this->desc->getPlugIn(strtolower($media),'NxDS_','ds');
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","field_text_choice_table");
			return null;
		}
		
		return $ds;
	}	

}

?>
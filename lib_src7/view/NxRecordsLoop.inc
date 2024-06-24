<?php

class NxRecordsLoop {
	var $records;
	var $record;
	var $fields;
	var $pages;

	function __construct($records=null,$record=null) {
		$this->records = $records;
		$this->record = $record;
		$this->recIndex=0;

		// records loop
		if ($records!==null)
		{
			if($records->count() > 0)
			{
				// pages variables
				if ($records->isPaged() > 0)
				{
					$_nxvar_context =& $$records->getPageContext();
					$_nxvar_page_index=$_nxvar_context["currentPageOffset"];
					$this->paged=true;
					$this->nbpages=$_nxvar_context["nbPages"];
					$this->page_alias= $_nxvar_page_alias = $_nxvar_context["pageAlias"];
					
					// remove page parameter
					$this->page_url_= str_replace(array(\'&&\',\'&\'),array(\'&\',\'&amp;\'),preg_replace("/${_nxvar_page_alias}=[^&]+&?/",\'\',$GLOBALS["NX_CURRENT_URL"]).(strpos($GLOBALS["NX_CURRENT_URL"],\'?\')===FALSE?\'?\':\'&\')."$_nxvar_page_alias=");
				}
				else {
					$this->nbpages=1;
				}
			}
		}

		if ($record!==null)
		{
		}
	}

	function __set($k,$v) {
	}
	
	function __get($k) {
		switch($k) {
			case 'fieldCount': return $this->fieldCount=$this->record->fieldCount() ;
			case 'record_key': return $this->record_key=$this->record->getUrlKey() ;
			case 'iterRec': return $this->iterRec = $this->records->getRecordIterator();
			case '': return $this-> ;
			case '': return $this-> ;
			case '': return $this-> ;
		}
	}

}

?>



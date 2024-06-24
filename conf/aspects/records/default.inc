<?php
 /* This class must be overloaded by record aspects */
  class aspects_records_default extends NxModule {
	  var $record;

  	  function aspects_records_default(&$record,&$module){
  	  	  $this->NxModule($module->props,$module->parent);
  	  	  $this->record = &$record;
	  }
	  function onNew(&$errorCB) {
	  	  nxLog("new record",'aspects');
	  }

	  function onBeforeSave($isInsert,&$errorCB) {
	  	  nxLog("onBeforeSave : ",'aspects');
	  	  return true;
	  }
	  function onSave($isInsert,&$errorCB) {
	  	  nxLog("onSave",'aspects');
	  }

	  function onBeforeDelete(&$errorCB) {
	  	  nxLog("onBeforeDelete",'aspects');
	  	  return true;
	  }
	  function onDelete(&$errorCB) {
	  	  nxLog("onDelete",'aspects');
	  }
  }
?>
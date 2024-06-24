<?php
 /* This class must be overloaded by record aspects */
  class NxData_RecordAspect{
  	  var $record;
  	  
  	  function __construct(&$record,&$context){
  	  	  $this->record = &$record;
  	  	  $this->context=&$context;
	  }
	  function onNew(&$errorCB) {
	  }

	  function onBeforeSave($isInsert,&$errorCB) {
	  	  return true;
	  }
	  function onSave($isInsert,&$errorCB) {
	  }

	  function onBeforeDelete(&$errorCB) {
	  	  return true;
	  }
	  function onDelete(&$errorCB) {
	  }
  }
?>
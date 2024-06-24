<?php
// package
$package = 'evalmag';

// prefix for dml views .record, .list, .search.
$base_data = 'modelcomp';

// data sources (optional)
$datasources = array(
	// WORKFLOW
	'header'   => 'modelcomp.record@evalmag', // state header

	// ADD
	'new'	   => 'modelcomp.add@evalmag',    // new record form
	'add_process'   => 'modelcomp.record@evalmag',  // save new form

	// EDIT
	'record_ws'   => 'modelcomp.record@evalmag',       // default record form
	'edit_process'   => 'modelcomp.record@evalmag', // save edited form

	// LIST
	'search'   => 'modelcomp.search@evalmag', // search form
	'excel'    => 'modelcomp.excel@evalmag',  // excel list export
	'list_ws'	   => 'modelcomp.list@evalmag',   // list

	// web service application
	'image_ws'	   => 'modelcomp.image_by_oid@evalmag',   // list

);

// strings name
$strings = 'modelcomp@evalmag';

// permissions foir the application
$permissions='';

// same role as in www .php file
$main_role='editor';

// form definition
$forms = array();

// optional views 
$views = array
(
	'excel' => 'exports.excel.records',
);

// base application
include GENERIS_PACKAGES.'workflow/application/base_application_crud_angular.inc';
?>
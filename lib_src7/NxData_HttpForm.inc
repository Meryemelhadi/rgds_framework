<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/* 
 * Datasource: manage data coming from HTTP POST/GET/COOKIE variables.
 * Author: Franck Yvetot
 * Copyright NxFrontier
 */

/*
usage:

// get field container
var $formContainer = new NxData_HttpForm("POST",array("firstName","lastName","email"));

// loop on records
for (var $recIt = $formContainer->getRecordIterator();
	recIt->isValid();
	recIt->next())
	{
		// loop on fields
		for (var $fieldIt = $recIt->getFieldIterator();
			$fieldIt->isValid();
			$fieldIt->next())
			{
				// use field value
				print $fieldIt->getValue();				
			}
		
	}

*/

// Obsolete class ?

require_once(NX_LIB . "NxData_RecordInputSource.inc");

// form data container
class NxData_HttpForm extends NxData_RecordInputSource
{
	function __construct($httpMethod,&$recDesc, &$errorCB) 
    {
		parent::__construct($recDesc);

		if (!isset($httpMethod))
		{
			$httpMethod = "POST";
		}

		/*
		$arrayName = "HTTP_". $httpMethod ."_VARS";
		*/
		$arrayName = "_$httpMethod";
		if (isset($GLOBALS[$arrayName]))
			$formArray =& $GLOBALS[$arrayName];
		else
			$formArray = array();

		nxLog("init record from http array: $arrayName",'NX_DATA');
	 	$this->initRecord($formArray, "form", $errorCB);
	 	
	 	if ($errorCB->isError())
	 		nxError('error in record init :'.$errorCB->getErrorMsg(),'NX_DATA');
	 	else
			nxLog('record initialised','NX_DATA');
    }
}

?>
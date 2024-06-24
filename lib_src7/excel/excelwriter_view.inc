<?php
	
  /*Writes excel Xml file and outputs to browser*/


class ExcelWriter_view
{
		
	var $error;
	var $newRow=false;
	var $filename;

	function ExcelWriter_view($filename="doc.xls")
	{
		$this->filename = $filename;
		ob_start();
	}
	
	function run($view='export.xls')
	{
		$buffer=ob_get_contents();
		ob_clean();
	  
		//We send to a browser
		header('Content-Type: application/vnd.ms-excel');
		// header('Content-Type: text/html');
		if(headers_sent())
			$this->Error('Some data has already been output to browser, can\'t send PDF file');
		header('Content-Length: '.strlen($buffer));
//		header('Content-disposition: inline; filename="'.$this->filename.'"');

		/* test for mac */
		header('Content-disposition: attachment; filename="'.$this->filename.'"');
		header("Expires: 0");
		header("Cache-Control: must-revalidate, post-check=0,pre-check=0");
		header("Pragma: public");
		
		echo $buffer;
	}

}
?>

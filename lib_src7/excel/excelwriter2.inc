<?php
	
  /*Writes excel Xml file and outputs to browser*/


class ExcelWriter2
{
		
	var $error;
	var $newRow=false;
	var $filename;

	function ExcelWriter2($filename="doc.xls")
	{
		$this->filename = $filename;
		  //envoi de l'entete Excel
		  // header("Content-Disposition: attachment; filename=".$filename);
		ob_start();
		echo $this->GetHeader();
	}
	
	function Output()
	{
		$this->AddData($this->GetFooter());
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

function AddData($str)
{
  echo $str;
}

		function GetHeader() //ISO-8859-1 us-ascii    text/html   application/vnd.ms-excel
		{
			$header = <<<EOH
        <html xmlns:o="urn:schemas-microsoft-com:office:office"
				xmlns:x="urn:schemas-microsoft-com:office:excel"
				xmlns="http://www.w3.org/TR/REC-html40">

				<head>
				<meta http-equiv="Content-Type" content="application/vnd.ms-excel; charset=iso-8859-1">
				<meta name="ProgId" content="Excel.Sheet">
				<!--[if gte mso 9]><xml>
				 <o:DocumentProperties>
				  <o:LastAuthor>Salamander</o:LastAuthor>
				  <o:LastSaved></o:LastSaved>
				  <o:Version></o:Version>
				 </o:DocumentProperties>
				 <o:OfficeDocumentSettings>
				  <o:DownloadComponents/>
				 </o:OfficeDocumentSettings>
				</xml><![endif]-->
				<style>
				<!--table
					{mso-displayed-decimal-separator:"\.";
					mso-displayed-thousand-separator:"\,";}
				@page
					{margin:1.0in .75in 1.0in .75in;
					mso-header-margin:.5in;
					mso-footer-margin:.5in;}
				tr
					{mso-height-source:auto;}
				col
					{mso-width-source:auto;}
				br
					{mso-data-placement:same-cell;}
				.style0
					{mso-number-format:General;
					text-align:general;
					vertical-align:bottom;
					white-space:nowrap;
					mso-rotate:0;
					mso-background-source:auto;
					mso-pattern:auto;
					color:windowtext;
					font-size:10.0pt;
					font-weight:400;
					font-style:normal;
					text-decoration:none;
					font-family:Arial;
					mso-generic-font-family:auto;
					mso-font-charset:0;
					border:none;
					mso-protection:locked visible;
					mso-style-name:Normal;
					mso-style-id:0;}
				td
					{mso-style-parent:style0;
					padding-top:1px;
					padding-right:1px;
					padding-left:1px;
					mso-ignore:padding;
					color:windowtext;
					font-size:10.0pt;
					font-weight:400;
					font-style:normal;
					text-decoration:none;
					font-family:Arial;
					mso-generic-font-family:auto;
					mso-font-charset:0;
					mso-number-format:General;
					text-align:general;
					vertical-align:top;
					border:none;
					mso-background-source:auto;
					mso-pattern:auto;
					mso-protection:locked visible;
					white-space:nowrap;
					mso-rotate:0;}
				.xl24
					{mso-style-parent:style0;
					white-space:normal;}				
				.xl25
					{mso-style-parent:style0;
					white-space:normal;
					font-weight:800;}
				-->
				</style>
				<!--[if gte mso 9]><xml>
				 <x:ExcelWorkbook>
				  <x:ExcelWorksheets>
				   <x:ExcelWorksheet>
					<x:Name>Sheet 1</x:Name>
					<x:WorksheetOptions>
					 <x:Selected/>
					 <x:ProtectContents>False</x:ProtectContents>
					 <x:ProtectObjects>False</x:ProtectObjects>
					 <x:ProtectScenarios>False</x:ProtectScenarios>
					</x:WorksheetOptions>
				   </x:ExcelWorksheet>
				  </x:ExcelWorksheets>
				  <x:WindowHeight>10005</x:WindowHeight>
				  <x:WindowWidth>10005</x:WindowWidth>
				  <x:WindowTopX>120</x:WindowTopX>
				  <x:WindowTopY>135</x:WindowTopY>
				  <x:ProtectStructure>False</x:ProtectStructure>
				  <x:ProtectWindows>False</x:ProtectWindows>
				 </x:ExcelWorkbook>
				</xml><![endif]-->
				</head>

				<body link="blue" vlink="purple">
				<table x:str border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse;table-layout:fixed;">
EOH;
			return $header;
		}


		function GetFooter()
		{
			return "</table></body></html>";
		}
		
		function writeLine($line_arr)
		{
			if(!is_array($line_arr))
			{
				$this->error="Error : Argument is not valid. Supply an valid Array.";
				return false;
			}
			$this->AddData("<tr>");
			foreach($line_arr as $col)
        $this->writeCol($col);
        //$this->AddData("<td class=xl24 width=64 >$col</td>");
			$this->AddData("</tr>");
		}

		function writeRow()
		{
			if($this->newRow==false)
				$this->AddData("<tr>");
			else
				$this->AddData("</tr><tr>");
			$this->newRow=true;	
		}

		function writeHeaderCell($value)
		{
			$value = str_replace("\n","<br>",$value);
			$this->AddData("<td class=\"xl25\" width=\"64\" >$value</td>");
		}
		
		function writeCol($value)
		{
			$value = str_replace("\n","<br>",$value);
			$this->AddData("<td class=\"xl24\" width=\"64\" >$value</td>");
		}
	}
?>

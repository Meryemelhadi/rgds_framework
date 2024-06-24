<?php
// ini_set('display_startup_errors', TRUE);
include_once NX_DIS .'phpexcel/Classes/PHPExcel.php';

class gen_xslx_list
{
	function __construct($config=null) {
		$this->config = $config;
	}

	function initConfig($config) {
		if(!isset($config->autoWidth))
			$config->autoWidth = false;

		return $this->config = $config;
	}

	function sendFile($records,$fileName,$config=null) {
		if(!$config)
			$config = $this->config;

		$fileName = preg_replace('/\.xlsx?/i','',$fileName).'.xlsx';

		$objPHPExcel = $this->generate($records,$config,$fileName);

		// Redirect output to a client’s web browser (Excel2007)
		header('Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet');
		header('Content-Disposition: attachment;filename="'.$fileName.'"');
		header('Cache-Control: max-age=0');
		// If you're serving to IE 9, then the following may be needed
		header('Cache-Control: max-age=1');

		// If you're serving to IE over SSL, then the following may be needed
		header ('Expires: Mon, 26 Jul 1997 05:00:00 GMT'); // Date in the past
		header ('Last-Modified: '.gmdate('D, d M Y H:i:s').' GMT'); // always modified
		header ('Cache-Control: cache, must-revalidate'); // HTTP/1.1
		header ('Pragma: public'); // HTTP/1.0

		$objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, 'Excel2007');
		$objWriter->setIncludeCharts(true);
		$objWriter->save('php://output');
	}

	function generate($records,$config,$fileName=null) 
	{
		if(isset($config->pathModel))
		{
			$objReader = PHPExcel_IOFactory::createReader('Excel2007');
			$objReader->setIncludeCharts(TRUE);
			$objPHPExcel = $objReader->load($config->pathModel);

			$objWriter = PHPExcel_IOFactory::createWriter($objPHPExcel, 'Excel2007');
			$objWriter->setIncludeCharts(true);
			$objWriter->save("C:\\Users\\Franck\\Downloads\\_TEST_".$fileName);
		}
		else
		{
			$objPHPExcel = new PHPExcel();
		}

		if(method_exists($config,'prepareData')) {
			$records = $config->prepareData($objPHPExcel,$records);
		}


		if(!isset($config->sheets)) {
			$config->sheets[] = $config;
		}

		// SHEETS (defaut : only one)
		foreach($config->sheets as $i=>$sheetConfig) 
		{
			$objPHPExcel->setActiveSheetIndex($i);
			$sheet = $objPHPExcel->getActiveSheet();

			if(isset($sheetConfig->sheetName))
				 $sheet->setTitle($sheetConfig->sheetName);
		
			// DATA
			if(!isset($sheetConfig->records) || $sheetConfig->records)
			{
				$this->addSheetData($sheet,$records,$sheetConfig);
			}

			// CHARTS
			if(isset($sheetConfig->charts))
			{
				$charts = $sheetConfig->charts;
				foreach($charts as $chart) 
				{
					$this->addChart(
						$sheet,
						$chart['title'],
						$chart['labels'],
						$chart['data'],
						$plotType=PHPExcel_Chart_DataSeries::TYPE_SCATTERCHART,
						$serieName='Série 1');
				}
			}
		}

		$this->setFormat($sheet,$records,$config);

		return $objPHPExcel;
	}

	function setCellValue($cell,$f)
	{
		$v = $f->html;			

		if(preg_match('/[0-9 ]+%$/',$v))
		{
			if(preg_match('/^[0-9 ]+%$/',$v))
				$type=PHPExcel_Style_NumberFormat::FORMAT_PERCENTAGE;
			else
				$type=PHPExcel_Style_NumberFormat::FORMAT_PERCENTAGE_00;

			$v=str_replace('%','',$v);
			$v=$v/100;

			$cell->setValue($v);

			$cell->getStyle()->getNumberFormat()->applyFromArray([
				"code" => $type
			]);
		}
		else
		{
			$cell->setValue($v);
		}
	}

	function setLabelRow($sheet,$line,$offset) {
		$c = $offset[0];
		$row0 = $offset[1];
		foreach($line->fields as $id=>$f)
		{
			if($f->isHidden())
				continue;

			$cell = $sheet->getCellByColumnAndRow($c,$row0);
			
			$cell->setValue($f->label);

			// set bold, wrap
			$cell->getStyle()->getFont()->setBold(true);
			$cell->getStyle()->getAlignment()->setWrapText(true);

			$c++;
		}
	}

	function setDataRows($sheet,$data,$offset) {
		$col0 = $offset[0];
		$row0 = $offset[1];
		foreach($data as $l=>$line) 
		{
			$c = $col0;
			foreach($line->fields as $id=>$f)
			{
				if($f->isHidden())
					continue;
				
				$cell = $sheet->getCellByColumnAndRow($c,$l+$row0);

				$this->setCellValue($cell,$f);

				$type = $f->desc->getType();
				if($type=='text')
					$cell->getStyle()->getAlignment()->setWrapText(true);

				$c++;
			}
		}
	}

	function addSheetData($sheet,$records,$config) {
		$data = $records->array;
		$line = $data[0];

		// data offset
		if(!isset($config->offsets))
			$config->offsets = array();

		// labels
		if(!isset($config->offsets['labels']))
			$config->offsets['labels'] = array(0,0);

		$this->setLabelRow($sheet,$line,$config->offsets['labels']);

		// values
		if(!isset($config->offsets['data']))
			$config->offsets['data'] = array($config->offsets['labels'][0],$config->offsets['labels'][1]+1);

		$this->setDataRows($sheet,$data,$config->offsets['data']);
	}

	function setFormat($sheet,$records,$config) {
		if(method_exists($config,'setFormat')) 
		{
			$config->setFormat($this,$sheet,$records,$config);
		}
		else
		{
			// dimensions auto
			if($config->autoWidth)
				$this->setFormatAutoWidth($sheet,$config);
		}
	}

	function setFormatAutoWidth($sheet,$config) {

		// dimensions auto
		foreach (range('A', $sheet->getHighestDataColumn()) as $col) 
		{
			$dim = $sheet->getColumnDimension($col);
			$dim->setAutoSize(true);
		}
		$sheet->calculateColumnWidths();

		if(!$config->widthFactor)
			$config->widthFactor=0.95;

		foreach (range('A', $sheet->getHighestDataColumn()) as $col) 
		{
			$dim = $sheet->getColumnDimension($col);
			$this->setWidthFactor($dim,$config->widthFactor);
		}
	}

	function setWidthFactor($dim,$factor=1) {
		$width = $dim->getWidth();
		if($width!=-1)
		{
			$dim->setWidth($width*$factor);
		}
	}

	function setFormatFixedWidth($sheet,$w=20) {
		// dimensions auto
		foreach (range('A', $sheet->getHighestDataColumn()) as $col) 
		{
			$dim = $sheet->getColumnDimension($col);
			$dim->setWidth($w);
		}
	}

	function setSheet() {
	}

	var $chartIdx=1;
	function addChart(
		$sheet,
		$title,
		$labelRange,
		$dataRange,
		$plotType=PHPExcel_Chart_DataSeries::TYPE_PIECHART,
		$serieName='Série 1') 
	{

		//	Set the Labels for each data series we want to plot
		//		Datatype
		//		Cell reference for data
		//		Format Code
		//		Number of datapoints in series
		//		Data values
		//		Data Marker
		$dataSeriesLabels1 = array(
			new PHPExcel_Chart_DataSeriesValues('String', $serieName /*'Worksheet!$C$1'*/, NULL, 1),	//	2011
		);
		//	Set the X-Axis Labels
		//		Datatype
		//		Cell reference for data
		//		Format Code
		//		Number of datapoints in series
		//		Data values
		//		Data Marker
		$xAxisTickValues1 = array(
			new PHPExcel_Chart_DataSeriesValues('String', $labelRange /*'Worksheet!$A$2:$A$5'*/, NULL, 4),	//	Q1 to Q4
		);
		//	Set the Data values for each data series we want to plot
		//		Datatype
		//		Cell reference for data
		//		Format Code
		//		Number of datapoints in series
		//		Data values
		//		Data Marker
		$dataSeriesValues1 = array(
			new PHPExcel_Chart_DataSeriesValues('Number', $dataRange /*'Worksheet!$C$2:$C$5'*/, NULL, 4),
		);

		//	Build the dataseries
		$series1 = new PHPExcel_Chart_DataSeries(
			$plotType,				// plotType
			NULL,			                                        // plotGrouping (Pie charts don't have any grouping)
			range(0, count($dataSeriesValues1)-1),					// plotOrder
			$dataSeriesLabels1,										// plotLabel
			$xAxisTickValues1,										// plotCategory
			$dataSeriesValues1										// plotValues
		);

		//	Set up a layout object for the Pie chart
		$layout1 = new PHPExcel_Chart_Layout();
		$layout1->setShowVal(TRUE);
		$layout1->setShowPercent(TRUE);

		//	Set the series in the plot area
		$plotArea1 = new PHPExcel_Chart_PlotArea($layout1, array($series1));
		//	Set the chart legend
		$legend1 = new PHPExcel_Chart_Legend(PHPExcel_Chart_Legend::POSITION_RIGHT, NULL, false);

		$title1 = new PHPExcel_Chart_Title($title /*'Test Pie Chart'*/);

		//	Create the chart
		$chart1 = new PHPExcel_Chart(
			'chart'.$this->chartIdx,		// name
			$title1,		// title
			$legend1,		// legend
			$plotArea1,		// plotArea
			true,			// plotVisibleOnly
			0,				// displayBlanksAs
			NULL,			// xAxisLabel
			NULL			// yAxisLabel		- Pie charts don't have a Y-Axis
		);

		$this->chartIdx++;

		//	Set the position where the chart should appear in the worksheet
		$chart1->setTopLeftPosition('B22');
		$chart1->setBottomRightPosition('H33');

		//	Add the chart to the worksheet
		$sheet->addChart($chart1);
	}
}

?>
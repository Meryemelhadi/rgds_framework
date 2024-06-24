<html>
<head>
<title><?php echo $this->getProperty("page.title","") ?></title>
<head>
<?php
	echo $this->getSyleSheets("page.head.css");
	echo $this->getScripts("page.head.scripts");
?>
</head>

<body <?php echo $this->getProperty("page.body.attributes","");?>>
<?php echo $this->getProperty("page.content","empty page"); ?>
</body>
</html>
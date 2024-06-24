<?php

class Template_console {

	function menu($widget)
	{
		$liens = $widget->menu;
		foreach ($liens as $a)
		{
		$url=$a['url'];
		$label=$a['label'];
		?>
		
		<a class="pm-button" href="{$url}">
			<span class="pm-button-background">
				<span class="pm-button-content pm-button-icon pm-button-default">{$label}</span></span></a>
		<?php
		}
	}

	function actions($widget)
	{
		$liens = $widget->actions;
		foreach ($liens as $a)
		{
		$url=$a['url'];
		$label=$a['label'];
		?>
		
		<a class="pm-button" href="{$url}">
			<span class="pm-button-background">
				<span class="pm-button-content pm-button-icon pm-button-default">{$label}</span></span></a>
		<?php
		}
	}

	function user($user)
	{
		echo "$user ID=".$user->empid->string ." HR Couters :".$user->displayCounters();
		$url='?';
		$oid=$user->oid->object;

?>
			<a href="{$url}&amp;task=display_user&amp;rh_user_oid={$oid}">Display
</a> | 
			<a href="{$url}&amp;task=select_user&amp;rh_user_oid={$oid}">Select
</a> | 
<?php
	}
}
?>

<?php $_nx_content_stack[]="page.content";	ob_start(); ?>

	
	<?php ob_start(); ?>

		
<div class="pm-page-title-wrapper theme-turquoise">
	<h1 class="pm-title">Gestion des Congés
	</h1>
	<span class="pm-droiteH1"></span></div>
<div class=""></div>
<div class="clearBoth"></div>
<p class="pm-page-message">
	<span class="pm-droiteIntro"></span>{string:"page_message_view"}
	<br/>{string:"page_message_view_details"}</p>
		<?php $t=new Template_console(); $t->menu($this->getProperty('page.menu')); ?>
		<?php $t->user($this->getProperty('page.user')); ?>

		{=page.content}
	<?php 	
		$__buf=ob_get_contents();
		ob_end_clean();
		echo preg_replace(
			array('/[ \t]+/'),
			array(' '), $__buf);
	; ?>
<?php $this->setProperty(array_pop($_nx_content_stack),ob_get_contents());ob_end_clean();?>

<!DOCTYPE  html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" >

	<head>
		<title>{=page.title}</title>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
		<link rel="stylesheet" type="text/css" href="/extJS/resources/css/ext-all.css"/>
		 {require:css/page.css:skin} 
		 {require:js/page_all.js:skin}  	
		
		<script src="/nx/controls/tinymce/js/tiny_mce/tiny_mce_src.js" type="text/javascript" language="Javascript"></script>
	</head>
	<body style="background:transparent;">
		<div id="content">
			{=page.content}
		</div>
	</body></html>
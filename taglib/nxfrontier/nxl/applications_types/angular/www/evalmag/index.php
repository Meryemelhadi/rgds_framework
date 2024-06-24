<?php
/* file automatically generated from NXL source [d:/webs/access/generis_2011/packages/evalmag/nxl/eval.nxl]. Do not edit it. */
		
$loginPerm='default';
$loginService='user';
global $NX_MIME;
$NX_MIME='text/html';
require_once('../../nx/NxSite.inc');

/* file automatically generated from NXL source [/home/webmaster/presencesoft/com.preprod_weldom/generis_2016/packages/formation_catalog/nxl/catalog.nxl]. Do not edit it. */
define('KEEP_SESSION',false);

/** MODIFIER CES VALEURS (si besoin)**/
$package = 'evalmag';
$app  = 'modelcomp';
/** FIN MODIF **/

if(isset($_GET['app']))
{
	$app =$_GET['app'];
}
elseif(isset($_GET['generic']) && ($_GET['generic']=='no' || $_GET['generic']=='false'))
{
	$app =$_GET['type'];
}

if(isset($_REQUEST['op']))
{
	/* application */
	if(!isset($_REQUEST['u']))
	{
		// mode session classique
		$loginPerm='default';
		$loginService='user';
	}
	// else : moe avec u/p fournis ou ukey

	require_once('../../nx/NxSite.inc');
	require_once(GENERIS_PACKAGES.'workflow/lib/api/APIBase.inc');

	/** MODIFIER CES VALEURS (si besoin)**/
	$role = 'ws';
	/** FIN MODIF **/

	$app=new APIBase(
		array(
			'nx.package' => $package,
			'application.id'=>"$app@$package",
			'application.role'=>$role,
			'application.dir'=>'application'
		),
		$siteDesc);
	$app->run();
}
else
{
	$loginPerm='default';
	$loginService='user';
	global $NX_MIME;
	$NX_MIME='text/html';
	require_once('../../nx/NxSite.inc');

	include_once GENERIS_PACKAGES."workflow/lib/workflow/ExecuteApp2.inc";

	/** MODIFIER CES VALEURS (si besoin)**/
	$role = 'editor';
	$skin = 'lightsky';
	/** FIN MODIF **/

	// nom application et package, par defaut, on utilise le nom du fichier comme application

	// use generic application application bootstrap that loads the application from the package
	$page=new ExecuteApp2(
		array(
			'app.id' => "$app@$package",
			'app.role' => $role,
			'app.defaultOperation'=>'main',
			'nx.package' => $package,
			'application.dir'=>'application',
			'page.skin' => $skin,
		),
		$siteDesc);

	/* call the application */
	try {
	$page->run();
	}
	catch(Exception $e) {
		echo $e;
	}
}
?>
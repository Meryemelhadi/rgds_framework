<?php

include_once('application_base.inc');

class application_angular extends application_base
{
	function page($package,$path,$role,$application,$skin,$fpath,$lib,$node)
	{
		$src= '<?php
/* file automatically generated from NXL source ['.$lib->context->getFileURI().']. Do not edit it. */
		
';	
		$lib->getAttribute($perms,$node,'permissions','default');

		// 'protected' => 'no',
		$lib->getAttribute($isprot,$node,'protected','no');
		switch(strtolower($isprot))
		{
			case '':
			case 'false':
			case 'public':
			case 'no':
				break;
			case 'true':
			case 'yes':
				$src .= "\$loginPerm='$perms';
\$loginService='user';
";
				break;
			default:
				$src .= "\$loginPerm='$perms';
\$loginService='{$isprot}';
";
		}
		
		if ($lib->getBool($isprot))
			$src .= '$loginPerm="default";
';
	    	
		// pop skin
		$lib->context->getLastResource('skin',true);

		// get mime and encoding
		$lib->getAttribute($type,$node,'type','html');
		switch(strtolower($type))
		{
			case 'js':
			case 'javascript':
				$mime='application/x-javascript';
				break;
			case 'css':
				$mime='text/css';
				break;
			case 'xml':
				$mime='text/xml';
				break;
			case 'html':
			default:
				$mime='text/html';
				break;
		}
		
		// get mime and encoding
		$lib->getAttribute($mime,$node,'mime',$mime);
		
		$src.="global \$NX_MIME;\n\$NX_MIME='$mime';\n";
		
		// get site conf path
		//$path=$lib->getLastResource('path',false);

		$sitePath = $lib->getSitePath($path).'NxSite.inc';
		$src.="require_once('$sitePath');";
				
		$content = <<<EOH

/* file automatically generated from NXL source [/home/webmaster/presencesoft/com.preprod_weldom/generis_2016/packages/formation_catalog/nxl/catalog.nxl]. Do not edit it. */
define('KEEP_SESSION',false);

/** MODIFIER CES VALEURS (si besoin)**/
\$package = '{$package}';
\$app  = '{$application}';
/** FIN MODIF **/

if(isset(\$_GET['app']))
{
	\$app =\$_GET['app'];
}
elseif(isset(\$_GET['generic']) && (\$_GET['generic']=='no' || \$_GET['generic']=='false'))
{
	\$app =\$_GET['type'];
}

if(isset(\$_REQUEST['op']))
{
	/* application */
	if(!isset(\$_REQUEST['u']))
	{
		// mode session classique
		\$loginPerm='default';
		\$loginService='user';
	}
	// else : moe avec u/p fournis ou ukey

	require_once('../../../nx/NxSite.inc');
	require_once(GENERIS_PACKAGES.'workflow/lib/api/APIBase.inc');

	/** MODIFIER CES VALEURS (si besoin)**/
	\$role = 'ws';
	/** FIN MODIF **/

	\$app=new APIBase(
		array(
			'nx.package' => \$package,
			'application.id'=>"\$app@\$package",
			'application.role'=>\$role,
			'application.dir'=>'application'
		),
		\$siteDesc);
	\$app->run();
}
else
{
	\$loginPerm='default';
	\$loginService='user';
	global \$NX_MIME;
	\$NX_MIME='text/html';
	require_once('../../../nx/NxSite.inc');

	include_once GENERIS_PACKAGES."workflow/lib/workflow/ExecuteApp2.inc";

	/** MODIFIER CES VALEURS (si besoin)**/
	\$role = 'editor';
	\$skin = 'lightsky';
	/** FIN MODIF **/

	// nom application et package, par defaut, on utilise le nom du fichier comme application

	// use generic application application bootstrap that loads the application from the package
	\$page=new ExecuteApp2(
		array(
			'app.id' => "\$app@\$package",
			'app.role' => \$role,
			'app.defaultOperation'=>'main',
			'nx.package' => \$package,
			'application.dir'=>'application',
			'page.skin' => \$skin,
		),
		\$siteDesc);

	/* call the application */
	try {
	\$page->run();
	}
	catch(Exception \$e) {
		echo \$e;
	}
}

EOH;
		$package=$lib->context->getLastResource('package',true);

		$src.= "\n$content?>";
		$src = $lib->cleanupSrc($src);
		$lib->context->saveSrc($fpath,$src);

		return $src;
	}


	function appDesc($package,$application,$processus,$role,$table,$base_data,$strings,$permissions) {

		return 								$s=<<<EOH
<?php
// package
\$package = '{$package}';

// prefix for dml views .record, .list, .search.
\$base_data = '{$package}';

// data sources (optional)
\$datasources = array(
	// WORKFLOW
	'header'   => '{$base_data}.record@$package', // state header

	// ADD
	'new'	   => '{$base_data}.add@$package',    // new record form
	'add_process'   => '{$base_data}.record@$package',  // save new form

	// EDIT
	'record_ws'   => '{$base_data}.record@$package',       // default record form
	'edit_process'   => '{$base_data}.record@$package', // save edited form

	// LIST
	'search'   => '{$base_data}.search@$package', // search form
	'excel'    => '{$base_data}.excel@$package',  // excel list export
	'list_ws'	   => '{$base_data}.list@$package',   // list

	// web service application
	'image_ws'	   => '{$base_data}.image_by_oid@$package',   // list

);

// strings name
\$strings = '{$strings}@{$package}';

// permissions foir the application
\$permissions='{$permissions}';

// same role as in www .php file
\$main_role='{$role}';

// form definition
\$forms = array();

// optional views 
\$views = array
(
	'excel' => 'exports.excel.records',
);

// base application
include GENERIS_PACKAGES.'workflow/application/base_application_crud_angular.inc';

EOH;
	}


	function data_model($package,$base_data,$table)
	{
		$xmlstart = '<?xml version="1.0"?' .'>';

		return 
			$s=<<<EOH
{$xmlstart}
<nxl version="1.0" xmlns="http://www.nxfrontier.com/nxl/nxl" package="{$package}">

<data xmlns="http://www.nxfrontier.com/dml/dml">
	
	<!-- tables definitions -->
	<tables  init="create">
		<table name="{$table}">
			<fields>
			
				<field name="oid" label="oid" type="oid" isKey="primary"/>

				<field name="name" label="name"  type="line" size="200" />
 
			</fields>
		</table>

	</tables>
		
	<!-- DML Views (gernerating SQL requests)-->
	<views>
		<view name="{$base_data}.list" table="{$table}">
			<key>
				<field name="name" label="name"  type="line" size="200" relation="like"/>
			</key>

			<pages alias="page" pageOffset="%{get:page}" recordsPerPage="15" />				
		
			<sort>
				<field name="name" label="name"  type="line" size="200" />
			</sort>
			<fields>
				<field name="oid" label="oid" type="oid" isKey="primary"/>

				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>

		<view name="{$base_data}.excel" table="{$table}">
			<key>
				<field name="name" label="name"  type="line" size="200" relation="like"/>
			</key>
			<sort>
				<field name="name" direction="asc"/>
			</sort>
			<fields>
				<field name="oid" label="object_id" type="oid" isKey="primary" show="hidden" isEdit="no" />
				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>
		
		<view name="{$base_data}.search" table="{$table}">
			<fields>
				<field name="name" label="name"  type="line" size="200" />
			</fields>
		</view>

		<view name="{$base_data}.record" table="{$table}">
			<key>
				<field name="oid"  value="%{request:oid}"/>
			</key>

			<fields>
				<field name="oid" label="oid" type="oid" isKey="primary"/>
				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>				

		<view name="{$base_data}.add" table="{$table}">
			<key>
				<field name="oid"  value="%{request:oid}"/>
			</key>

			<fields>
				<field name="oid" label="oid" type="oid" isKey="primary"/>
				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>				
	</views>
	
</data>
</nxl>
EOH;
	}

	function data_handlers($package,$base_data,$table)
	{
		return null;
	}
	function tasks_handlers($fs,$path,$package,$application)
	{
	}

	function strings($package,$strings,$string_base,$isFemale=false,$startsVowel=false)
	{
			$s=<<<EOH
{$xmlstart}
<nxl version="1.0"
	xmlns="http://www.nxfrontier.com/nxl/nxl" 
	package="{$package}"
>

	<!-- French Strings -->
	<i8n:strings name="{$strings}" lang="fr" xmlns="http://www.w3.org/1999/xhtml"
			xmlns:i8n="http://www.nxfrontier.com/nxl/i8n">

		<i8n:string id="empty_list" value="Aucun xxx n\'est enregistré" />
		<i8n:string id="export_excel" value="Export Excel" />
	
		<!-- les labels des champs de données -->
		<fields xmlns="http://www.nxfrontier.com/nxl/i8n">

			<field name="name" label="Nom"  />

		</fields>
			
	</i8n:strings>
	
</nxl>
EOH;

	$s = $this->remapStrings($s,$string_base);
	return $s;
	}
}

?>
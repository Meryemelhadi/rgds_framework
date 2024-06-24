<?php

include_once('application_base.inc');

class application_dashboard extends application_base
{
	function app_operation($package)
	{
		return null;
	}

	function appDesc($package,$application,$processus,$role,$table,$base_data,$strings,$permissions) {
		return 	$s=<<<EOH
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
	'record'   => '{$base_data}.record@$package',       // default record form
	'edit_process'   => '{$base_data}.record@$package', // save edited form

	// LIST
	'search'   => '{$base_data}.search@$package', // search form
	'excel'    => 'handler:{$base_data}.excel@$package',  // excel list export
	'list'	   => '{$base_data}.list@$package',   // list

	'dashboard'	   => 'handler:{$base_data}.dashboard@$package',   // dashboard
);

// strings name
\$strings = '{$strings}@{$package}';

// permissions foir the application
\$permissions='{$permissions}';

// same role as in www .php file
\$main_role='{$role}';

\$this->ctxt->setProperty('app.form.css',<<<ECSS
	/* CSS RULES here */
ECSS
);

\$this->ctxt->setProperty('app.form.js',<<<EJS
	/* js code */
EJS
);

// form definition
\$forms = array(
	'record' => array(
		'tabs' => array(
			'Informations' => array(
				'title' => 'Informations', 
				'fields' => array('*')
			),
		)
	)
);

// optional views 
\$views = array
(
	'dashboard' => 'dashboard.list@workflow',
	'excel' => 'exports.excel.records',
);

// set property for use by the workflow model
\$this->ctxt->setProperty('workflow.views',\$views);

// icons
\$icons = array(
	'add'=>'si-add',
	'edit'=>'si-page-white-edit',
	'Supprimer'=>'si-delete',
	'delete'=>'si-delete',
	'submit'=>'si-disk',
	'save'=>'si-disk',
	'save_continue'=>'si-disk',
	'export_excel'=>'si-page-white-excel'
);
\$this->ctxt->setProperty('workflow.icons', \$icons);

// id de la classe dashboard à appeller
\$dashboard_task_id = '{$application}';

// base application
include GENERIS_PACKAGES.'workflow/application/base_application_dashboard.inc';

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
				<field name="state"  type="line" size="250" default="%{property:workflow.default_state}" isEdit="no"/>

				<field name="name" label="name"  type="line" size="200" />
 
			</fields>
		</table>

	</tables>
		
	<!-- DML Views (gernerating SQL requests)-->
	<views>
		<!-- list of items -->
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
				<field name="state"  type="line" isEdit="no"/>

				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>

		<view name="{$base_data}.dashboard" table="{$table}" handler="lib.{$package}.records.{$base_data}_requests">
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


		<!-- export excel -->
		<view name="{$base_data}.excel" table="{$table}" handler="lib.{$package}.records.{$base_data}_requests">
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
		
		<!-- search form -->
		<view name="{$base_data}.search" table="{$table}">
			<fields>
				<field name="name" label="name"  type="line" size="200" />
				<field name="state"  alias="state" type="line" size="20" show="hidden"/>
			</fields>
		</view>

		<!-- record form -->
		<view name="{$base_data}.record" table="{$table}">
			<key>
				<field name="oid"  value="%{request:oid}"/>
			</key>

			<fields>
				<field name="oid" label="oid" type="oid" isKey="primary"/>
				<field name="state" label="Etat" type="line" size="20" />
				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>				
	</views>
	
</data>
</nxl>
EOH;
	}

	function data_handlers($fs,$path,$package,$base_data,$table,$application)
	{
		$clsPath  = $path."{$base_data}_requests.inc";
		if(is_file($clsPath))
			return;

		$clsName = "lib_{$package}_records_{$base_data}_requests";

		$s = <<<EOH
require_once(NX_LIB.'framework.inc');
require_once(NX_LIB.'NxData_ArrayRecords.inc');

/* application */
class {$clsName}
{
	var \$ctxt;

	function __construct(\$ctxt,\$instance) {
		\$this->ctxt=\$ctxt;
	}

	function getRecords(\$recDesc,\$errorCB,\$data,\$props) 
	{
		\$searchRecord = \$this->ctxt->getProperty('page.search');

		if(\$searchRecord)
		{
			\$kWords = \$searchRecord->name->db;
		}
		else
			\$kWords = '';


		\$countKey = '';
		\$key = '';

		// gestion de la pagination
		\$pageAlias 		= 'page';
		\$pageOffset 	= isset(\$_GET[\$pageAlias])? \$_GET[\$pageAlias] : 0;
		\$recordsPerPage = 10;
		\$indexFirst 	= \$pageOffset * \$recordsPerPage;

		\$limit = "";
		if( (isset(\$_REQUEST['nxo'])?\$_REQUEST['nxo']:"") != 'xls_export' ) {
			\$limit = "LIMIT \$indexFirst,\$recordsPerPage";
		}
				
		\$query = <<<QUERY
		
		SELECT SQL_CALC_FOUND_ROWS 
			_name 
		FROM 
			_{$table}
		WHERE 
			_name LIKE '%{\$kWords}%'

		\$limit		
QUERY;

		// exec query
		\$errorCB=new NxErrorCB();
		\$db = &\$this->ctxt->getDB(\$errorCB);
		\$res = \$db->queryWithData(\$query);
		if (!\$res) {

		}
		\$records =  new NxData_ArrayRecords(\$res, \$recDesc,'db');
			
		// count records for pagination
		\$queryCount = 'SELECT FOUND_ROWS() as nbrecords';
		\$resCount = \$db->queryWithData(\$queryCount);
		if (\$resCount) {
			\$totalRecords=\$resCount[0]['nbrecords'];
		}
		\$records->setPageCounters(\$totalRecords,\$pageOffset,\$recordsPerPage,\$pageAlias);

		return \$records;
	}

	function countRecords(\$recDesc,\$errorCB)    {}
}

EOH;

		$fs->saveFile($clsPath,"<?php\n{$s}?>");

		return null;
	}

	function tasks_handlers($fs,$path,$package,$application)
	{
		$clsName = "Workflow_Task_{$application}";
		$clsPath  = $path."{$clsName}.inc";

		if(is_file($clsPath))
			return;


		$s = <<<EOH

include_once(NX_LIB.'framework.inc');

class {$clsName}  extends Workflow_Task
{
	var \$params;
	var \$ctxt;

	function __construct(\$ctxt,\$params=null) {
		\$this->ctxt = \$ctxt;
		\$this->params = \$params;
	}

	function run(\$workflow, \$user)
	{
		\$model = \$workflow->model;
		\$this->ctxt->setProperty('workflow',\$workflow);

		// get List DML
        \$data = \$this->getParam('data','dashboard');
		\$dsList = \$model->getDS(\$data);
		if(!\$dsList) 
			\$dsList = \$dataModel->getDS(\$data);

		if(!\$dsList) 
			return false;

		// get Search DML
        \$dsSearch = \$this->getParam('search','search');
		\$ds = \$model->getDS(\$dsSearch);
		if(!\$ds) 
			\$ds=\$dataModel->getDS(\$dsSearch);

		if(\$ds) 
		{
			// load search record
			\$osearch = new PersistentObject();
			\$osearch->load(\$ds,'request');
			\$dataSearch = \$osearch->record;
			\$this->ctxt->setProperty('page.search',\$dataSearch);

			// prepare the filter key (parameters of search used for pagination)
			\$filterKey = \$this->setFilterKey(\$dataSearch);
			\$filterKeyUrl = '&amp;filterKey='.urlencode(\$filterKey);
			\$this->ctxt->setProperty('filterKeyUrl',\$filterKeyUrl);
		}
		else
		{
			\$data=null;
			\$filterKeyUrl = '';
		}

		// buttons
		\$this->ctxt->setProperty('list.url','?nxo='.\$_REQUEST['nxo'].\$filterKeyUrl);

        // set toolbar buttons
        \$props=\$this->setButtons();

		// load record from request in create mode (with clone enabled if using models)
		\$col = new PersistentCollection();
		\$col->load(\$dsList,'handler',null,\$dataSearch);
		if(!\$col->isOk())
			return false;

		// display list
        \$view = \$this->getParam('view','dashboard');
		\$view = \$model->getDisplayView(\$view);
        \$view_type = \$this->getParam('view_type','HTML');
        \$showSearch = \$this->getParam('showSearch','yes');

		\$this->ctxt->runView(\$view,array(
			'page.records' => \$col->records,
			'fmethod' => 'toHTML',
			'view.type' => \$view_type,
			'forceDisplay'=>\$showSearch
			),null);

		return true;
	}

	function setFilterKey(\$searchData)
	{
		\$searchDataVar='searchData';
		\$_searchKey_=((\$_nx_rec2=\${\$searchDataVar})!=null?\$_nx_rec2->getUrlKey(true):'');
		if((isset(\$_REQUEST["page"])?\$_REQUEST["page"]:""))
		{
			\$_searchKey_ = preg_replace("/page=[^&]+(&(amp;)?)?/",'',\$_searchKey_);  
			\$_searchKey_=\$_searchKey_.'&amp;page='.(isset(\$_REQUEST["page"])?\$_REQUEST["page"]:"");
		}

		if((isset(\$_REQUEST["sort_by"])?\$_REQUEST["sort_by"]:""))
		{
			\$_searchKey_ = preg_replace("/sort_by=[^&]+(&(amp;)?)?/",'',\$_searchKey_);  
			\$_searchKey_=\$_searchKey_.'&amp;sort_by='.(isset(\$_REQUEST["sort_by"])?\$_REQUEST["sort_by"]:"");
		}

		if((isset(\$_REQUEST["sort_dir"])?\$_REQUEST["sort_dir"]:""))
		{
			\$_searchKey_ = preg_replace("/sort_dir=[^&]+(&(amp;)?)?/",'',\$_searchKey_);  
			\$_searchKey_=\$_searchKey_.'&amp;sort_dir='.(isset(\$_REQUEST["sort_dir"])?\$_REQUEST["sort_dir"]:"");
		}

		\$this->ctxt->setProperty("filterKey",\$filterKey = \$_searchKey_);

		if(isset(\$_GET['nxs']))
			\$_searchKey_ = 'nxs='.\$_GET['nxs'].'&'.\$_searchKey_;

		if(isset(\$_GET['nxo']))
			\$_searchKey_ = 'nxo='.\$_GET['nxo'].'&'.\$_searchKey_;

		if(isset(\$_GET['wao']))
			\$_searchKey_ = 'wao='.\$_GET['wao'].'&'.\$_searchKey_;

		\$GLOBALS["NX_CURRENT_URL"] ='?'.\$_searchKey_;	
        
		return \$filterKey;
	}

}

EOH;

		$fs->saveFile($clsPath,"<?php\n{$s}?>");

		return null;
	}



	function strings($package,$strings,$string_base)
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

		<i8n:string id="dashboard" value="Tableau de bord - xxx" />
		<i8n:string id="page_title" value="La liste des xxxs" />
		
		<i8n:string id="create record" value="Création xxx" />

		<i8n:string id="list records" value="Liste des xxxs" />
		<i8n:string id="add record" value="Ajout d\'un nouveau xxx" />
		<i8n:string id="edit record" value="Traitement xxx" />
		<i8n:string id="exec action" value="Traitement xxx" />
		<i8n:string id="page_title_view" value="Détails du xxx" />
		
		<i8n:string id="items_search" value="xxx(s) trouvé(s)" />	
		<i8n:string id="edit_helper" value="Editer" />	
		<i8n:string id="delete_helper" value="Supprimer" />	
		<i8n:string id="legend" value="Légende" />

		<i8n:string id="page_message_list" value="Voici la liste des xxxs" />
		<i8n:string id="page_message_add" value="Ce formulaire vous permet d\'ajouter un nouveau xxx" />
		<i8n:string id="page_message_edit" value="Ce formulaire vous permet de modifier le xxx sélectionné" />
		<i8n:string id="page_message_view" value="Détails du xxx sélectionné" />
		
		<i8n:string id="page_message_list_details" value="Cliquer sur un xxx pour en visualiser les détails" />
		<i8n:string id="page_message_add_details" value="Après avoir remplis les champs du formulaire, cliquer sur envoyer pour ajouter le nouveau xxx" />
		<i8n:string id="page_message_edit_details" value="Cliquer sur \'Envoyer\' pour valider les modifications et revenir à la liste des xxx, ou sur \'Retour\' pour annuler les modifications" />
		<i8n:string id="page_message_view_details" value="Cliquer sur \'Retour\' pour revenir à la liste des xxxs" />

		<i8n:string id="dlg_wait_title" value="Veuillez patienter" />
		<i8n:string id="dlg_wait_content" value="En cours d\'ajout des nouvelles données" />

		<i8n:string id="dlg_success_title" value="Action réussie" />
		<i8n:string id="dlg_success_content" value="Opération effectuée avec succès" />

		<i8n:string id="dlg_back_title" value="Annulation de la saisie en cours" />
		<i8n:string id="dlg_back_content" value="Cette action va supprimer les données en cours de saisie. Souhaitez-vous continuer ?" />

		<i8n:string id="dlg_delete_title" value="Suppression d\'un xxx" />
		<i8n:string id="dlg_delete_content" value="Etes vous sûr de vouloir supprimer ce xxx ?" />

		<i8n:string id="dlg_success" value="Valider" />
		<i8n:string id="dlg_cancel" value="Annuler" />
		<i8n:string id="dlg_ok" value="Ok" />

		<i8n:string id="empty_list" value="Aucun xxx n\'est enregistrée" />
		
		<i8n:string id="details" value="Détails" />
		<i8n:string id="edit" value="Modifier" />
		<i8n:string id="delete" value="Supprimer" />

		<i8n:string id="Next" value="Suivant" />
		<i8n:string id="Previous" value="Précédent" />
		<i8n:string id="Results" value="Résultats" />		

		<i8n:string id="search" value="Rechercher" />
		<i8n:string id="add" value="Ajouter" />
		<i8n:string id="export_excel" value="Export Excel" />
		<i8n:string id="save" value="Enregistrer" />
		<i8n:string id="save and continue" value="Enregistrer et continuer" />
		<i8n:string id="back" value="Retour" />
		<i8n:string id="submit" value="Envoyer" />
		<i8n:string id="attached_files_title" value="Fichiers attachés" />
	
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
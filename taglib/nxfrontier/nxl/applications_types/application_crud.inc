<?php

include_once('application_base.inc');

class application_crud extends application_base
{
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
	'record'   => '{$base_data}.record@$package',       // default record form
	'edit_process'   => '{$base_data}.record@$package', // save edited form

	// LIST
	'search'   => '{$base_data}.search@$package', // search form
	'excel'    => '{$base_data}.excel@$package',  // excel list export
	'list'	   => '{$base_data}.list@$package',   // list
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
	'list' => 'workflow.list_page@workflow',
	'record' => 'workflow.xedit_generic_attachments3@workflow',
	'excel' => 'exports.excel.records',
);

// base application
include GENERIS_PACKAGES.'workflow/application/base_application_crud.inc';

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

		<i8n:string id="empty_list" value="Aucun xxx n\'est enregistré" />
		
		<i8n:string id="details" value="Détails" />
		<i8n:string id="edit" value="Modifier" />
		<i8n:string id="delete" value="Supprimer" />

		<i8n:string id="Next" value="Suivant" />
		<i8n:string id="Previous" value="Précédent" />
		<i8n:string id="Results" value="Résultats" />		

		<i8n:string id="Success" value="Opération réussie" />
		<i8n:string id="Error" value="Erreur" />
		<i8n:string id="Warning" value="Attention" />

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
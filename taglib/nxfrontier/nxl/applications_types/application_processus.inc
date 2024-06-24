<?php

include_once('application_base.inc');

class application_processus extends application_base
{
	// model application
	function appDesc($package,$application,$processus,$role,$table,$base_data,$strings,$permissions,$isCreator=true) {
		if($isCreator)
			$listMainActions = <<<EOH
/* list action : no add button for validator */
\$listMainActions = array(
	'export_excel' => array(
		'name'=>('export_excel'),
		'nxo' => 'export_excel',
		'filterKey' => true
	)
);
EOH;
		else
			$listMainActions='';
		
		
		return 				$s=<<<EOH
<?php
// package
\$package = '{$package}';

// prefix for dml views .record, .list, .search.
\$base_data = '{$package}';

// data sources (optional)
\$datasources = array(
	// WORKFLOW
	'header'   => '{$base_data}.header@$package', // state header

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

	// TABS
	'list_by_states'	   => '{$base_data}.list_by_states@$package',   // list tabs

	'log.record' => '{$table}_log.record@investment_request',
	'log.list_by_item' => '{$table}_log.list_by_item@investment_request',

);
// set property for use by the workflow model
\$this->ctxt->setProperty('workflow.data',\$datasources);

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
	'print'  => 'workflow.xedit_generic_attachments3@workflow',
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

\$title = 'list records';
\$workflow_id = '{$processus}';
\$workflow_role = '{$role}';

{$listMainActions}

// base application
include GENERIS_PACKAGES.'workflow/application/base_application_workflow.inc';

EOH;
	}

	// ================ model data ========================

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

		<table name="{$table}_log">
			
			<key mode="record">
				<field name="oid" type="oid" value="%{property:oid}"/>
			</key>
			
			<key mode="list_by_item">
				<field name="item_oid" type="oid" value="%{property:item_oid}"/>
			</key>

			<fields>
				<!-- Identifiant de la mission -->
				<field name="oid" label="object_id" type="oid" isKey="primary" show="hidden" isEdit="no"/>

				<!-- Identifiant de la demande -->
				<field name="item_oid" label="item_oid" type="ref-oid" isKey="secondary" show="hidden"/>

				<!-- Identifiant du validateur -->
				<field name="user_oid"  label="user_oid" type="text-choice" 
					list="users_all" prompt="Collaborateur..." multiple="no" control="select"
					ds="db:user.all@admin2" ds_format="{field:first_name} {field:last_name}" 
					ds_field_value="oid"
					show="show" isEdit="no"/>

				<!-- Etat de la demande -->
				<field name="state" label="demand_state_log" type="integer"/>

				<!-- action -->
				<field name="action" label="action" type="line"/>

				<field name="date" label="date" type="timestamp" default="now" />

			</fields>
		</table>
	</tables>
		
	<!-- DML Views (gernerating SQL requests)-->
	<views>
		<!-- list of items -->
		<view name="{$base_data}.list" table="{$table}">
			<key>
				<field name="name" label="name"  type="line" size="200" relation="like"/>
				<field name="state" type="line" relation="in" value="%{property:workflow.state_list}" />			
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

		<!-- list of item states created by current user -->
		<view name="{$base_data}.list_creator_by_states" table="{$table}">
			<key>
				<field name="state" type="line" relation="in" value="%{property:workflow.state_list}" />			
			</key>

			<group_by>
				<field name="state"  type="line" size="20" default="0" isEdit="no"/>			
			</group_by>

			<fields>
				<field name="oid" label="object id" type="oid" isKey="primary" show="hidden" isEdit="no"/>
				<field name="state"  type="line" size="20" default="0" isEdit="no"/>
				<field name="count" type="ref-oid" sql="count(*)" />
			</fields>
		</view>

		<!-- list of items assigned to current user -->
		<view name="{$base_data}.list_by_states" table="{$table}">
			<key>
				<field name="empid" type="integer" value="%{user:oid}"/>
				<field name="state" type="line" relation="in" value="%{property:workflow.state_list}" />			
			</key>

			<group_by>
				<field name="state"  type="line" size="20" default="0" isEdit="no"/>			
			</group_by>

			<fields>
				<field name="oid" label="object id" type="oid" isKey="primary" show="hidden" isEdit="no"/>
				<field name="state"  type="line" size="20" default="0" isEdit="no"/>
				<field name="count" type="ref-oid" sql="count(*)" />
			</fields>
		</view>

		<!-- export excel -->
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

		<!-- header for preloading record and check its state -->
		<view name="{$base_data}.header" table="{$table}">
			<key>
				<field name="oid"  value="%{request:oid}"/>
			</key>

			<fields>
				<field name="oid" label="oid" type="oid" isKey="primary"/>
				<field name="state" label="Etat" type="line" size="20" />
			</fields>		
		</view>				

		<!-- form for adding a new record-->
		<view name="{$base_data}.add" table="{$table}">
			<key>
				<field name="oid"  value="%{request:oid}"/>
			</key>

			<fields>
				<field name="oid" label="oid" type="oid" isKey="primary"/>
				<field name="state" label="Etat" type="line" size="20" />
				<field name="name" label="name"  type="line" size="200" />
			</fields>		
		</view>				

		<view name="{$base_data}.add_process" table="{$table}">
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

	function data_handlers($package,$base_data,$table)
	{
		return null;
	}

	function tasks_handlers($fs,$path,$package,$application)
	{
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
	
		<!-- LOG -->
		<i8n:string id="action" value="Action exécutée" />
		<i8n:string id="by" value="Acteur du workflow" />
		<i8n:string id="at" value="Date/horaire" />
		<i8n:string id="no action" value="La demande n'a pas été encore été envoyée" />

		<!-- les labels des champs de données -->
		<fields xmlns="http://www.nxfrontier.com/nxl/i8n">

			<field name="name" label="Nom"  />
			<field name="state" label="Etape processus" type="ref-oid" isKey="secondary" />

		</fields>
			
	</i8n:strings>
	
</nxl>
EOH;

		$s = $this->remapStrings($s,$string_base);
		return $s;
	}

}

?>
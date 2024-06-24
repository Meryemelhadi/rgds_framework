<?php 

require_once(NX_XLIB.'lib/framework.inc');

if (!defined('HR_RULE_SET'))
	define('HR_RULE_SET','maroc');

include_once(NX_XLIB.'lib/rh/conge/'.HR_RULE_SET.'/demand.inc');

/**
 *	Classe de validation de l'application 
 *
 *	@author 
 *	@version 1.O
 *
 */
class lib_rh_conge_services_my_demands
{
	var $ctxt;

	function lib_rh_conge_services_my_demands($ctxt)
	{
		$this->ctxt = $ctxt;
		Framework::setContext($this->ctxt = $ctxt);
	}

	/**
	 *	Fonction de validation lors de l'ajout d'une demande
	 *
	 *	@return : true dans le cas ou la validation passe bien / Identifiant du message d'erreur en cas d'erreur	 
	 */
	function submitDemand($userOid,$demandRecord)
	{
		// $uoid= Framework::getUser()->getProperty('oid',0);
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doSubmit($user))
		{
			return true;
		}
		else
		{
			return $demand->checkRules($user);
		}
	}

	/**
	 *	Fonction de modification d'une demande
	 *
	 *	@return : true dans le cas ou la validation passe bien / Identifiant du message d'erreur en cas d'erreur	 
	 */
	function editDemand($userOid,$demandRecord)
	{
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doModify())
		{
			return true;
		}
		else
		{
			return $demand->checkRules($user);
		}
	}

	// return 1 if user cant modify validated demand, 6 if can modify
	function getStateModifyLimit($userOid)
	{
		$demand = rh_conge_demandPeer::getEmpty();
		return $demand->getStateModifyLimit();
	}

	function acceptDemand($userOid,$demandRecord)
	{
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doSubmit($user,false))
		{
			return true;
		}
		else
		{
			return $demand->checkRules($user);
		}
	}

	function denyDemand($userOid,$demandRecord)
	{
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doCancel($user))
		{
			return true;
		}
		return true;
	}


	/**
	 *	Fonction de validation lors de la modification d'une demande
	 *
	 *	@return : true dans le cas ou la validation passe bien / Identifiant du message d'erreur en cas d'erreur	 
	 */
	function deleteDemand($userOid,$demandRecord)
	{
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doCancel($user))
		{
			return true;
		}

		return true;
	}

	function getActions($userOid,$validatorOid,$demandRecord,&$canEdit,&$canValidate)
	{
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		$actions = $demand->getActions($userOid,$validatorOid);

		$canEdit = false;
		$canValidate= false;

		// it is possible to change the demand if not yet started
		if ($demand->canModify())
		{
			if (isset($actions['modify']))
			{
				$canEdit = true;
			}
			if (isset($actions['accept']))
			{
				$canValidate = true;
			}
		}

	}

	function getViewActions($userOid,$validatorOid,$demandRecord,$params='')
	{
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		$actions = $demand->getActions($userOid,$validatorOid);
		$props=array();

		$urls = array(
			'list' => array( 'url' => 'nxo=list&nxs='),
		);

		if (isset($actions['modify']))
		{
			$props['edit.url']='&nxo=edit_record'.$params;
			$props['delete.url']='&nxo=delete_record'.$params;
		}
		if (isset($actions['accept']))
		{
			$props['accept.url']='&nxo=process_accept'.$params;
			$props['refuse.url']='&nxo=process_deny'.$params;
		}

		$this->ctxt->addProperties($props);
	}

	
	function getValidationPath($demandRecord,$userOid=null)
	{
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);

		$uoid = $demand->empid->object;
		if ($uoid==null)
			$uoid = $userOid;
		$user = rh_userPeer::getByOid($uoid);

		$validatorName=$validationGroup='';
		$demand->getUserValidatorInfos($user,$validators,$validationGroup);
		$s='validators : '.rh_groupPeer::getByOid($demand->getUserGroup($user)->object);
		$path=array();
		$path[] = "{$user}";
		while($validationGroup)
		{
			$group=rh_groupPeer::getByOid($validationGroup);
			$path[] = "<span>{$validators} ({$group})</span>";
			$parentGroup=rh_groupPeer::getByOid($demand->getGroupParent($group)->object);
			$demand->getGroupValidatorInfos($user,$parentGroup,$validators,$validationGroup);
		}
		$path[] = "Traitement";

		$s = ''.implode(' =&gt; ',$path);
		return $s;
	}

	function AJAX_getReliquat()
	{
		$uoid= Framework::getUser()->getProperty('oid',0);
		$user = rh_userPeer::getByOid($uoid);

		$demand = rh_conge_demandPeer::getFromRequest();

		// check rules
		$res=$demand->checkRules($user);
		if ($res!==true)
		{
			$returnvalues['type']='reliquat';
			$returnvalues['status']='error';
			$returnvalues['demandedDays']=-1;
			$returnvalues['message']=$res;
			$script['actions'][] = $returnvalues;
		}
		else
		{
			// rules is ok, display infos
			$returnvalues=$demand->getInfos($user);
			$returnvalues['type']='reliquat';
			$script['actions'][] = $returnvalues;
		}

		$action->type = 'alert';
		$action->message = 'welcome to Generis';
		$script['actions'][] = $action;

		include(NX_LIB.'json/json'.NX_PHP_VER.'.inc');
//			return 's='.json_encode($script).';MAJ_TTValue("xxxx",$("observations-control"),"",s["status"],s);';
		return json_encode($script);
	}	
}

?>
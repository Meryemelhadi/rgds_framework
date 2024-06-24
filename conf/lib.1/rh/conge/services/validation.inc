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
class lib_rh_conge_services_validation
{
	var $ctxt;

	function __construct($ctxt)
	{
		$this->ctxt = $ctxt;
		Framework::setContext($this->ctxt = $ctxt);
	}

	/**
	 *	Fonction de validation lors de l'ajout d'une demande
	 *
	 *	@return : true dans le cas ou la validation passe bien / Identifiant du message d'erreur en cas d'erreur	 
	 */
	function addDemand($demandRecord)
	{
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		$userOid = $demand->empid->object;
		$user = rh_userPeer::getByOid($userOid);

		$validatorOid = Framework::getUser()->getProperty('oid',0);
		$validator = rh_userPeer::getByOid($validatorOid);

		if ($demand->doSubmit($user,true)===false)
		{
			return $demand->checkRules($user,true);
		}

		/*
		if ($demand->doAccept($validator)===false)
		{
			return $demand->checkRules($user);
		}
		*/

		return true;
	}

	/**
	 *	Fonction de validation lors de la modification d'une demande
	 *
	 *	@return : true dans le cas ou la validation passe bien / Identifiant du message d'erreur en cas d'erreur	 
	 */
	function editDemand($userOid,$demandRecord)
	{
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($userOid != $demand->empid->object)
			// with validator
			$res = $demand->doModify($user);
		else
			$res = $demand->doModify();

		if ($res)
		{
			return true;
		}
		else
		{
			return $demand->checkRules($user,false);
		}
	}

	function acceptDemand($userOid,$demandRecord)
	{
		$validator = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doAccept($validator,true))
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
		$validator = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doDeny($validator))
		{
			return true;
		}
		return true;
	}


	/**
	 *	Fonction de destruction de la demande
	 *
	 *	@return : true dans le cas ou la validation passe bien / Identifiant du message d'erreur en cas d'erreur	 
	 */
	function deleteDemand($userOid,$demandRecord)
	{
		$user = rh_userPeer::getByOid($userOid);
		$demand = rh_conge_demandPeer::getFromRecord($demandRecord);
		if ($demand->doDelete($user))
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
	
	function getValidationPath($userOid)
	{
		$demand = rh_conge_demandPeer::getFromRequest();
		$user = rh_userPeer::getByOid($userOid);

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
		$demand = rh_conge_demandPeer::getFromRequest();

		//$uoid= Framework::getUser()->getProperty('oid',0);
		$uoid = $demand->empid->object;
		$user = rh_userPeer::getByOid($uoid);

		// check rules
		$res=$demand->checkRules($user,true);
		if ($res!==true)
		{
			$returnvalues=$demand->getInfos($user);
			$returnvalues['empid']="$uoid";
			$returnvalues['status']='error';
			$returnvalues['demandedDays']=-1;
			$returnvalues['message']=$res;
		}
		else
		{
			// rules is ok, display infos
			$returnvalues=$demand->getInfos($user);
		}

		if (isset($returnvalues['demandedDays']))
		{
			include(NX_LIB.'json/json'.NX_PHP_VER.'.inc');
			return json_encode($returnvalues);
		}
	}	
}

?>
<?php

include_once NX_XLIB."lib/rh/user.inc";
include_once(NX_XLIB.'lib/rh/hrtime.class.inc');

class workflow_demand extends PersistentObject
{
	function setState($state) {
		$this->demand_state->object=$state;
	}
	function getState() {
		return $this->demand_state->object;
	}

	function loadConfig() {
		return false;
	}
}

class rh_demand extends workflow_demand
{
	var $lang;
	var $ruleset;

	const  DEMAND_INIT=0;
	const  DEMAND_PENDING=1;
	const  DEMAND_MODIFY=2;
	const  DEMAND_VALIDATED=3;
	const  DEMAND_DENIED=4;
	const  DEMAND_PROCESSED=5;
	const  DEMAND_CANCEL=6;
	const  DEMAND_DELETED=7;
	const  DEMAND_FORCE_EDIT=8;

	function __construct($ruleset='default') {
		$this->ruleset=$ruleset;
	}

	function getStateModifyLimit()
	{
		return self::DEMAND_PROCESSED;
	}

	// workflow configuration
	function getDemandValidatorGroup() {
		return $this->group_oid;
	}
	function getDemandValidators() {
		return null;
	}

	// user field : main group
	function getUserGroup($user) {
		return $user->group;
	}
	// user field : hr validator
	function getUserValidators($user) {
		return $user->hr_manager;
	}
	// user field : hr group
	function getUserValidatorGroup($user) {
		return $user->mgc_parent_group_RH;
	}
	// group field : hr parent group
	function getGroupParent($group) {
		return $group->mgc_parent_group_RH;
	}
	// group field : hr group validators
	function getGroupValidators($group) {
		return $group->hr_manager;
	}

	// check if demand rules validates
	function isValid($demand,$user)
	{
		return $demand->checkRules($user);
	}

	// get extra information
	function getCheckInfos($demand,$user)
	{
		return $demand->getInfos($user);
	}

	// get user validator from his record and his group
	function getUserValidatorInfos($user,&$userValidators,&$validationGroupOid)
	{
		$userOid = $user->oid->object;

		// get user validation group from user record (overloads his main group)
		$userValidatorGroupOid = $this->getUserValidatorGroup($user)->object;
		if ($userValidatorGroupOid!='' && $userValidatorGroupOid!='?')
		{
			// use RH group in user record
			$fGroupOid = $userValidatorGroupOid;
		}
		else
		{
			// no validator group in user object => use main group
			$fGroupOid = $this->getUserGroup($user)->object;
		}

		// get group validator
		$group = rh_groupPeer::getByOid($fGroupOid);
		$groupValidatorsOid='';
		if (!$group->isOk() || 
			!$this->getGroupValidatorInfos($user,$group,$groupValidators,$validationGroupOid))
		{
			// if no validator group => auto validator
			return false;
		}

		// get validators from the user record, or group validator
		$userValidators = $this->getUserValidators($user);
		if ($userValidators->string=='')
			$userValidators=$groupValidators;

		return true;
	}

	// get group validator info (from current group or its parents if current one has no validator)
	function getGroupValidatorInfos($user,$group,&$groupValidators,&$validationGroupOid)
	{
		$userOid=$user->oid->object;

		// keep all groups in the path so that we dont loop forever..
		$groupsArray = array();
		
		while($group->isOk() && ($fGroupOid=$group->oid->object))
		{
			$groupsArray[] = $fGroupOid;

			// get validators for the current group
			$fvalidators=$this->getGroupValidators($group);
			$validatorsOid=$fvalidators->object;

			// get parent group 
			$parentGroupOid= $this->getGroupParent($group)->object;

			// stop if validators are found and are not the user himself
			if(	trim($validatorsOid) != '' && !in_array($userOid, explode('|', trim($validatorsOid))))
			{
				// finished searching : we have validators (who isnt the user himself)
				// get validators from the user record, or group validator
				$groupValidators=$fvalidators;
				$validationGroupOid=$fGroupOid;	
				return true;
			}
			elseif(trim($parentGroupOid == '') || trim($parentGroupOid == '?') || in_array($parentGroupOid, $groupsArray))
			{
				// no parent group or group loop
				$validationGroupOid='';
				return false;
			}
			else
			{
				// loop: search validator in parent group..
				$group = rh_groupPeer::getByOid($parentGroupOid);
			}
		}

		$validationGroupOid='';
		return false;
	}

	// get group validator info from the demand object itself
	function getCurrentGroupValidatorInfos($user,&$validators,&$validationGroup) {
		$validators=$validationGroup='';
		$this->getUserValidatorInfos($user,$validators,$validationGroup);
		$currentValidatorGroupOid=$this->getDemandValidatorGroup()->object;
		if ($validationGroup != $currentValidatorGroupOid)
		{
			// this is not the first validator group => get current group validators
			$currentGroup = rh_groupPeer::getByOid($currentValidatorGroupOid);
			$this->getGroupValidatorInfos($user,$currentGroup,$validators,$validationGroup);
		}
		return true;
	}

	// get parent group info in the workflow
	function moveToParentGroup($user,$validatorsOid,$validationGroupOid) {
		$demandGroup = rh_groupPeer::getByOid($this->getDemandValidatorGroup()->object);
		$parentGroup = rh_groupPeer::getByOid($this->getGroupParent($demandGroup)->object);
		$this->getGroupValidatorInfos($user,$parentGroup,$validators,$validationGroupOid);
		$this->getDemandValidatorGroup()->object = $validationGroupOid;

		return $validationGroupOid;
	}

	//================ object store/delete ============

	function delete($ds=null,$media='db',$params=null)
	{
		if (!$this->isOk())
			return false;

		if (!$ds) {
			$ds=$this->ds_demand;
		}
		if (!$params) {
			$params= array('oid'=>$this->oid->object);
		}

		rh_validation_logPeer::deleteByDemand($this->oid->object);
		
		return parent::delete($ds,$media,$params);
	}
	function store($ds,$op='insert',$media='db',$params=null)
	{
		if (!$this->isOk())
			return false;

		if (!$ds) {
			if ($op=='insert')
				$ds=$this->ds_demand_new;
			else
				$ds=$this->ds_demand;
		}
		if (!$params) {
			$params= array('oid'=>$this->oid->object);
		}
		
		$res=parent::store($ds,$op,$media,$params);

		// get oid if demand created
		if ($op=='insert')
			$this->oid->object=$this->getLastInsertId($media);

		return $res;
	}

	// workflow actions
	function getActions($demandOid,$userOid=null,$validatorOid=null) {
		$oid = $demandOid;

		$lang = array(
			'demand'=>array(
				'submit'=>'Submit',
				'accept'=>'Accept',
				'process'=>'Process',
				'modify'=>'Modify',
				'cancel'=>'Cancel',
				'deny'=>'Deny',
				'force_edit'=>'Force Edit'
			)
		);

		if (!$this->isOk() || $this->oid===null)
			return array();
		
		$oid=$this->oid->object;

		switch($this->demand_state->object)
		{
			case self::DEMAND_MODIFY:
				return 
					array(
						'submit' => array(
							'url'   => "{$url}&amp;demand_id=$oid",
							'label' => $lang['demand']['submit']
							),
						'cancel' => array(
							'url'   => "{$url}&amp;demand_id=$oid",
							'label' => $lang['demand']['cancel']
							)
					);
			case self::DEMAND_PENDING:
				return 
					array(
						'accept' => array(
							'url'   => "{$url}&amp;demand_id=$oid",
							'label' => $lang['demand']['accept']
							),
						'modify' => array(
							'url'   => "{$url}&amp;demand_id=$oid",
							'label' => $lang['demand']['modify']
							),
						'cancel' => array(
							'url'   => "{$url}&amp;demand_id=$oid",
							'label' => $lang['demand']['cancel']
							),
						'deny' => array(
							'url'   => "{$url}&amp;demand_id=$oid",
							'label' => $lang['demand']['deny']
							)
					);
			case self::DEMAND_VALIDATED:
			case self::DEMAND_PROCESSED:
			case self::DEMAND_FORCE_EDIT:
				return 
					array(
						'process' => array(
							'url'   => "{$url}&amp;demand_id='.$oid.'&action=process",
							'label' => $lang['demand']['process']
							),
						'cancel' => array(
							'url'   => "{$url}&amp;demand_id='.$oid.'&action=cancel",
							'label' => $lang['demand']['cancel']
							),
						'modify' => array(
							'url'   => "{$url}&amp;demand_id='.$oid.'&action=modify",
							'label' => $lang['demand']['modify']
							),
						'force_edit' => array(
							'url'   => "{$url}&amp;demand_id='.$oid.'&action=force_edit",
							'label' => $lang['demand']['force_edit']
							)
					);
				break;
			default:
				return array();
		}
	}


	// ============ WORKFLOW TRANSITIONS

	// submit demand
	function doSubmit($user,$force=false)
	{
		if ($this->checkRules($user,$force)!==true)
			return false;

		// get users validators and group
		$userValidatorsOid=$validationGroupOid=null;
		$this->getUserValidatorInfos($user, $userValidatorsOid,$validationGroupOid);

		switch($this->demand_state->object)
		{
			case self::DEMAND_MODIFY:
				if ($userValidatorsOid=='')
				{
					// no validator => auto validate => validated state
					$this->setState(self::DEMAND_VALIDATED);
					$this->store($this->ds_demand,'update','db');
					$this->log($user);
					$this->sendMessageToService($user);
				}
				else
				{
					// pending request
					$this->setState(self::DEMAND_PENDING);
					$this->getDemandValidatorGroup()->object = $validationGroupOid;
					$this->store($this->ds_demand,'update','db');
					$this->log($user);
					$this->sendMessageToNextValidators($user,$userValidatorsOid);
				}
				break;
			case self::DEMAND_PENDING:
			case self::DEMAND_INIT:
				$this->empid->object=$user->oid->object;
				if ($userValidatorsOid=='')
				{
					// no validator => auto validate => validated state
					$this->setState(self::DEMAND_VALIDATED);
					$this->store($this->ds_demand_new,'insert','db');
					$this->log($user);
					$this->sendMessageToService($user);
				}
				else
				{
					// pending request
					$this->setState(self::DEMAND_PENDING);
					$this->getDemandValidatorGroup()->object = $validationGroupOid;
					$this->store($this->ds_demand_new,'insert','db');
					$this->log($user,null,self::DEMAND_INIT);
					$this->sendMessageToNextValidators($user,$userValidatorsOid);
				}
				break;
		}

		$this->sendMessageToUser($user);
		return true;
	}

	// cancel demand by user or validator or HR
	function doCancel($validator=null)
	{
		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		switch($this->demand_state->object)
		{
			case self::DEMAND_PENDING:
			case self::DEMAND_MODIFY:
				$this->setState(self::DEMAND_CANCEL);
				$this->log($user,$validator);
				$this->store($this->ds_demand,'update','db');
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			case self::DEMAND_VALIDATED:
			case self::DEMAND_PROCESSED:
			case self::DEMAND_FORCE_EDIT:
				$this->setState(self::DEMAND_CANCEL);
				$this->log($user,$validator);
				$this->store($this->ds_demand,'update','db');

				// put back days to user
				$this->getInfos($user);	
				$this->resetUser($user,$this->infos);

				$this->sendMessageToValidator($user,$validator);
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			default:
				throw new Exception('invalid workflow state');
		}
		return true;
	}

	function doAccept($validator,$force=false)
	{
		// check validator ?

		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		$this->getInfos($user);		
		if ($this->checkRules($user,$force)===true)
		{
			$days_to_take=$this->infos['demandedDays'];
		}
		else
//			return $this->doDeny($user,$validator);
			return false;

		switch($this->demand_state->object)
		{
			case self::DEMAND_PENDING:
				$validatorsOid=$validationGroupOid=null;
				$validationGroupOid = $this->moveToParentGroup($user,$validatorsOid,$validationGroupOid);
				if ($validationGroupOid=='')
				{
					$this->setState(self::DEMAND_VALIDATED);
					$this->days_to_take->object = $days_to_take;
					$this->store($this->ds_demand,'update','db');
					$this->log($user,$validator);

					// remove reliquat from user
					$this->applyToUser($user,$this->infos);
					
					$this->sendMessageToService($user);
					$this->sendMessageToUser($user);
				}
				else 
				{
					$this->store($this->ds_demand,'update','db');
					$this->log($user,$validator,self::DEMAND_VALIDATED);
					$this->sendMessageToNextValidators($user,$validatorsOid);
				}
				
				break;
			default:
				throw new Exception('invalid workflow state');
		}
		return true;
	}

	function doDeny($validator=null)
	{
		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		switch($this->demand_state->object)
		{
			case self::DEMAND_PENDING:
			case self::DEMAND_MODIFY:
				$this->setState(self::DEMAND_DENIED);
				$this->store($this->ds_demand,'update','db');

				$this->log($user,$validator);
				
				$this->sendMessageToValidator($validator,$user);
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			case self::DEMAND_VALIDATED:
			case self::DEMAND_PROCESSED:
			case self::DEMAND_FORCE_EDIT:
				$this->setState(self::DEMAND_DENIED);
				$this->store($this->ds_demand,'update','db');

				$this->log($user,$validator);

				// put back days to user
				$this->getInfos($user);		
				$this->resetUser($user,$this->infos);

				$this->sendMessageToValidator($validator,$user);
				$this->sendMessageToService($demand,$user);
				$this->sendMessageToUser($user);
				break;
			default:
				throw new Exception('invalid workflow state');
		}
		return true;
	}

	function doModify($validator=null)
	{
		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		$validatorsOid=$validationGroupOid=null;
		$this->getUserValidatorInfos($user, $validatorsOid,$validationGroupOid);

		switch($this->demand_state->object)
		{
			case self::DEMAND_PENDING:
			case self::DEMAND_MODIFY:
			case self::DEMAND_DENIED:
				$this->setState(self::DEMAND_MODIFY);
				// set it back to first group..
				$this->getDemandValidatorGroup()->object = $validationGroupOid;
//				$this->store($this->ds_demand,'update','db');
				$this->log($user,$validator);
				if ($validator===null)
				{
					// self modification
					$this->setState(self::DEMAND_PENDING);
					$this->sendMessageToNextValidators($user,$validatorsOid);
				}

				$this->store($this->ds_demand,'update','db');

				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);

				break;
			case self::DEMAND_PROCESSED:
			case self::DEMAND_VALIDATED:
			case self::DEMAND_FORCE_EDIT:
				if ($validator===null)
				{
					// modified by employee

					// cancel demand
					$this->setState(self::DEMAND_CANCEL);
					$this->store($this->ds_demand,'update','db');
					$this->log($user);

					// put back days to user
					$this->getInfos($user);		
					$this->resetUser($user,$this->infos);

					// submit it again..
					if ($userValidatorsOid=='')
					{
						// no validator => auto validate => validated state
						$this->setState(self::DEMAND_VALIDATED);
						$this->store($this->ds_demand,'update','db');
						$this->log($user);
						$this->sendMessageToService($user);
					}
					else
					{
						// pending request
						$this->setState(self::DEMAND_PENDING);
						$this->getDemandValidatorGroup()->object = $validationGroupOid;
						$this->store($this->ds_demand,'update','db');
						$this->log($user);
						$this->sendMessageToNextValidators($user,$userValidatorsOid);
					}
				}
				else
				{
					// modified by validator
					$this->setState(self::DEMAND_CANCEL);
					$this->store($this->ds_demand,'update','db');
					$this->log($user,$validator);

					// put back days to user
					$this->getInfos($user);		
					$this->resetUser($user,$this->infos);
					
					$this->sendMessageToService($user);
					$this->sendMessageToUser($user);
					$this->setState(self::DEMAND_MODIFY);

					// set it back to first group..
					$this->getDemandValidatorGroup()->object = $validationGroupOid;
					$this->store($this->ds_demand,'update','db');
					$this->log($user);
					
					$this->sendMessageToNextValidators($user,$validatorsOid);
					$this->sendMessageToService($user);
					$this->sendMessageToUser($user);
				}
				break;
			default:
				throw new Exception('invalid workflow state');
		}
		return true;
	}

	// force the edit of a demand even if already processed
	function doForceEdit($validator=null)
	{
		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		$validatorsOid=$validationGroupOid=null;
		$this->getUserValidatorInfos($user, $validatorsOid,$validationGroupOid);

		switch($this->demand_state->object)
		{
			case self::DEMAND_PENDING:
			case self::DEMAND_MODIFY:
				$this->setState(self::DEMAND_MODIFY);
				// set it back to first group..
				$this->getDemandValidatorGroup()->object = $validationGroupOid;
//				$this->store($this->ds_demand,'update','db');
				$this->log($user,$validator);
				if ($validator===null)
				{
					// self modification
					$this->setState(self::DEMAND_PENDING);
					$this->sendMessageToNextValidators($user,$validatorsOid);
				}

				$this->store($this->ds_demand,'update','db');

				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);

				break;
			case self::DEMAND_VALIDATED:
				$this->setState(self::DEMAND_CANCEL);
				$this->store($this->ds_demand,'update','db');
				$this->log($user,$validator);

				// put back days to user
				$this->getInfos($user);		
				$this->resetUser($user,$this->infos);
				
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);

				// set it back to first group..
				$this->setState(self::DEMAND_MODIFY);
				$this->getDemandValidatorGroup()->object = $validationGroupOid;
				$this->store($this->ds_demand,'update','db');
				$this->log($user);
				
				$this->sendMessageToNextValidators($user,$validatorsOid);
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			case self::DEMAND_PROCESSED:
			case self::DEMAND_FORCE_EDIT:
				$this->setState(self::DEMAND_CANCEL);
				$this->store($this->ds_demand,'update','db');
				$this->log($user,$validator);

				// put back days to user
				$this->getInfos($user);		
				$this->resetUser($user,$this->infos);
				
				// set it back to validated so that it can be processed by HR team..
				$this->setState(self::DEMAND_FORCE_EDIT);
				$this->store($this->ds_demand,'update','db');
				$this->log($user);
				
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			default:
				throw new Exception('invalid workflow state');
		}
		return true;
	}

	function doDelete($validator=null)
	{
		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		$userValidators=$validationGroupOid=null;
		$this->getUserValidatorInfos($user, $userValidators,$validationGroupOid);

		switch($this->demand_state->object)
		{
			case self::DEMAND_PENDING:
			case self::DEMAND_MODIFY:
				$this->setState(self::DEMAND_DELETED);
				$this->store($this->ds_demand,'update','db');
				$this->log($user,$validator);
				
				$this->sendMessageToNextValidators($user,$validatorsOid);
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			case self::DEMAND_PROCESSED:
			case self::DEMAND_VALIDATED:
			case self::DEMAND_FORCE_EDIT:
				$this->setState(self::DEMAND_DELETED);
				$days_to_take = $this->days_to_take->object;
				$this->store($this->ds_demand,'update','db');
				$this->log($user,$validator);

				// put back days to user
				$this->resetUser($user,$this->infos);
				
				$this->sendMessageToNextValidators($user,$validatorsOid);
				$this->sendMessageToService($user);
				$this->sendMessageToUser($user);
				break;
			default:
				throw new Exception('invalid workflow state');
		}
		return true;
	}

	function doProcessService($processor=null)
	{
		$user=rh_userPeer::getByOid($this->empid->object,'rh.user.conge');

		$this->setState(self::DEMAND_PROCESSED);
		$this->store($this->ds_demand,'update','db');
		$this->log($user,$processor);
		$this->sendMessageToUser($user,$processor);
		return true;
	}

	// ============ END OF WORKFLOW TRANSITIONS
	
	// ================= Logging of actions on that demand into attached table
	function log($user,$validator=null,$state=null)
	{
		$entry=rh_validation_logPeer::getEmpty();
		$entry->demand_id->object=$this->oid->object;
		if ($validator!==null)
			$entry->validator_oid->object=$validator->oid->object;
		else
			$entry->validator_oid->object=$user->oid->object;

		if ($state===null)
			$entry->demand_state_log->object=$this->getState();
		else
			$entry->demand_state_log->object=$state;
		$entry->add();
	}

	// ================= Messaging to the user and to others

	function sendMessageToUser($user,$validator=null)
	{
		$message = messagePeer::getEmpty();
		$message->from->object = $user->oid;
		$message->to->object = $user->oid;
		$message->subject->object = "Intranet &gt; {$this->type} pour {$user} : {$this->demand_state->html}";
		$message->body->object = '';

		$ctxt = Framework::getContext();
		$html='';
		$message->html='';
		$message->setHTML(
			$this->emailUserView,
			array(
				'user'=>$user,
				'demand'=>$this,
				'validator'=>$validator,
				'page.record'=>$this->record
			) 
		); 

		Messenger::send($message);
	}

	function sendMessageToNextValidators($user,$validatorsOid)
	{
		if (!isset($validatorsOid))
			return;

		// get validators by ID list
		$validators = rh_userPeer::getByOidList($validatorsOid->object);

		foreach($validators as $validator)
		{
			$this->sendMessageToValidator($user,$validator);
		}
	}
	
	function sendMessageToValidator($user,$validator)
	{
		$message = messagePeer::getEmpty();
		$message->from->object = $user->oid;
		$message->to->object = $validator->oid;
		$message->subject->object = "Intranet &gt; {$this->type} pour {$user} : {$this->demand_state->html}";
		$message->body->object = '';

		$ctxt = Framework::getContext();
		$html='';
		$message->html='';
		$message->setHTML(
			$this->emailUserView,
			array(
				'user'=>$user,
				'demand'=>$this,
				'validator'=>$validator,
				'page.record'=>$this->record
			) 
		); 

		Messenger::send($message);
	}

	function sendMessageToService($user)
	{
	}

	// ================ BUSINESS RULES ==============

	// check if demand can be modified or cancelled
	function canModify($self=true)
	{
		if ($this->demand_state->object > self::DEMAND_PROCESSED)
			return false;

		if ($self)
		{
			// user can modify if date <= now
			$now = strtotime(date("m/d/y",time()));
			if($this->start_date->time <= $now)
				return false;
		}

		return true;
	}

	// v?rifies les regles de coherences pour le cong?s
	function checkRules($user,$force=false) 
	{
		return true;
	}

	var $infos=null;
	function resetInfos() {
		$this->infos = null;
	}

	// get informations (counters) for that demand
	function getInfos($user)
	{
		if ($this->infos !== null)
				return $this->infos;
		$this->infos = array(
					"status"		=> 'ok'
		);
		return $this->infos;
	}

	// apply demand to user counters
	function applyToUser($user,$infos) {
	}
	
	// reset user counters (undo "apply")
	function resetUser($user,$infos,$days_to_take=null) {
	}

	// ===================== for tests
	function __toString() {
		try {
			if (!$this->isOk())
				return 'empty demand';

			$user = rh_userPeer::getByOid($this->empid->object);
			if (!$user->isOk())
				$user='';

			$s="Demand {$this->ruleset} (user:{$user} state:{$this->demand_state->string}<br/>";
			$infos=$this->getInfos($user);
			foreach ($infos as $k=>$v)
				$s.=(", $k:$v");

			$fgroup=$this->getDemandValidatorGroup();
			if ($fgroup->object)
			{
				$group = rh_groupPeer::getByOid($fgroup->object);
				$s .= "<br/>group validator:{$group}";
			}

			return $s;
		} catch (Exception $e) { return $e; }
	}

}

class rh_conge_demand extends rh_demand
{
	var $ds_demand = 'rh.conge.demand@rh';
	var $ds_demand_new = 'rh.conge.demand.new@rh';

	var $ds_by_date = 'rh.conge.demand.by_date@rh';
	var $ds_by_oid = 'rh.conge.demand@rh';

	var $ds_user = 'rh.user.conge@rh';
	var $ds_group = 'rh.group.conge@rh';
	var $emailUserView = 'rh.conge.views.acceptation_mail_orm';

	var $type = 'demande de cong&eacute;';

	var $params;

	function __construct($ruleset='default') {
		parent::__construct($ruleset);

		$this->params=array(
			'nbDaysperWeek'=>defined(RH_NB_DAYS_PER_WEEK)?RH_NB_DAYS_PER_WEEK:6,
			'canLeaveSameDay'=>defined(RH_CAN_LEAVE_SAME_DAY)?RH_CAN_LEAVE_SAME_DAY:false,
			'ds_holidays'=>'rh.jours_feries@rh'
		);
	}


	// ============= BUSINESS RULES 
	function canLeaveSameDay() {
		return $this->params['canLeaveSameDay'];
	}

	// v?rifies les regles de coherences pour le cong?s
	function checkRules($user,$force=false) 
	{
		$demandeObj=$this;

		$currentDemandID = $this->oid;
		$startDateTime   = $this->start_date->time;
		$endDateTime	 = $this->end_date->time;
		$halfStartDate   = $this->half_start_date->object;
		$halfEndDate     = $this->half_end_date->object;

		if ($startDateTime==0 or $endDateTime==0)
			return "Merci d'indiquer les dates de votre cong&eacute; ci-dessus";
		
		// StartDate doit ?tre strictement sup?rieure a la date d'aujourd'hui
		$now = strtotime(date("m/d/y",time()));
		if (!$force && !$this->canLeaveSameDay())
		{
			if( $startDateTime <= $now )
				return "La date de d&eacute;but doit &ecirc;tre sup&eacute;rieure &agrave; la date courante";
		}
		else
		{
			if(!$force && ($startDateTime+24*60*60 <= $now) )
				return "La date de d&eacute;but doit &ecirc;tre sup&eacute;rieure &agrave; la date courante";
		}

		if( $startDateTime == $endDateTime )
			return "La date de d&eacute;but doit &ecirc;tre strictement inf&eacute;rieure &agrave; la date de reprise";

		$first_day_weekday = HR_Time::getDayOfTheWeek($startDateTime);
		$last_day_weekday = HR_Time::getDayOfTheWeek($endDateTime);
		if( $first_day_weekday == 7 || $last_day_weekday==7)
			return "La date de début ou de fin de congé ne peut pas être un Dimanche";

		// Le reliquat doit ?tre sup?rieur au nombre de jours demand?s
		$userReliquat = $user->reliquat->object;

		// get reliquat data
		$infos=$this->getInfos($user);
		$demandedDays=$infos['demandedDays'];

		// calculer le nombre de jours si pas d?ja fait..
		if ($demandedDays === null)
		{
			$demandedDays = $this->countWorkingDays(
				$startDateTime, $endDateTime,
				$halfStartDate,$halfEndDate,$nbHols);
		}
		
		// v?rifier si on d?passe le quotat de jours de cong?s
		if( $demandedDays > $userReliquat )
			return "Votre cr&eacute;dit ne vous permet pas de prendre ce cong&eacute;";

		// StartDate doit ?tre inf??rieure ou ?gale au endDate
		if( $startDateTime > $endDateTime )
			return "La date de d&eacute;but du cong&eacute; doit &ecirc;tre inf&eacute;rieure ou &eacute;gale &agrave; la date de reprise";
		
		return $this->checkOverlapRH();
	}

	function checkOverlapRH() {
		// Liste des demandes de cong?s coincidant avec la demande
		$ctxt = Framework::getContext();
		$demands = rh_conge_demandPeer::getByDs('rh.conge.demand.valid_demands',$userOid=null,$params=null,$this->record);
		foreach ($demands->array as $demand)
		{
			if ( 
				($demand->start_date->time >= $this->end_date->time) ||
				($demand->end_date->time <= $this->start_date->time) )
				continue;
			return "Il existe d&eacute;j&agrave; des demandes de cong&eacute; coincidant avec le cong&eacute; que vous demandez";
		}


/*
	$listeConges = $ctxt->getRecords('rh.conge.demand.valid_demands',null,'db',false,$demandeObj);
		if( $listeConges && $listeConges->count() > 0)
			return "Il existe d&eacute;j&agrave; des demandes de cong&eacute; coincidant avec le cong&eacute; que vous demandez";
*/

		// Liste des ordres de mission coincidant avec la demande
		$listeMissions = $ctxt->getRecords('rh.conge.demand.demande_mission',null,'db',false,$this->record);
		if( $listeMissions &&  $listeMissions->count() > 0 )
			return "La demande de cong&eacute; coincide avec un ou plusieurs ordres de mission existants";	

		return true;
	}


	function resetInfos() {
		$this->infos = null;
	}

	// get informations (counters) for that demand
	function getInfos($user)
	{
		if ($this->infos !== null)
				return $this->infos;

		$demandeObj=$this;
		$tab = array();
		
		$startDateField = $demandeObj->start_date;
		$startDateTime  = $startDateField->time;

		$endDateField  	= $demandeObj->end_date;
		$endDateTime    = $endDateField->time;

		$userReliquat 	= $user->reliquat->object;
		$lastCredit 	= $user->last_credit->object;
		$currentCredit	= $user->current_credit->object;
		$currentYearField 	= $user->current_year;
			
		$halfStartDate = $demandeObj->half_start_date->object;
		$halfEndDate   = $demandeObj->half_end_date->object;

		$demandedDays = $this->countWorkingDays(
			$startDateField, $endDateField,
			$halfStartDate,$halfEndDate,$nbHols);

		// get legal days to remove from count
		$legalDays = $this->getLegalDays($demandeObj->legal_days->object,$legalDaysName);

		// remove legal days
		$demandedDays -= $legalDays;

		// test du nombre de jours demandes			
		if ( $demandedDays <= 0)
		{
			$demandedDays = 0 ;
			
			$this->infos = array(
					'status'		=> 'ok',
					'nbHolidays'    => $nbHols,
					'currentCredit' => $currentCredit,
					'lastCredit'    => $lastCredit,
					'demandedDays'  => $demandedDays,
					'reliquat'      => $userReliquat,
					'message'		=> 
						(($legalDays <= 0) ? '' : 'cong&eacute; l&eacute;gal '.$legalDaysName.' = '.$legalDays.' jour'.(($legalDays>1)? 's' : '' ).'').
						(($nbHols <= 0) ? '' : ' jours f&eacute;ri&eacute;s = '.$nbHols.' jour'.(($nbHols>1)? 's' : '' ).'')
			);
			
		}
			
		// fin du test de nombre de jours demandes
		else if ( $userReliquat >= $demandedDays )
		{
			if ( $lastCredit <= 0 )
			{
				$currentCredit = $currentCredit - $demandedDays;
			}
			else
			{
				if ( $lastCredit != 0 )
				{
					if ( $lastCredit >= $demandedDays )
					{
						$lastCredit = $lastCredit - $demandedDays;
					}
					else
					{ 
						$nbDays = 0;
						
						$nbDays = $demandedDays - $lastCredit;
						
						$lastCredit = 0;
						
						$currentCredit = $currentCredit - $nbDays;
					}
				}	  
			}

			$userReliquat = $lastCredit + $currentCredit;

			$this->infos = array(
					"status"		=> 'ok',
					"nbHolidays"    => $nbHols,
					"currentCredit" => $currentCredit,
					"lastCredit"    => $lastCredit,
					"demandedDays"  => $demandedDays,
					"reliquat"      => $userReliquat,
					"legalDays"		=> $legalDays,
					"message"		=> 
						(($legalDays <= 0) ? '' : 'Cong&eacute; l&eacute;gal '.$legalDaysName.' = '.$legalDays.' jour'.(($legalDays>1)? 's' : '' ).'').
						(($nbHols <= 0) ? '' : ' Jour'.(($nbHols>1)? 's' : '' ).' f&eacute;ri&eacute;'.(($nbHols>1)? 's' : '' ).' = '.$nbHols)

				);
		}
		else
		{
			$this->infos = array(
					"status"		=> 'error',
					"message"		=> 'Votre demande de cong&eacute;s d&eacute;passe votre cr&eacute;dit actuel de '.($demandedDays-$currentCredit). ' jours',
					"currentCredit" => $currentCredit,
					"lastCredit"    => $lastCredit,
					"demandedDays"  => $demandedDays,
					"reliquat"      => $userReliquat,
					"legalDays"		=> $legalDays,
					"nbHolidays"    => $nbHols
				);
		}

		return $this->infos;
	  }

	// ================ MANAGING USER COUNTERS (dependent on that workflow rules)

	// apply demand to user counters
	function applyToUser($user,$infos) 
	{
		$user->current_credit->object= $infos['currentCredit'];
		$user->last_credit->object	 = $infos['lastCredit'];
		$user->reliquat->object		 = $infos['reliquat'];
		$user->store($this->ds_user,'update','db');
	}
	
	// reset user counters (undo "apply")
	function resetUser($user,$infos,$days_to_take=null) {
		if ($days_to_take===null)
		{
			$days_to_take=$infos['demandedDays'];
		}

		$user->current_credit->object += $days_to_take;
		$user->reliquat->object += $days_to_take;
		$user->store($this->ds_user,'update','db');
	}

	// le credit de l'utilisateur acquis pour la periode
	function getNominalYearCredit($user,$toDateTime) 
	{
		return 18;
	}

	// refresh hr counters for the demand. Apply counters to the user.
	function refreshReliquat() {
		$user=rh_userPeer::getByOid($this->empid->object);

		// user already updated?
		$state = $this->demand_state->object;
		if ($state==self::DEMAND_VALIDATED || $state==self::DEMAND_PROCESSED || $state==self::DEMAND_FORCE_EDIT)
			$updateUser = true;
		else
			$updateUser = false;

		// put back demanded days to user
		if ($updateUser)
			$this->resetUser($user,null,$this->days_to_take->object);

		// update reliquat
		$this->resetInfos();		
		$this->getInfos($user);		
		$days_to_take=$this->infos['demandedDays'];
		$this->days_to_take->object = $days_to_take;
		$this->store($this->ds_demand,'update','db');

		if ($updateUser)
			$this->applyToUser($user,$this->infos);
	}

	function updateYearCredit($user,$year)
	{
		$startOfNewYear = strtotime('01/01/'.$year);
		$newCredit = $this->getNominalYearCredit($user,$startOfNewYear);

		$userReliquat 	= $user->reliquat->object;
		$lastCredit 	= $user->last_credit->object;
		$currentCredit	= $user->current_credit->object;

		$user->last_credit->object = $user->reliquat->object;
		$user->current_credit->object = $newCredit;
		$user->reliquat->object = $userReliquat + $newCredit;
		$user->current_year->object = $year;

		$user->store();
	}
	
	// date de début d'année ou de service
	function getNbYearsOfService($user,$toDateTime)
	{
		$beginService = $user->start_service->time;
		// $nbMonths = HR_Time::nbFullMonths($beginService, $toDateTime);
//		$nbYears = floor($nbMonths/12);
		$nbYears = HR_Time::nbFullYears($beginService, $toDateTime);
		return $nbYears;
	}

	// date de début d'année ou de service
	function getStartOfYearForUser($user)
	{
		$beginService = $user->start_service->time;
		$startOfYear = strtotime('01/01/'.date("Y"));
		if ($beginService > $startOfYear)
			return $beginService;
		else
			return $startOfYear;
	}
	function getEndOfYearForUser($user)
	{
		$endOfYear = strtotime('31/12/'.date("Y"));
		return $endOfYear;
	}

	/* ========== TIME MANAGEMENT ============ */
	function getLegalDays($legaldayK,&$legalDaysName)
	{
		$day = RH_LegalDayPeer::getByOid($legaldayK);
		if (!$day->isOk())
			throw  new Exception('unknown legal day '.$legalDayType);

		$legalDaysName = $day->legal_day_name->string;
		return (int)$day->allowed_days->html;
	}

	/* returns an array with all holiday days (format Y-m-d), except week ends */
	function getHolidaysArray($startTime,$endTime)
	{
		$hols = new PersistentCollection($this->params['ds_holidays'],'db',
			array('start_date'=>$startTime,'end_date'=>$endTime)
		);
		$holidayObjs = $hols->array;

		// get all days in the holiday period, except week ends
		$hdArray = array();
		for ( $i = 0; $i < count($holidayObjs); $i++ )
		{
			$currentRecord = $holidayObjs[$i];

			$startDatetime	= $currentRecord->start_date->time;
			$endDateTime	= $currentRecord->end_date->time;

			$currentDay = $startDatetime;
			while( $currentDay <= $endDateTime )
			{
				if (!$this->isWeekend($currentDay))
					$hdArray[] = date('Y-m-d',$currentDay);

				$currentDay+=60*60*24;
			}
		}

		return array_unique($hdArray);
	}

	function isWeekend($dateTime)
	{
		$dayOfW = HR_Time::getDayOfTheWeek($dateTime);

		// is saturday part of week end?
		$weekendStart = $this->params['nbDaysperWeek']+1;

		return ($dayOfW >= $weekendStart);
	}

	function isAWorkingDay($dateTime, $holidays)
	{
		// part of the week end?
		if ($this->isWeekend($dateTime))
			return false;

		foreach($holidays as $holiday)
		{
			$time_stamp=strtotime($holiday);

			if ( $dateTime >= $time_stamp && 
				$dateTime  < $time_stamp + 24*60*60)
			{
				// part of a holiday
				return false;
			}
		}

		return true;
	}

	function countWorkingDays($startDateField, $endDateField,$halfStartDate,$halfEndDate,&$nbHols)
	{
		$startDateTime	=$startDateField->time;
		$endDateTime	=$endDateField->time;
		$days = ($endDateTime - $startDateTime) / (60*60*24);

		$no_full_weeks = floor($days / 7);
		$no_remaining_days = fmod ($days, 7);

		$first_day_weekday = HR_Time::getDayOfTheWeek($startDateTime);
		$last_day_weekday = HR_Time::getDayOfTheWeek($endDateTime);

		$first_week = HR_Time::getWeek($startDateTime);
		$last_week = HR_Time::getWeek($endDateTime);

		// case 1: same week
		if ($first_week==$last_week)
		{
			// remove weekend days if same week
			if ($last_day_weekday > $this->params['nbDaysperWeek'] ) 
			{
				$no_remaining_days-= $last_day_weekday - $this->params['nbDaysperWeek'];
			}
		}
		// case 2 : at least on 2 weeks
		elseif ($first_day_weekday <= $last_day_weekday)
		{
			if ($first_day_weekday <= 6 && 6 <= $last_day_weekday) 
			{
				$no_remaining_days--;
			}
			if ($first_day_weekday <= 7 && 7 <= $last_day_weekday) 
			{
				$no_remaining_days--;
			}
		}

		else
		{
			if ($first_day_weekday <= $this->params['nbDaysperWeek'])
			{
				$no_remaining_days-= 7 - $this->params['nbDaysperWeek'];
			}
		}

		
		// count working days in between start/end weeks
		$nbDaysperWeek=$this->params['nbDaysperWeek'];
		$workingDays = $no_full_weeks * $nbDaysperWeek;
		if ($no_remaining_days > 0 )
		{
			$workingDays += $no_remaining_days;
		}
		
		// remove holidays
		$start=trim($startDateField->db,"'");
		$end=trim($endDateField->db,"'");
		$holidays=$this->getHolidaysArray($start,$end);

		$nbHols=0;
		foreach($holidays as $holiday)
		{
			$time_stamp=strtotime($holiday);

			if ( $startDateTime <= $time_stamp && $time_stamp < $endDateTime && 
				!$this->isWeekend($time_stamp) )
			{
				$nbHols++;
				$workingDays--;
			}
		}

		// update with half days
		if( $workingDays > 0 || ($first_week==$last_week && $first_day_weekday == $last_day_weekday))
		{
			if ($this->isAWorkingDay($startDateTime, $holidays))
				$workingDays -= (($halfStartDate==1)?0.5:0);
				
			if ($this->isAWorkingDay($endDateTime, $holidays))
				$workingDays +=(($halfEndDate==1)?0.5:0);
		}

		return $workingDays;
	}
	// ===================== for tests
	function __toString() {
		try {
			if (!$this->isOk())
				return 'empty demand';

			$user = rh_userPeer::getByOid($this->empid->object);
			if (!$user->isOk())
				$user='';

			$s="Demand {$this->ruleset} ({$this->request_date->string}): user:{$user} start:{$this->start_date->string} end:{$this->end_date->string} state:{$this->demand_state->string}<br/>";
			$infos=$this->getInfos($user);
			foreach ($infos as $k=>$v)
				$s.=(", $k:$v");

			$fgroup=$this->getDemandValidatorGroup();
			if ($fgroup->object)
			{
				$group = rh_groupPeer::getByOid($fgroup->object);
				$s .= "<br/>group validator:{$group}";
			}

			return $s;
		} catch (Exception $e) { return $e; }
	}

}

/** log management for that demand */
class rh_validation_log extends PersistentObject
{
	function __toString() {
		if (!$this->isOk())
			return 'empty log entry';

		$validator = rh_userPeer::getByOid($this->validator_oid->object);
		if (!$validator->isOk())
			$user='';

		$s="date:{$this->validation_date->string} by:{$validator} action:{$this->demand_state_log->string}";
		return $s;
	}

	function add($ds='rh.conge.log',$media='db')
	{
		$this->store($ds,'insert');

		// get oid back to object
		$this->oid->object=$this->getLastInsertId($media);
	}
}

class rh_validation_logPeer 
{
	static function getEmpty($ds='rh.conge.log') {
		$o = new rh_validation_log();
		$o->load($ds,'empty');
		return $o;
	}
	static function getByOid($oid,$ds='rh.conge.log') {
		$o = new rh_validation_log();
		$o->load($ds,'db', array('oid'=>$oid));
		return $o;
	}
	static function getListByValidator($oid,$ds='rh.conge.log.byValidator') {
		$col = new PersistentCollection();
		$col->load($ds,'db', array('validator_oid'=>$oid));
		return $col->rh_validation_log;
	}
	static function getListByDemand($oid,$ds='rh.conge.log.byDemand') {
		$col = new PersistentCollection();
		$col->load($ds,'db', array('demand_id'=>$oid));
		return $col->rh_validation_log;
	}
	static function getListByUser($oid,$ds='rh.conge.log.byUser') {
		$col = new PersistentCollection();
		$col->load($ds,'db', array('user_oid'=>$oid));
		return $col->rh_validation_log;
	}
	static function deleteByDemand($oid,$ds='rh.conge.log.byDemand') {
		$log=rh_validation_logPeer::getListByDemand($oid,$ds);
		foreach ($log as $entry)
		{
			$entry->delete('rh.conge.log','db',array('oid'=>$entry->oid->object));
		}
	}
}

?>
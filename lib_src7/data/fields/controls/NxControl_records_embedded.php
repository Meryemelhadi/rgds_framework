<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// singleton
$GLOBALS['NxControl_records_level']=0;

class NxControl_records
{
	function __construct()
	{
	}
	function loadFromDB(&$f)
	{
	}

	function & storeToDB(&$f,$op='insert')
	{	
	}

	function embed($f) {

	}

	function loadRecords(&$f)
	{
		$desc=&$f->desc;		
		$this->props =& $desc;

		// get field name
		$fname=$f->getAlias();
		$fwildcard='$'.$desc->getAlias().'$';
		$fRegWildcard='[$]'.$desc->getAlias().'[$]';

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$insertTop=$desc->getProperty('insert_new','top',false)=='top'?true:false;
		$kval=$f->record->getProperty($key,'',false,'object');
		$force = $desc->getProperty('load_on_empty','false',false);
		if ($kval!=='' || $force=="true") 
		{
			$key_group=$desc->getProperty('key_group','fk_group',false);
			$max_recs=$desc->getProperty('max_records','0',false);
			$min_recs=$desc->getProperty('min_records','0',false);
						  
			//get DS and view
			$media=null;
			$view=null;
			$ds = $this->getDS($desc,$media,$view,'ds');
			if ($ds==null)
				return null;

			// set key property
			$ds->setProperty($key,$kval);
			$ds->setProperty($key_group,$fname);
			$ds->setProperty('parentRecord',$f->record);
			
			// get records
			$errorCB=new NxErrorCB();
			$records =  $ds->getRecords($view, $errorCB);
		}
		else
			$records = null;

		// get empty record ds
		$media_empty='empty';
		$view_empty=null;
		$ds_empty = $this->getDS($desc,$media_empty,$view_empty,'ds_record');
		if ($ds_empty==null)
			return null;

		$ds_empty->setProperty($key,$kval);
		$ds_empty->setProperty($key_group,$fname);
		$ds_empty->setProperty('parentRecord',$f->record);


		// complete with empty records if required
		if ($records != null)
			$count = $records->count();
		else
			$count = 0;

		// add a prefix to each of these records
		$i=0;
		if ($count > 0)
		{
			// clone records to get in memory collection
			$records = $records->clone_it($errorCB,false);
		}
		else
		{
			include_once(NX_LIB.'NxData_ArrayRecords.inc');
			$records = new NxData_ArrayRecords(null);
		}


		// complete collection if minimum number is required
		for ($i=$count; $i < $min_recs; $i++)
		{
			$empty_records = & $ds_empty->getRecords($view_empty, $errorCB);
			$iter = $empty_records->getRecordIterator();
			$empty_record= $iter->getCurrent();
			$empty_record->parentRecord = $f->record;
			$records->addRecord($empty_record);
			$count++;
		}

		// set record prefix
		if ($count > 0)
			for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
			{
				$record =& $iter->getCurrent();
				$record->setPrefix("{$fname}_{$i}__");
				$f->record->updateWith($record,$override='no',$append='yes');
			}						
	}	

	function toRecords(&$f) 
	{
		$desc=&$f->desc;		
		$this->props =& $desc;

		// get field name
		$fname=$f->getAlias();

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$kval=$f->record->getProperty($key,'',false,'object');
		$key_group=$desc->getProperty('key_group','fk_group',false);
		
		//get DS and view
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view);
		if ($ds==null)
			return null;
			
		// set key property
		$ds->setProperty($key,$kval);
		$ds->setProperty($key_group,$fname);
		$ds->setProperty('parentRecord',$f->record);
		
		// get records
		$errorCB=new NxErrorCB();
		$records = & $ds->getRecords($view, $errorCB);
		
		return $records;
		
	}
	
	function toHTML(&$f)
	{
		$desc=&$f->desc;		
		$this->props =& $desc;

		// get field name
		$fname=$f->getAlias();

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$kval=$f->record->getProperty($key,'',false,'object');
		if ($kval==='')
			return '';

		$key_group=$desc->getProperty('key_group','fk_group',false);
		
		//get DS and view
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view);
		if ($ds==null)
			return null;
			
		// set key property
		$ds->setProperty($key,$kval);
		$ds->setProperty($key_group,$fname);
		$ds->setProperty('parentRecord',$f->record);
		
		// get records
		$errorCB=new NxErrorCB();
		$records = & $ds->getRecords($view, $errorCB);

		// add a prefix to each of these records (for groups)
		$i=0;
		if ($records != null && $records->count()>0)
		{
			// clone records to get in memory collection
			$records = $records->clone_it($errorCB,false);
			
			for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
			{
				$record =& $iter->getCurrent();
//				$record->setPrefix("{$fname}_{$i}__");
			}
		}		

			
		// get TPL source
		if (($tpl=$desc->getProperty('tpl',null,false))==null)
			if (($tpl=$desc->getProperty($fname.'.tpl',null,false))==null)
			{
				// exec tpl view
				if ((($view=$desc->getProperty('view',null,false))==null) && 
					(($view=$desc->getProperty($fname.'.view',null,false))==null))
					return null;

				if (isset($GLOBALS['NX_SKIN']))
					$skin=$GLOBALS['NX_SKIN'];
				else
					$skin=null;
					
				$rec_skin=$desc->getProperty('skin','default',true);

				$desc->runView(
					$view,
					array('page.skin'=>$rec_skin,'page.records'=>$records,'fmethod'=>'toHTML'),
					'page.subform','', 'auto',$rec_skin);
				if ($skin)
					$GLOBALS['NX_SKIN']=$skin;
				else
					unset($GLOBALS['NX_SKIN']);
				
				return $desc->getProperty('page.subform','',false);
			}
			
		// $tpl='<div class="field"><span class="label">{field.label}</span><span class="value">{field}</span></div>';
		$s='';
		// exec inline tpl view
		if ($tpl!=null)
		{
			// add empty record
		//	$s.=$this->execTPL($empty_record,$tpl,'html');
		
			// add other records
			if ($records != null && $records->count()>0)
			{
				for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
				{
					$record =& $iter->getCurrent();
					$record->recordPrefix="{$i}_{$fname}_";
					$record->parentRecord = $f->record;
					$s.=$this->execTPL($record,$tpl,'html');
				}
			}
			else
				$s.=$desc->getProperty($fname.'.html','',false);
			
			return $s;
		}

		return '';
	}
	function toForm($value,&$f)
	{
		$desc=&$f->desc;		
		$this->props =& $desc;

		// get field name
		$fname=$f->getAlias();
		$fwildcard='$'.$desc->getAlias().'$';
		$fRegWildcard='[$]'.$desc->getAlias().'[$]';

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$insertTop=$desc->getProperty('insert_new','top',false)=='top'?true:false;
		$kval=$f->record->getProperty($key,'',false,'object');
		$force = $desc->getProperty('load_on_empty','false',false);
		if ($kval!=='' || $force=="true") 
		{
			$key_group=$desc->getProperty('key_group','fk_group',false);
			$max_recs=$desc->getProperty('max_records','0',false);
			$min_recs=$desc->getProperty('min_records','0',false);
						  
			//get DS and view
			$media=null;
			$view=null;
			$ds = $this->getDS($desc,$media,$view,'ds');
			if ($ds==null)
				return null;

			// set key property
			$ds->setProperty($key,$kval);
			$ds->setProperty($key_group,$fname);
			$ds->setProperty('parentRecord',$f->record);
			
			// get records
			$errorCB=new NxErrorCB();
			$records =  $ds->getRecords($view, $errorCB);
		}
		else
			$records = null;

		// get empty record ds
		$media_empty='empty';
		$view_empty=null;
		$ds_empty = $this->getDS($desc,$media_empty,$view_empty,'ds_record');
		if ($ds_empty==null)
			return null;

		$ds_empty->setProperty($key,$kval);
		$ds_empty->setProperty($key_group,$fname);
		$ds_empty->setProperty('parentRecord',$f->record);


		// complete with empty records if required
		if ($records != null)
			$count = $records->count();
		else
			$count = 0;

		// add a prefix to each of these records
		$i=0;
		if ($count > 0)
		{
			// clone records to get in memory collection
			$records = $records->clone_it($errorCB,false);
		}
		else
		{
			include_once(NX_LIB.'NxData_ArrayRecords.inc');
			$records = new NxData_ArrayRecords(null);
		}


		// complete collection if minimum number is required
		for ($i=$count; $i < $min_recs; $i++)
		{
			$empty_records = & $ds_empty->getRecords($view_empty, $errorCB);
			$iter = $empty_records->getRecordIterator();
			$empty_record= $iter->getCurrent();
			$empty_record->parentRecord = $f->record;
			$records->addRecord($empty_record);
			$count++;
		}

		// set record prefix
		if ($count > 0)
			for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
			{
				$record =& $iter->getCurrent();
				$record->setPrefix("{$fname}_{$i}__");
				$record->parentRecord = $f->record;
			}

		$empty_records = & $ds_empty->getRecords($view_empty, $errorCB);
		$iter = $empty_records->getRecordIterator();
		$empty_record=& $iter->getCurrent();
        $empty_rec_prefix="{$fname}_{$fwildcard}__";
		$empty_record->setPrefix($empty_rec_prefix);
			
		// get TPL source
		if (($tpl=$desc->getProperty('tpl',null,false))==null)
			if (($tpl=$desc->getProperty($fname.'.tpl',null,false))==null)
			{
		   		$GLOBALS['NxControl_records_level']++;
				$desc->setProperty('record_level',$GLOBALS['NxControl_records_level']);

				// exec tpl view
				if ((($view=$desc->getProperty('edit_view',null,false))==null) && 
					(($view=$desc->getProperty('view',null,false))==null) && 
					(($view=$desc->getProperty($fname.'.view',null,false))==null))
					return null;
				$desc->runView($view,array('page.records'=>$empty_records,'fmethod'=>'toHTML','sfrecord.parentId'=>$f->record->getPrefix(),'sfrecord.isNew'=>1,'subrecord.counter'=>'$fwildcard'),'page.subform');
				$pattern='__pattern__'.$fname;
				$newTarget=$pattern.'_new';
//				$jsPattern=preg_replace('/_##/','',$pattern);
				$jsPattern=$pattern;
				$desc->setProperty('sf_record_name',$fname);
				if ($max_recs > 0)
					$max_limit="if (window.{$jsPattern}_counter >= {$max_recs}) return;";
				else
					$max_limit='';
				
				$jsNew=<<<EOH
				if (typeof window.{$jsPattern}_counter == "undefined")
					window.{$jsPattern}_counter = {$i};

				window.new$jsPattern=function(after,isTop) {
					{$max_limit}
					var pat=document.getElementById('$pattern');
					var newTarget=document.getElementById('{$newTarget}');
					if (!pat)
						return;
						
					var s= pat.innerHTML.replace(/{$fRegWildcard}_counter/g,{$jsPattern}_counter+1).replace(/{$fRegWildcard}/g,{$jsPattern}_counter++);

					var newEl=document.createElement('div');
                    newEl.innerHTML=s;                     						


					if (isTop && newTarget.hasChildNodes())
						newTarget.insertBefore(newEl,newTarget.firstChild)
					else
						newTarget.appendChild(newEl);

					// exec generated scripts
                    var scriptsEls=newEl.getElementsByTagName('script');
                    for (var i=0;i<scriptsEls.length;i++)
                    {
                       script = scriptsEls[i].text;
                        if (script!='')
                            eval(script);
                    }



				}
EOH;

				if (false)
                    $jsNew='<script>'.$jsNew.'</script>'.'<span style="display:none"><span class="script">'.$jsNew.'</span></span>';
                else
                    $jsNew='<script>'.$jsNew.'</script>';

				$add = $desc->getProperty('add_button','yes',false);                
                if ($add=='yes' || $add=='true')
                {
					$addStr=$desc->getPrompt($desc->getString('add record',null,'form_fields').' '.$fname);
					$s = '<div class="fields_records_wrapper"><div class="fields_records_add" onclick="window.new'.$jsPattern.'(this,'.($insertTop?'true':'false').')"><span>'.$addStr.'</span></div>
					<div style="display:none" id="'.$pattern.'">'.$jsNew.$desc->getProperty('page.subform','',false).
	                '<input type="hidden" name="'.$empty_rec_prefix.'__op__" value="new"/></div>';
				}
				else
                {
					$s = '<div class="fields_records_wrapper">
					<div style="display:none" id="'.$pattern.'">'.$jsNew.$desc->getProperty('page.subform','',false).
	                '<input type="hidden" name="'.$empty_rec_prefix.'__op__" value="new"/></div>';
				}
				if ($insertTop)
					$s.='<div id="'.$newTarget.'"></div>';
									
				if($count>0)
				{
					$desc->runView($view,array('page.records'=>$records,'fmethod'=>'toHTML','sfrecord.isNew'=>0,'sfrecord.parentId'=>$f->record->getPrefix(),'delete_button'=>$desc->getProperty('delete_button','yes',false)),'page.subform');
					$s .= $desc->getProperty('page.subform','',false);
				}

				if (!$insertTop)
					$s.='<div id="'.$newTarget.'"></div>';

				$s.='</div>';
				
				$GLOBALS['NxControl_records_level']--;
				return $s;
			}
			
		// $tpl='<div class="field"><span class="label">{field.label}</span><span class="value">{field}</span></div>';
		$s='';
		// exec inline tpl view
		if ($tpl!=null)
		{
	   		$GLOBALS['NxControl_records_level']++;
			$ds->setProperty('record_level',$GLOBALS['NxControl_records_level']);

			// add empty record
			$s.=$this->execTPL($empty_record,$tpl,'form');
		
			// add other records
			if ($records != null && $records->count()>0)
			{
				for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
				{
					$record =& $iter->getCurrent();
					$record->recordPrefix="{$i}_{$fname}_";
					$record->parentRecord = $f->record;
					$s.=$this->execTPL($record,$tpl,'form');
				}
			}
			else
				$s.=$desc->getProperty($fname.'.html','',false);
			
	   		$GLOBALS['NxControl_records_level']--;
			return $s;
		}

		return '';
	}

	function readForm(&$recData,&$f)
	{
		$desc=&$f->desc;		
		$this->props =& $desc;

		// get field prefix
		$fprefix=$f->getAlias();

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$key_group=$desc->getProperty('key_group','fk_group',false);
		$kval=$f->record->getProperty($key,'',true,'object');
		
		// get field used as FK
		$fkname=$desc->getProperty('fkey','parent_oid',false);
		
		// get records: extract fields as (index)_(fname1)_(fname2)
		$this->data = array();
		$name=preg_replace('/[.-]+/','_',$fprefix);
		foreach ($recData as $k=>$v)
		{
			// fillup data[rec index][fname]
			preg_replaceX("/^({$name}_([0-9]+)__)([^$]*)$/e",'\$this->data[$2]["$3"]=\$v;',$k,$this);
		}
		$data=$this->data;

		$this->files = array();
		foreach ($_FILES as $k=>$v)
		{
			// fillup data[rec index][fname]
			preg_replaceX("/^({$name}_([0-9]+)__)(.*)$/e",'\$this->files[$2]["$3"]=\$v;',$k,$this);
		}
		$files=$this->files;
				
		// get DS
		$media=null;
		$view=$view_record=null;
		$ds = $this->getDS($desc,$media,$view_record,'ds_record');
		if ($ds==null)
			return null;
		$ds->setProperty($key_group,$fprefix);
		$view=$view_record;
			
		// $view_record=$desc->getProperty('ds_record',$view.'.record',false);
		
		include_once(NX_LIB.'NxData_RecordInputSource.inc');
		$errorCB=new NxErrorCB();
		$recDesc=$ds->getRecDesc($view,$errorCB);
		if (!$recDesc)
			return null;
			
		// put records (if exist, update, else insert new)
		$f->newRecords=array();
		$_saveFILES=$_FILES;
		$f->record->saveLastInsertID();
		foreach ($data as $i=>$recData2)
		{				
			// init files collection for that subform
			$_FILES=array();
			if (count($files))
				foreach ($files[$i] as $fn=>$finfo)
				{
					$_FILES[$fn]=$finfo;
				}
			
			$rec2 = new NxData_RecordInputSource($recDesc);
			$rec2->initRecord($recData2, 'form', $errorCB);

			// set key property
			$ds->setProperty($key,$kval);
			$ds->setProperty($key_group,$fprefix);
			$ds->setProperty('parentRecord',$f->record);
			
			// new record if tagged by the JS or if no valid oid
			if ($recData2['__op__']=='new'  || $rec2->record->oid->object=='')
			{
				if (!(
					(isset($recData2['__op__']) && $recData2['__op__']=='delete') || 
					(isset($recData2['__delete__'])&&($recData2['__delete__']=='delete'))
					))
				{
					// check if parent has already been added (if not yet created, will require a second pass)
					$k=$rec2->record->getFieldValue($fkname,'object',null);
					if ($k && ($k!='0'))
                    {
                        nxLog("SUB FORM : INSERT 1) with KEY($fkname)=$k",'SUB_FORM');
						$ds->putRecords($rec2,$view_record,$errorCB,'insert');
                    }
					elseif($kval && ($kval!='0'))
                    {
                        nxLog("SUB FORM : INSERT 2) with KEY($fkname)=$kval",'SUB_FORM');
                        $rec2->record->setField($fkname,$kval);
                        $ds->putRecords($rec2,$view_record,$errorCB,'insert');
                    }
                    else
						$f->newRecords[]=$rec2;
				}
			}
			elseif ($recData2['__op__']=='delete' || 
				(isset($recData2['__delete__'])&&($recData2['__delete__']=='delete')))
			{
				$ds->deleteRecords($view_record,$rec2,$errorCB);			
			}
			elseif (!isset($recData2['__op__']))
			{
				// no op => update
				$ds->putRecords($rec2,$view_record,$errorCB,'update');
			}
			else
				nxError('bad operation for field (type=records), sub form:'.$recData2['__op__'],'DATA');
		}

		$f->record->restoreLastInsertID();
		$_FILES=$_saveFILES;
		return '';
	}

	function onNew(&$f,$fk)
	{
		$desc=&$f->desc;
		$this->props =& $desc;

		// get field prefix
		$fprefix=$f->getAlias();

		// get field used as FK
		$fkname=$desc->getProperty('fkey','parent_oid',false);
						
		// get DS
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view,'ds_record');
		if ($ds==null)
			return null;

		$ds->setProperty('fk_group',$fprefix);
		
		$errorCB=new NxErrorCB();
		$recDesc=$ds->getRecDesc($view,$errorCB);
		if (!$recDesc)
			return null;
			
		// put records (if exist, update, else insert new)
		if (!$f || !isset($f->newRecords))
			return '';

		$f->record->saveLastInsertID();

		foreach ($f->newRecords as $rec2)
		{				
			$rec2->record->setField($fkname,$fk);
			$ds->putRecords($rec2,$view,$errorCB,'insert');
		}

		$f->record->restoreLastInsertID();

		return '';
	}

	function onDelete($f, &$errorCB)
	{
		$desc=&$f->desc;		
		$this->props =& $desc;

		// get field name
		$fname=$f->getAlias();
		$fwildcard='$'.$desc->getAlias().'$';
		$fRegWildcard='[$]'.$desc->getAlias().'[$]';

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$kval=$f->record->getProperty($key,'',false,'object');
		$key_group=$desc->getProperty('key_group','fk_group',false);
		$max_recs=$desc->getProperty('max_records','0',false);
		              
		//get DS and view
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view,'ds');
		if ($ds==null)
			return null;

		// set key property
		$ds->setProperty($key,$kval);
		$ds->setProperty($key_group,$fname);
		$ds->setProperty('parentRecord',$f->record);

		// get records
		$errorCB=new NxErrorCB();
		$records =& $ds->getRecords($view, $errorCB);
		
		$ds->deleteRecords($view,$records,$errorCB);			
	}

	function getDS(&$desc,&$media,&$view,$att='ds') 
	{
		// get ds name
		if (($dsn=$desc->getProperty($att,null,false))==null)
			return '';
			
		// get ds settings
		if ($dsn!='')
		{
			$ads=explode(':',$dsn);
			switch(count($ads))
			{
				case 2:
					list($media2,$view)=$ads;
					break;
				case 1:
					list($view)=$ads;
					$media2='db';
					break;
				default:
					return null;
			}
		}
		else
		{
			return null;
		}

		if ($media == null)
			$media=$media2;

		// get db data source plugin		
		$ds =& $desc->getPlugIn(strtolower($media),'NxDS_','ds');
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","control_records");
			return null;
		}
		
		return $ds;
	}

	function execTPL($record,$tpl,$fmethod='html')
	{
		$s='';
		if (!$record)
			return $s;
		
		$this->record=$this->record;
		$iterField = $record->getFieldIterator();
		for ($iterField->reset(); $iterField->isValid(); $iterField->next())
		{
			$this->field = & $iterField->getCurrent();
			
			$s.=preg_replaceX(
				array(
					'/\\[/',
					'/\\]/',
					'/[{%][{%]?(field.(label))[}%]/e',
					'/[{%][{%]?(field.(js|html|object))[}%]/e',
					'/[{%][{%]?(field)[}%]/e',
					'/[{%][{%]?(field.(js|html|object)):([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?[}%]/e',
					'/[{%][{%]?(field):([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?[}%]/e',
					), 
				array(
					'<','>',
					'$this->field->get$2()',
					'$this->field->to$2()',
					'$this->field->to'.$fmethod.'()',
					'$this->mapField($this->record,"$3","$2")',
					'$this->mapField($this->record,"$2","'.$fmethod.'")',
					),
				$tpl,$this);
		}
			
		return $s;
	}
	
	function mapString($str,$pack)
	{
		return $this->getString($str,null,$pack);
	}
	
	function mapProp($p,$isDeft,$dft,$static,$inPHP)
	{
		return $this->props->getProperty(trim($p),$dft);
	}
	
	// return label for field (or value if field not found)
	function mapField($record,$name,$method='html'){
		if ($record)
		{
			$f=$record->getField($name);
			if (!$f)
				$f=$fval;			
		}
		else
			$f=$fval;
			
		if (!$f)
			return '';
			
		$method="to$method";
		return str_replace("'","\\'",$f->$method());
	}	
}

?>
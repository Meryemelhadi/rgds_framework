<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxControl_records
{
	function __construct()
	{
	}
	
	function toHTMLold(&$f)
	{
		$this->props =& $desc;

		// get field name
		$fname=$f->getAlias();
		
		//get DS and view
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view);
		if ($ds==null)
			return null;
		
		// get records
		$records = & $ds->getRecords($view, $this->error);
		
		// get TPL source
		if (($tpl=$desc->getProperty('tpl',null,false))==null)
			if (($tpl=$desc->getProperty($fname.'.tpl',null,false))==null)
			{
				// exec tpl view
				if (($view=$desc->getProperty('view',null,false))==null)
					return null;
					
				ob_start();
					$desc->runView($view,array('page.records'=>$records));
					$this->setProperty($propName,$s=ob_get_contents());
				ob_end_clean();
				
				return $s;
			}
			
		// no view, exec inline tpl view (only {field} macro is supported)
		if ($tpl!=null)
		{
			// ok we're in, we have the records, lets build the source now..
			$s='';
			if ($records != null && $records->count()>0)
				for ($iter = $records->getRecordIterator(); $iter->isValid(); $iter->next())
				{
					$record =& $iter->getCurrent();				
					$s.=$this->execTPL($record,$tpl,'html');
				}
			else
				$s=$desc->getProperty($fname.'.html','',false);
			
			return $s;
		}

		return '';
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
				$record->setPrefix("{$fname}_{$i}__");
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

				$desc->runView($view,array('page.records'=>$records,'fmethod'=>'toHTML'),'page.subform');
				return $desc->getProperty('page.subform','',false);
			}
			
		// $tpl='<div class="field"><span class="label">{field.label}</span><span class="value">{field}</span></div>';
		$s='';
		// exec inline tpl view
		if ($tpl!=null)
		{
			// add empty record
			$s.=$this->execTPL($empty_record,$tpl,'html');
		
			// add other records
			if ($records != null && $records->count()>0)
			{
				for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
				{
					$record =& $iter->getCurrent();
					$record->recordPrefix="{$i}_{$fname}_";
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
		
		// get records
		$errorCB=new NxErrorCB();
		$records = $ds->getRecords($view, $errorCB);
		$records2=null;

		// add a prefix to each of these records
		$i=0;
		if ($records != null && $records->count()>0)
		{
			// clone records to get in memory collection
			$records2 = $records->clone_it($errorCB,false);
			
			for ($iter = $records2->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
			{
				$record = $iter->getCurrent();
				//nxLog("set record prefix[{$i}]={$fname}_{$i}__ , count=".$record->count());
				$record->setPrefix("{$fname}_{$i}__");
				/*for ($iterf = $record->getFieldIterator(); $iterf->isValid(); $iterf->next())
				{
					$f =& $iterf->getCurrent();
					nxLog("check field alias=".$f->getAlias());				
					nxLog("check field record prefix=".$f->record->getPrefix());					
				}*/		
			}
			
			/*for ($iter =& $records2->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
			{
				$record =& $iter->getCurrent();
				nxLog("===== get record prefix[{$i}]=".$record->getPrefix());
				for ($iterf = $record->getFieldIterator(); $iterf->isValid(); $iterf->next())
				{
					$f =& $iterf->getCurrent();
					nxLog("get field alias=".$f->getAlias());				
					nxLog("get field record prefix=".$f->record->getPrefix());					
				}		
			}*/		
		}		

		// get empty record		
		$ds =& $desc->getPlugIn('empty','NxDS_','ds');
		$ds->setProperty($key,$kval);
		$ds->setProperty($key_group,$fname);
		
		$empty_records = & $ds->getRecords($view, $errorCB);
		$iter = $empty_records->getRecordIterator();
		$empty_record=&$iter->getCurrent();
		$emptyPrefix="{$fname}_{$fwildcard}__";
		//nxLog("set empty prefix=$emptyPrefix");
		$empty_record->setPrefix($emptyPrefix);
		
//		$emptyPrefix=$empty_record->getPrefix();
//		nxLog("get empty prefix=$emptyPrefix");
//		$empty_record->recordPrefix="100_{$fname}_";
			
		// get TPL source
		if (($tpl=$desc->getProperty('tpl',null,false))==null)
			if (($tpl=$desc->getProperty($fname.'.tpl',null,false))==null)
			{
				// exec tpl view
				if ((($view=$desc->getProperty('edit_view',null,false))==null) && 
					(($view=$desc->getProperty('view',null,false))==null) && 
					(($view=$desc->getProperty($fname.'.view',null,false))==null))
					return null;

				$desc->runView($view,array('page.records'=>$empty_records,'fmethod'=>'toHTML'),'page.subform');
				$pattern='__pattern__'.$fname;
				$newTarget=$pattern.'_new';
//				$jsPattern=preg_replace('/_##/','',$pattern);
				$jsPattern=$pattern;
				$jsNew=<<<EOH
				<script>
				window.{$jsPattern}_counter = {$i};
				window.new$jsPattern=function(after) {				
					var pat=document.getElementById('$pattern');
					var newTarget=document.getElementById('{$newTarget}');
					if (!pat)
						return;
						
					var s= pat.innerHTML.replace(/{$fRegWildcard}/g,{$jsPattern}_counter++);
					//debugger;
					
					var reScript=/<script[^>]*>([\\S\\s]*?)<\/script>/ig;
					var script='',match;
					if (match=reScript.exec(s))
						while (match=reScript.exec(s))
						{
							script += match[1];
						}
					if (script!='')
						eval(script);
						
					newTarget.innerHTML+=s;
				}
				</script>
EOH;
				$addStr=$desc->getString('add record',null,'form_fields');
				$s = '<div class="fields_records_add" onclick="new'.$jsPattern.'(this)"><span>'.$addStr.'</span></div>
				<div style="display:none" id="'.$pattern.'">'.$jsNew.$desc->getProperty('page.subform','',false).'<input type="hidden" name="'.$emptyPrefix.'__op__" value="new"/></div>';
									
				$desc->runView($view,array('page.records'=>$records2,'fmethod'=>'toHTML'),'page.subform');
				$s .= $desc->getProperty('page.subform','',false);
				$s.='<div id="'.$newTarget.'"></div>';
				
				return $s;
			}
			
		// $tpl='<div class="field"><span class="label">{field.label}</span><span class="value">{field}</span></div>';
		$s='';
		// exec inline tpl view
		if ($tpl!=null)
		{
			// add empty record
			$s.=$this->execTPL($empty_record,$tpl,'form');
		
			// add other records
			if ($records != null && $records->count()>0)
			{
				for ($iter = $records->getRecordIterator(),$i=0; $iter->isValid(); $iter->next(),$i++)
				{
					$record =& $iter->getCurrent();
					$record->recordPrefix="{$i}_{$fname}_";
					$s.=$this->execTPL($record,$tpl,'form');
				}
			}
			else
				$s.=$desc->getProperty($fname.'.html','',false);
			
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
			$this->v = $v;
			preg_replaceX("/^({$name}_([0-9]+)__)([^$]*)$/e",'\$this->data[$2]["$3"]=\$this->v;',$k,$this);
		}
		$data = $this->data;

		$this->files = array();
		foreach ($_FILES as $k=>$v)
		{
			// fillup data[rec index][fname]
			$this->v = $v;
			preg_replaceX("/^({$name}_([0-9]+)__)(.*)$/e",'\$this->files[$2]["$3"]=\$this->v;',$k,$this);
		}
		$files = $this->files;
				
		// get DS
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view);
		if ($ds==null)
			return null;
		$ds->setProperty($key_group,$fprefix);
			
		$view_record=$desc->getProperty('ds_record',$view.'.record',false);
		
		include_once(NX_LIB.'NxData_RecordInputSource.inc');
		$errorCB=new NxErrorCB();
		$recDesc=$ds->getRecDesc($view,$errorCB);
		if (!$recDesc)
			return null;
			
		// put records (if exist, update, else insert new)
		$f->newRecords=array();
		$_saveFILES=$_FILES;
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
			
			if ($recData2['__op__']=='new')
			{
				if (!($recData2['__op__']=='delete' || 
					(isset($recData2['__delete__'])&&($recData2['__delete__']=='delete'))))
				{
					// check if parent has already been added (if not yet created, will require a second pass)
					$k=$rec2->record->getFieldValue($fkname,'object',null);
					if ($k && ($k!='0'))
						$ds->putRecords($rec2,$view,$errorCB,'insert');
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
		$ds = $this->getDS($desc,$media,$view);
		if ($ds==null)
			return null;

			$ds->setProperty('fk_group',$fprefix);
		
		$errorCB=new NxErrorCB();
		$recDesc=$ds->getRecDesc($view,$errorCB);
		if (!$recDesc)
			return null;
			
		// put records (if exist, update, else insert new)
		foreach ($f->newRecords as $rec2)
		{				
			$rec2->record->setField($fkname,$fk);
			$ds->putRecords($rec2,$view,$errorCB,'insert');
		}

		return '';
	}

	function onDelete($f, &$errorCB)
	{
		$desc=&$f->desc;		
		
		//get DS and view
		$media=null;
		$view=null;
		$ds = $this->getDS($desc,$media,$view);
		if ($ds==null)
			return null;
		
		// get records
		$errorCB=new NxErrorCB();
		$records =& $ds->getRecords($view, $errorCB);
		
		$ds->deleteRecords($view,$records,$errorCB);			
	}

	function getDS(&$desc,&$media,&$view) {
				
		// get ds name
		if (($dsn=$desc->getProperty('ds',null,false))==null)
			return '';
			
		// get ds settings
		if ($dsn!='')
		{
			$ads=explode(':',$dsn);
			switch(count($ads))
			{
				case 2:
					list($media,$view)=$ads;
					break;
				case 1:
					list($view)=$ads;
					$media='db';
					break;
				default:
					return null;
			}
		}
		else
		{
			return null;
		}

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
		$this->record = $record;

		$iterField = $record->getFieldIterator();
		for ($iterField->reset(); $iterField->isValid(); $iterField->next())
		{
			$this->field = & $iterField->getCurrent();
			
			$fform=$field->toForm();
			
			$s.=preg_replaceX(
				array(
					'/[{%](field.(label))[}%]/e',
					'/[{%](field.(js|html|object))[}%]/e',
					'/[{%](field)[}%]/e',
					'/[{%](field.(js|html|object)):([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?[}%]/e',
					'/[{%](field):([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?[}%]/e',
					), 
				array(
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
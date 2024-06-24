<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
	CASE 1 : mode 1->N (foreign table (N recs)pointing to table (1 rec))

		FIELD IN TABLE 1 REC
		<field name="sites" label="sites"  
			type="text-choice" multiple="yes" control="lists_filter" 
			ds="formation_store.listAll@formation_catalog" 
			ds_format="{field:name} {field:postcode}" ds_field_value="eoid" prompt="Choisissez ..." 

			ds_foreign_table_get="formation_store.get_by_bomanager" 
			ds_foreign_table_set="formation_store.set_bomanager_by_store_list"/>

		TABLE VIEWS IN FOREIGN TABLE (N recs)
		<view name="formation_store.get_by_bomanager" table="formation_store">		
			<key>
				<field name="bomanager"  value="%{property:key}"/>
			</key>
			<sort>
				<field name="name" label="name"  direction="asc" />
			</sort>
			<fields>
				<field name="bomanager" label="bomanager"  multiLines="no" type="line" isProp="key" />
				<field name="eoid" label="soid" type="ref-oid" isKey="secondary" isProp="value"/>
			</fields>		
		</view>

		<view name="formation_store.set_bomanager_by_store_list" table="formation_store">		
			<key>
				<field name="eoid" relation="in" value="%{property:values}"/>
			</key>
			<sort>
				<field name="name" label="name"  direction="asc" />
			</sort>
			<fields>
				<field name="eoid" label="soid" type="ref-oid" isKey="secondary" isProp="value"/>

				<field name="bomanager" label="bomanager"  multiLines="no" type="text-choice" multiple="no" control="select" ds="formation_bo_managers.listAll" ds_format="{field:first_name} {field:last_name}" ds_field_value="user" prompt="Gestionnaire ..."  with_refresh="true" />
			</fields>		
		</view>

	CASE 2 : mode N<->N (foreign table (N recs)pointing to table (N rec) via link table)


*/

include_once(NX_LIB.'data/fields/NxDataField_text_choice.php');
include_once(NX_LIB.'NxData_RecordInputSource.inc');

/** Data field : choice list */
class NxDataField_text_choice_table extends NxDataField_text_choice
{
	function toRecords() 
	{
		$desc=&$this->desc;

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$kval=$this->record->getProperty($key,'',true,'object');
		
		// table name
		$tforeigntable = $desc->getProperty('ds','',false);
		if (!$tforeigntable)
		{
			// no link table, use foreign table
			$tforeigntable = $desc->getProperty('ds_foreign_table_get','',false);
		}

		if (!$tforeigntable)
		{
			$tjointure = $desc->getProperty('ds_link','',false);
			if (!$tjointure)
				$tjointure = $desc->getProperty('table','',false).$this->getName().'.by_key';
				
			$tforeigntable = $tjointure;
		}

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($tforeigntable,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: $tforeigntable [$media]","NxDataField_text_choice_table");
			return null;
		}
		
		// return all records in link table for that set of keys
		$errorCB=new NxErrorCB();
		$records = null;
		$ds->setProperty('key',$kval);
		$ds->setProperty('group',$this->getName());
		$records = & $ds->getRecords($view, $errorCB);		
		return $records;	
	}
	
	// function called when creating new record from a repository (such as DB)
	function onNew($oid,&$errorCB)
	{
		$this->onSave('insert',$errorCB,$oid);
		return ;
	}

	// function called when deleting a record from a repository (such as DB)
	function onDelete(&$errorCB)
	{
		$desc=&$this->desc;

//DebugBreak();

		// get value as list
		$value=trim($this->value,$desc->sep);
		if (isset($value) && $value!='')
		{
			$values = explode($desc->sep,$value);
		}
		else
			return;

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		$kval=$this->record->getProperty($key,'',true,'object');
		

		// CAS 1 - get link table if any
		if ($tjointure = $desc->getProperty('ds_link','',false))
		{
			$this->resetValueN2N($kval,$tjointure);
		}

		// CAS 2 - use foreign table and field (table.field)
		elseif($ds_foreign_table_field = $this->desc->getProperty('ds_foreign_table_field',false,false))
		{
			$multiple = $desc->getProperty('multiple',true,false);
			if($multiple)
				// 12N avec requête créée via table.fname, pas de jointure
				$this->resetValue12N($kval,$desc);
			else
				// N21 avec requête créée via table.fname, pas de jointure
				$this->resetValueN21($values,$kval,$desc);
		}

		// CAS 3 - use DML views set and reset
		elseif($tforeign_reset = $desc->getProperty('ds_foreign_table_reset','',false))
		{
			// get db data source plugin
			$media='';
			$view='';
			$ds=$this->getDS($tforeign_reset,$media,$view) ;
			if (!$ds)
			{
				nxError("Can't find datasource foreign table: [$media]","NxDataField_text_choice_table");
				return null;
			}
					
			// reset all records in foreign table
			$errorCB=new NxErrorCB();
			$records = null;
			$ds->setProperty('key',$kval);
			$ds->setProperty('fkey',$kval);
			$ds->setProperty('val','');
			$ds->setProperty('fval','');

			// get recdesc for reset
			$errorCB=new NxErrorCB();
			$recDesc=$ds->getRecDesc($view,$errorCB);
			if (!$recDesc)
				return null;

			// user fake record to execute update request
			$rec2 = new NxData_RecordInputSource($recDesc);
			$ds->putRecords($rec2,$view,$errorCB,'update');
		}
		else
			nxError('Text choice table : no valid descriptor (delete)');
	}
	
	// function adding a record
	function onSave($isInsert,&$errorCB,$oid=null)
	{
		$desc=&$this->desc;


		// get value as list
		$value=trim($this->value,$desc->sep);
		if (isset($value) && $value!='')
		{
			$values = explode($desc->sep,$value);
		}
		else
			$values = array();

		// get field key in record
		$key=$desc->getProperty('key','oid',false);
		if ($oid===null)
			$kval=$this->record->getProperty($key,null,true,'object');
		else
			$kval=$oid;

		if ($kval==null)
			return;	

		$ds_foreign_table_field = $this->desc->getProperty('ds_foreign_table_field',false,false);

		// CAS 1 - get link table if any
		if ($tjointure = $desc->getProperty('ds_link','',false))
		{
			// N2N, N21 ou 12N avec jointure
			$this->setValueN2N($values,$kval,$tjointure);
		}

		// CAS 2 - get foreign table and field (table.field) if any
		elseif($ds_foreign_table_field = $this->desc->getProperty('ds_foreign_table_field',false,false))
		{
			// sans table de jointure mais avec champ dans table foreign
			$multiple = $desc->getProperty('multiple',true,false);

			if($multiple)
				// 12N avec requête créée via table.fname, pas de jointure
				$this->setValue12N($values,$kval,$desc);
			else
				// N21 avec requête créée via table.fname, pas de jointure
				$this->setValueN21($value,$kval,$desc);
		}

		// CAS 3 - use DML views set and reset
		elseif($tforeign_reset = $desc->getProperty('ds_foreign_table_reset','',false))
		{
			// 12N avec des vues pour set, reset (pas de table de jointure)
			$this->setValue12N_withViews($values,$kval,$desc);
		}

		else
			nxError('Text choice table : no valid descriptor (onSave)');
	}
	
	// ================= MANAGE FOREIGN WITH LINK TABLE  ===================

	function setValueN2N($values,$kval,$tjointure) 
	{
		$desc = $this->desc;

		$reverse = $desc->getProperty('ds_reverse',false,false);
		if(!$reverse || $reverse=='false')
        {
            $kn = '_key';
            $kv = '_val';
        }
		else
        {
            $kn = '_val';
            $kv = '_key';
        }

		// CAS 2 : N to N - use joint table
		/* if (!$tjointure)
			$tjointure = $desc->getProperty('table','',false).$this->getName().'.by_key'; */

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($tjointure,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","NxDataField_text_choice_table");
			return null;
		}

		// get recdesc for link table
		$errorCB=new NxErrorCB();
		$recDesc=$ds->getRecDesc($view,$errorCB);
		if (!$recDesc)
			return null;

		// table name (link)
		$tlink = $recDesc->getProperty('table');
		//nxLog('values to set : '.implode(',',$values),'DS_DB');

		// store foreign objects that were linked before change (they need to be reset)
		$aForeignOids = $this->getForeignOidsN2N($ds,$tlink,$kval,$kn,$kv);
		//nxLog('get foreign objects that were linked before change : '.implode(',',$aForeignOids),'DS_DB');
				
		// delete all records in link table for that set of keys
		$this->deleteJointRecords($ds,$tlink,$kval,$kn);

		if($values)
		{
			// populate joint table
			$this->addValuesInJoint($values,$ds,$tlink,$kval,$kn,$kv);

			// get all foreign oids to set with new values and add to list of objects to reset
			$aForeignOids2 = $this->getForeignOidsN2N($ds,$tlink,$kval,$kn,$kv);
			//nxLog('get all foreign oids to set with new values and add to list of objects to reset : '.implode(',',$aForeignOids2),'DS_DB');

			// $aForeignOids = explode(',',$foreignOids)+explode(',',$foreignOids2);
			// $aForeignOids += $aForeignOids2;
			$aForeignOids = array_merge($aForeignOids,$aForeignOids2);

			//nxLog('Now list of objects to reset : '.implode(',',$aForeignOids),'DS_DB');

		}

		if(count($aForeignOids))
		{
			$aForeignOids = array_unique($aForeignOids);
			//nxLog('Now list of objects to reset (UNIQUE): '.implode(',',$aForeignOids),'DS_DB');
		}
		$foreignOids3 = implode(',',$aForeignOids);

		//nxLog('Foreign keys to reset (were before or after update): '.$foreignOids3,'DS_DB');

		// get foreign table/field
		list($table2,$_fname,$_foreignKey) = $this->getForeign_TableField();

		if($table2 && $_fname)
		{
			// set values to foreign table (built from tlink), previous and new objects
			$this->setForeignValuesN2N($foreignOids3,$table2,$_fname,$ds,$tlink,$kn,$kv);
		}
	}

	function resetValueN2N($kval,$tjointure) 
	{
		return $this->setValueN2N(null,$kval,$tjointure) ;

		/*
		$desc = $this->desc;

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($tjointure,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","NxDataField_text_choice_table");
			return null;
		}
		
		// delete all records in link table for that set of keys
		$errorCB=new NxErrorCB();
		$records = null;
		$ds->setProperty('key',$kval);
		$ds->setProperty('group',$this->getName());
		$ds->deleteRecords($view, $records, $errorCB,$ds);
		*/
	}


	/*
	function getForeignOidsN2N_old($ds,$tlink,$kval,$kn,$kv) {
		// bug : group concat has max len of 1024
		$errorCB=new NxErrorCB();

		$sql = "
			SELECT $kn, GROUP_CONCAT(DISTINCT $kv SEPARATOR ',') AS oids2
			FROM $tlink
			WHERE $kn = '$kval'
			GROUP BY $kn		
		 ";
 		 $res = $ds->query($sql,$errorCB,true);
         if($res)
		    return $res[0]['oids2'];
         else
		    return '';
	}
	*/

	function getForeignOidsN2N($ds,$tlink,$kval,$kn,$kv) {
		$errorCB=new NxErrorCB();
		$aOids = array();

		$sql = "
			SELECT $kv AS oid
			FROM $tlink
			WHERE $kn = '$kval'
		 ";
		$res = $ds->query($sql,$errorCB,true);
		if($res)
		{
			foreach($res as $rec)
			{
				$aOids[]=$rec['oid'];
			}
		}
		
		return $aOids;
	}

	function deleteJointRecords($ds,$tlink,$kval,$kn) {
		/*
		$errorCB=new NxErrorCB();
		$records = null;
		$ds->setProperty('key',$kval);
		$ds->setProperty('group',$this->getName());
		$ds->deleteRecords($view, $records, $errorCB,$ds);		
		*/

		$errorCB=new NxErrorCB();
		$sql = "
		 DELETE FROM $tlink
		 WHERE $kn = '$kval'
		 ";
		 $ds->query($sql,$errorCB);
	}

	function addValuesInJoint($values,$ds,$tlink,$kval,$kn,$kv) {
		/*
		// add values to joint table
		foreach($values as $v)
		{
			$data = array('key'=>$kval,'val'=>$v,'group'=>$this->getName());
			$rec2 = new NxData_RecordInputSource($recDesc);
			$rec2->initRecord($data, 'form', $errorCB);
			$ds->putRecords($rec2,$view,$errorCB,'insert');
		}
		*/

		if(!count($values))
			return;
		
		$errorCB=new NxErrorCB();
		$joint_records = implode("),(KEY,",$values);
		$joint_records = preg_replace('/KEY/',$kval,$joint_records);
		$sql = "
		 INSERT INTO $tlink ($kn,$kv)
		 VALUES 
			($kval,$joint_records)
		 ";
		 $ds->query($sql,$errorCB);
	}

	function setForeignValuesN2N($foreignOids3,$table2,$_fname,$ds,$tlink,$kn,$kv) 
	{
        $errorCB=new NxErrorCB();

		$sql = "
		-- update other table
		UPDATE $table2 AS T2

		-- with captured oids
		LEFT JOIN (
			SELECT $kv, CONVERT(CONCAT('|',GROUP_CONCAT(DISTINCT CONVERT($kn,char(8)) SEPARATOR '|'),'|'),char(10000)) AS oids
			FROM $tlink AS TL
			-- WHERE $kv in ($foreignOids3)
			GROUP BY $kv	
		) AS TV
		ON TV.$kv = T2._oid

		SET
			T2.$_fname = IF(TV.oids IS NULL,'',TV.oids)
		";
		
		$ds->query($sql,$errorCB);
	}

	// ================= MANAGE FOREIGN 1->N (no link table) ===================

	function setValue12N($values,$kval,$desc) 
	{
		$desc = $this->desc;

		list($table2,$_fname,$_foreignKey) = $this->getForeign_TableField();

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($table2,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","NxDataField_text_choice_table");
			return null;
		}

		// reset values on foreign table
		$this->resetForeignValues12N($kval,$table2,$_fname,$ds);

		// set new values
		$this->setForeignValues12N($kval,$values,$table2,$_fname,$_foreignKey,$ds);
	}

	function resetValue12N($kval,$desc) 
	{
		$desc = $this->desc;

		list($table2,$f_name,$_foreignKey) = $this->getForeign_TableField();

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($table2,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","NxDataField_text_choice_table");
			return null;
		}

		// reset values on foreign table
		$this->resetForeignValues12N($kval,$table2,$_fname,$ds);
	}
	
	// set value to foreign table/fname 
	function resetForeignValues12N($kval,$table2,$_fname,$ds) 
	{
        $errorCB=new NxErrorCB();       

		$sql = "
		-- reset other table
		UPDATE $table2 AS T2

		SET
			T2.$_fname = ''

		WHERE
			T2.$_fname = '$kval'
		";
		
		$ds->query($sql,$errorCB);
	}

	// set current key to foreign table.field / by foreign key
	function setForeignValues12N($kval,$aValues,$table2,$_fname,$_foreignKey,$ds) 
	{
		if(count($aValues)==0)
			return;

        $errorCB=new NxErrorCB(); 
		$values_csv = "'".implode("','",$aValues)."'";

		$sql = "
		-- update other table
		UPDATE $table2 AS T2

		SET
			T2.$_fname = '$kval'

		WHERE
			T2.$_foreignKey in ($values_csv)
		";
		
		$ds->query($sql,$errorCB);
	}


	// ================= MANAGE FOREIGN N->1 (no link table) ===================

	function setValueN21($value,$kval,$desc) 
	{
		$desc = $this->desc;

		list($table2,$_fname,$_foreignKey) = $this->getForeign_TableField();

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($table2,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","NxDataField_text_choice_table");
			return null;
		}

		// remove my key to foreign table value
		$this->removeKeyFromForeignN21($ds,$table2,$_fname,$kval);

		// add key to foreign field (ex. |6|7| )
		$this->addKeyToForeignN21($ds,$table2,$_fname,$_foreignKey,$kval,$value);
	}

	function resetValueN21($values,$kval,$desc) 
	{
		$desc = $this->desc;

		list($table2,$f_name,$_foreignKey) = $this->getForeign_TableField();

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($table2,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","NxDataField_text_choice_table");
			return null;
		}

		// remove my key to foreign table value
		$this->removeKeyFromForeignN21($ds,$table2,$f_name,$kval);
	}


	// return foreign object (key and field value)
	function getForeignOidValueN21($ds,$tlink,$kval,$kn,$kv) {
		$errorCB=new NxErrorCB();

		$sql = "
			SELECT $_foreignKey AS OID,$_foreignValue as V
			FROM $table2
			WHERE $_foreignValue LIKE '|$kval|'
		 ";
 		 $res = $ds->query($sql,$errorCB,true);
         if($res)
		    return array($res[0]['OID'],$res[0]['V']);
         else
		    return array(null,null);
	}

	function removeKeyFromForeignN21($ds,$table2,$_foreignField,$kval) {
		$errorCB=new NxErrorCB();

		$sql = "
			UPDATE $table2
			SET $_foreignField = REPLACE(REPLACE($_foreignField,'|$kval|','|'),'||','')
			WHERE $_foreignField LIKE '%|$kval|%'
		 ";
 		 $res = $ds->query($sql,$errorCB,false);
	}
	function addKeyToForeignN21($ds,$table2,$_foreignField,$_foreignKey,$kval,$value) {
		$errorCB=new NxErrorCB();

		$sql = "
			UPDATE $table2
			SET $_foreignField = REPLACE(CONCAT($_foreignField,'|$kval|'),'||','|')
			WHERE $_foreignKey = '$value'
		 ";
 		 $res = $ds->query($sql,$errorCB,false);
	}

/*0
	function setForeignField12N($v,$ds,$tlink,$kval,$kn,$kv) {
		$errorCB=new NxErrorCB();

		$sql = "
			UPDATE
			SELECT $kn, GROUP_CONCAT(DISTINCT $kv SEPARATOR ',') AS oids2
			FROM $tlink
			WHERE $kn = '$kval'
			GROUP BY $kn		
		 ";
 		 $res = $ds->query($sql,$errorCB,true);
         if($res)
		    return $res[0]['oids2'];
         else
		    return '';
	}
*/
	
	// ================= MANAGE FOREIGN WITH VIEWS ===================

	function setValue12N_withViews($values,$kval,$desc) 
	{
		$tforeign_reset = $desc->getProperty('ds_foreign_table_reset','',false);
		$tforeign_set = $desc->getProperty('ds_foreign_table_set','',false);
		if(!$tforeign_reset || !$tforeign_set)
		{
			nxError("no valid attributes: [ds_foreign_table_reset,ds_foreign_table_set or ds_link]","NxDataField_text_choice_table");
			return null;
		}

		// get db data source plugin
		$media='';
		$view='';
		$ds=$this->getDS($tforeign_reset,$media,$view) ;
		if (!$ds)
		{
			nxError("Can't find datasource foreign table: [$media]","NxDataField_text_choice_table");
			return null;
		}
				
		// reset all records in foreign table
		$errorCB=new NxErrorCB();
		$records = null;
		$ds->setProperty('key',$kval);
		$ds->setProperty('fkey',$kval);
		$ds->setProperty('val','');
		$ds->setProperty('fval','');

		// get recdesc for reset
		$errorCB=new NxErrorCB();
		$recDesc=$ds->getRecDesc($view,$errorCB);
		if (!$recDesc)
			return null;

		// user fake record to execute update request
		$rec2 = new NxData_RecordInputSource($recDesc);
		$ds->putRecords($rec2,$view,$errorCB,'update');

		// now set values
		$ds=$this->getDS($tforeign_set,$media,$view) ;
		$recDesc=$ds->getRecDesc($view,$errorCB);
		$rec2 = new NxData_RecordInputSource($recDesc);

		$newValues = implode("','",$values);

		$ds->setProperty('val',$newValues);
		$ds->setProperty('values',$newValues);
		$ds->putRecords($rec2,$view,$errorCB,'update');
	}



	// ================ UTILITIES ===================
	
	function getDS($attribute,&$media,&$view) 
	{			
		// get ds name
		if (($dsn=$this->desc->getProperty($attribute,null,false))==null)
			$dsn = $attribute;
			
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
		return $this->_getDS($media);
	}

	function _getDS($media='db')
	{
		// get db data source plugin		
		$ds =& $this->desc->getPlugIn(strtolower($media),'NxDS_','ds');
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","field_text_choice_table");
			return null;
		}
		
		return $ds;
	}


	// get foreign table/fname 
	function getForeign_TableField($ds=null)
	{
        $errorCB=new NxErrorCB();       
		
		$ds_foreign_table_field = $this->desc->getProperty('ds_foreign_table_field',false,false);
		$foreignkey = $this->desc->getProperty('ds_field_value','oid',false);
		if($ds_foreign_table_field)
		{
			list($table2,$fname) = explode('.',$ds_foreign_table_field);
			$ds = $this->_getDS();

			$recDesc2=$ds->getRecDesc($table2,$errorCB);
			$table2 = $recDesc2->getProperty('table');
			$fdesc = $recDesc2->getFieldDesc($fname);
			$_fname = $fdesc->getProperty('ufname');

			$fdesc = $recDesc2->getFieldDesc($foreignkey);
			$_foreignkey = $fdesc->getProperty('ufname');
		}
		else
		{
			$tforeign_reset = $this->desc->getProperty('ds_foreign_table_reset','',false);

			// update other table with enum values
			if(!$tforeign_set)
				return false;

			// get foreign table name
			$recDesc2=$ds->getRecDesc($tforeign_set,$errorCB);
			if (!$recDesc2)
				return;
			$table2 = $recDesc2->getProperty('table');

			// get foreign field name
			$fkeys = array_keys($recDesc2->fdescs);
			foreach($fkeys as $fname)
			{
				if($fname != 'oid')
					break;
			}

			$fdesc = $recDesc2->getFieldDesc($fname);
			$_fname = $fdesc->getProperty('ufname');

			$_foreignkey='_'.$ds_foreign_key;
		}

		return array($table2,$_fname,$_foreignkey);
	}
}

?>
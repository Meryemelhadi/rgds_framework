<?php 

// library for dealing with array of records: sort etc.
class NxRecords_tools
{
	// move a record up in an list of records: change order of record in list and setup indexes
	// returns array of modified records
	function move_up($records,$key_fname,$key_fvalue,$index_name,$index_type='int')
	{
		// get records as a record array	
		$recs2=$records->getRecordArray();
		
		// search record to move
		$len=$recs2->count;
		for($i=0; $i<$len;$i++)
		{
			if ($recs2->records[$i]->getFieldValue($key_fname,'string',null)==$key_fvalue)
				break;
		}
			
		// if found and not at top, change orders
		if ($i!=0 && $i!=$len)
		{
			// found it, now swap records..
			$rec=$recs2->records[$i];
			$recs2->records[$i]=$recs2->records[$i-1];
			$recs2->records[$i-1]=$rec;
		}
		
		// array sorted, now set up indexes that have changed
		return NxRecords_tools::reindex($recs2,$index_name,$index_type);
	}

	// move a record down in an list of records: change order of record in list and setup indexes
	// returns array of modified records
	function move_down($records,$key_fname,$key_fvalue,$index_name,$index_type='int')
	{
		// get records as a record array	
		$recs2=$records->getRecordArray();
		
		// search record to move
		$len=$recs2->count;
		for($i=0; $i<$len;$i++)
		{
			if ($recs2->records[$i]->getFieldValue($key_fname,'string',null)==$key_fvalue)
				break;
		}

		// if found and not at bottom already, change orders
		if ($i < $len-1)
		{
			// found it, now swap records..
			$rec=$recs2->records[$i];
			$recs2->records[$i]=$recs2->records[$i+1];
			$recs2->records[$i+1]=$rec;
		}
			
		
		// array sorted, now set up indexes that have changed
		// return array of modified records
		return NxRecords_tools::reindex($recs2,$index_name,$index_type);
	}
	
	function reindex($recs2,$index_name,$index_type='int') {
		// array sorted, now set up indexes that have changed
		$sort_arr=array();
		$len=$recs2->count;
		if ($index_type=='int')
			// integer sort
			for($i=0; $i<$len;$i++)
			{
				$rec=$recs2->records[$i];
				if ($rec->getFieldValue($index_name,'string',null)!="$i")
				{
					// change index and store record for update
					$rec->setField($index_name,$i);
					$sort_arr[]=$rec;
				}
			}
		else
			// alpha sort
			for($i=0; $i<$len;$i++)
			{
				// works for indexes between 0 and 63
				$index=(($i<10)?$i:(($i<37)?chr(ord('a')+($i-10)):(chr(ord('A')+($i-37)))));
				
				$rec=$recs2->records[$i];
				if ($rec->getFieldValue($index_name,'string',null)!="$index")
				{
					// change index and store record for update
					$rec->setField($index_name,"$index");
					$sort_arr[]=$rec;
				}
			}
			
		// returns a new array of records
		// require_once(NX_LIB."NxData_ArrayRecords.inc");
		return new NxData_ArrayRecords($sort_arr,$rec->getRecDesc());
	}
}

?>
<?PHP

include_once(NX_TAG_LIB.'nxfrontier/xo/core/taglib_nxfrontier_xo_core.inc');

// cms article library
class taglib_nxfrontier_cms_manage_nav extends taglib_nxfrontier_xo_core
{

	/*
	<move_up records="#var:records#" key="#$aid#" 
		xmlns:nxl="http://www.nxfrontier.com/cms/manage_nav"/>

	*/
	function move_up($node,$content,$level,$pad)
	{
		// get records
		if (!$this->getAttribute($records,$node,'records',null,true))
			return $this->fatal('no records attribute');
		
		// get key value of article to move
		if (!$this->getAttribute($key_fvalue,$node,'key_fvalue',null,true,true)!=null)
			return $this->fatal('no key_fvalue attribute');

		// get key value of article to move
		if (!$this->getAttribute($key_fname,$node,'key_fname',null,true,true)!=null)
			return $this->fatal('no key_fname attribute');
			
		// get field name for article index
		if (!$this->getAttribute($index_fname,$node,'index_fname',null,true,true)!=null)
			return $this->fatal('no index_fname attribute');
			
		// request library load
		$this->context->addInclude('data/records/NxRecords_tools.inc');

		return "\n{$pad}NxRecords_tools::move_up($records,$key_fname,$key_fvalue,$index_fname)";
	}
	
	/*
	<move_down records="#var:records#" key="#$aid#" 
		xmlns:nxl="http://www.nxfrontier.com/cms/manage_nav"/>

	*/
	function move_down($node,$content,$level,$pad)
	{
		// get records
		if (!$this->getAttribute($records,$node,'records',null,true))
			return $this->fatal('no records attribute');
		
		// get key value of article to move
		if (!$this->getAttribute($key_fvalue,$node,'key_fvalue',null,true,true)!=null)
			return $this->fatal('no key_fvalue attribute');

		// get key value of article to move
		if (!$this->getAttribute($key_fname,$node,'key_fname',null,true,true)!=null)
			return $this->fatal('no key_fname attribute');
			
		// get field name for article index
		if (!$this->getAttribute($index_fname,$node,'index_fname',null,true,true)!=null)
			return $this->fatal('no index_fname attribute');
			
		// request library load
		$this->context->addInclude('data/records/NxRecords_tools.inc');

		return "\n{$pad}NxRecords_tools::move_down($records,$key_fname,$key_fvalue,$index_fname)";
	}
}

?>
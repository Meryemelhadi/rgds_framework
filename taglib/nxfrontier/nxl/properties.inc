<?PHP

// core library for nxl language
class taglib_nxfrontier_nxl_properties extends NxTagLib
{
	function header()
	{
		return "// import library session";
	}
	
	function listAll($node,$content,$level,$pad)
	{	
		$this->getAttribute($ns,$node,'ns','http://www.nxfrontier.com/nxl/properties/map');

		$bag=array();
		$props=$this->props;
		while($props)
		{
			$bag += $props->getProperties();
			$props=$props->parent;
		}

		ksort($bag);
		
		$s='<properties xmlns="'.$ns.'">';
		foreach ($bag as $k=>$v)
		{
			$s .= '<property name="'.$k.'">'.$v."</property>\n";
		}
		$s.='</properties>';

		$res=$this->context->processFragmentSource($s,'',0,null,true);
		return $res;
	}
}

?>

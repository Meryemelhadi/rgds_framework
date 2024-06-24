<?php

class maps_geocode_uk
{
	function postcode_2_Coord($postcode,&$long,&$lat) {
		
		$apiURL = "http://brainoff.com/geocoder/rest/?post=";
		$locXML = file_get_contents($apiURL.urlencode($postcode));
		
		$_lat=$_long=null;
		preg_replaceX(
			array('/<geo:long>([0-9-+.]+</geo:long>/e',
				'/<geo:lat>([0-9-+.]+</geo:lat>/e'),
			array('$_long="$1"','$_lat="$1"'),
			$locXML);
		
		return ($_long !==null && $_lat!==null);
	}
	
}

?>
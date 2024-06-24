<?php

class maps_geocode_uk
{
	function postcode_2_Coord($postcode,&$long,&$lat) {
		
		$apiURL = "http://brainoff.com/geocoder/rest/?post=";
		$locXML = file_get_contents($apiURL.urlencode($postcode));
		
		$this->_lat=$this->_long=null;
		preg_replaceX(
			array('#<geo:long>([0-9-+.]+)</geo:long>#e',
				'#<geo:lat>([0-9-+.]+)</geo:lat>#e'),
			array('$this->_long="$1"','$this->_lat="$1"'),
			$locXML,$this);
			
		if ($this->_long !=null && $this->_lat!=null)
		{
			$long=$this->_long;
			$lat=$this->_lat;
			return true;
		}
		
		return false;
	}

	function postcode_2_Coord2_1($postcode,&$long,&$lat) {
		
		include_once('SOAP/Client.php');
		
		putenv('WSDLCACHE='.NX_GEN.'soap/');
		
		$objWsdl = new SOAP_WSDL($url='http://www.discoverdance.co.uk/UKCoordinates/GeographicalCoord.asmx?wsdl',array(),
		true); 
		$objClient = $objWsdl->getProxy(); 
	
		$res = $objClient->GetUKCoord($postcode);
		if (is_string($res))
		{
			list($_long,$_lat)=explode(',',$res);
			if ($_long!=9999)
			{
				$long=$_long;
				$lat=$_lat;
				return true;
			}
		}
		else
		{
			nxError("SOAP error in $url : ".$res);
		}
		
		return false;
	}

	function postcode_2_Coord2($postcode,&$long,&$lat) {
		
		$apiURL = "http://geo.localsearchmaps.com/?country=UK&use=google2&format=xml&zip=";
		$locXML = file_get_contents($apiURL.urlencode($postcode));
		
		$this->_lat=$this->_long=null;
		preg_replaceX(
			array('#<longitude>([0-9-+.]+)</longitude>#e',
				'#<latitude>([0-9-+.]+)</latitude>#e'),
			array('$this->_long="$1"','$this->_lat="$1"'),
			$locXML,$this);
			
		if ($this->_long !=null && $this->_lat!=null)
		{
			$long=$this->_long;
			$lat=$this->_lat;
			return true;
		}
		
		return  maps_geocode_uk::postcode_2_Coord2_1($postcode,$long,$lat)|| maps_geocode_uk::postcode_2_Coord($postcode,&$long,&$lat);
	}

	
}

?>
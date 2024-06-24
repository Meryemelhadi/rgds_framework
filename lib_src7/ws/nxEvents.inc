<?php

class NxEvents
{
	static function fireEvent($event,$subject,$body,$details=null,$cb=null,$routePresence=false,$routeOthers=true) {
		$iftt=$routeOthers && (defined('IFTT_KEY') && IFTT_KEY);
		$integromat=$routeOthers && (defined('INTEGROMAT_KEY') && INTEGROMAT_KEY);
		$presence=$routePresence && (defined('PRESENCE_EVENT_KEY') && PRESENCE_EVENT_KEY);

		if(!($iftt || $integromat || $presence))
			return false;
	
		if($presence)
			$files = array();
		else
			$files = null;

		self::addDetails($details,$event);
		if($cb)
		{
			$cb($event,$subject,$body,$details,$files);
		}

		if($presence)
			self::fireEvent_Presence(PRESENCE_EVENT_KEY,$event,$subject,$body,$details,$files);

		if($iftt)
			self::fireEvent_IFTT(IFTT_KEY,$event,$subject,$body,$details);

		if($integromat)
			self::fireEvent_integromat(INTEGROMAT_KEY,$event,$subject,$body,$details);

		return true;
	}

	static function encodeFile($filePath) {
		$filesB64 = null;
		if(is_file($filePath))
		{
			$fs=Framework::fs();
			$fileContent=$fs->file_get_contents($filePath,'rb');
			$b64fileContent=base64_encode($fileContent);
			$filesB64 = array(
				'name'=>base64_encode(basename($filePath)),
				'src'=>base64_encode($fileContent)
			);
		}

		return $filesB64;
	}

	static function decodeFile($fileDesc) {
		if(isset($fileDesc['name']) && isset($fileDesc['src']))
		{
			return array(
				'name'=>base64_decode($fileDesc['name']),
				'src'=>base64_decode($fileDesc['src']),
			);
		}
		return null;
	}

	static function fireEventKeys($keys,$event,$subject,$body,$details=null,$cb=null) {
		self::addDetails($details,$event);
		if($cb)
		{
			$files = array();
			$cb($event,$subject,$body,$details,$files);
		}

 		if(isset($keys['IFTT_KEY']) && $keys['IFTT_KEY'])
			self::fireEvent_IFTT($keys['IFTT_KEY'],$event,$subject,$body,$details);

 		if(isset($keys['INTEGROMAT_KEY']) && $keys['INTEGROMAT_KEY'])
			self::fireEvent_integromat($keys['INTEGROMAT_KEY'],$event,$subject,$body,$details);

 		if(isset($keys['PRESENCE_KEY']) && $keys['PRESENCE_KEY'])
			self::fireEvent_Presence($keys['PRESENCE_KEY'],$event,$subject,$body,$details,$files);
		else if(isset($keys['PRESENCE_KEY_URL']) && $keys['PRESENCE_KEY_URL'])
			self::fireEvent_Presence($keys['PRESENCE_KEY_URL'],$event,$subject,$body,$details,$files);
	}

	static function addDetails(&$details,$event) {
		if(is_string($details))
		{
			if($details)
				$details = array('details'=>$details);
			else
				$details = array();
		}

		if(!isset($details['event']))
			$details['event']=$event;

		$details['date_event']=date("Y-m-d H:i:s");
		if(isset($GLOBALS['_NX_user']))
		{
			$details['user']=$GLOBALS['_NX_user']->getProperty('user.fullName','',false);
		}
	}

	static function fireEvent_IFTT($key,$event,$subject,$body,$details=null,$cb=null) {
		nxLogStart('Fire Event IFTTT '.$event,'EVENT');
	
		if(count($aev = explode('@',$event))>1)
		{
			$event = $aev[1].'_'.$aev[0];
		}

		$url = 	"https://maker.ifttt.com/trigger/{$event}/with/key/".$key;

		$data = array(
			'value1' => $subject,  // subject
			'value2' => $body,
			'value3'=>json_encode($details,is_string(JSON_PRETTY_PRINT)?128:JSON_PRETTY_PRINT)
		);

		/*
		$data = array(
			'value1' => 'test value 1',  // subject
			'value2' => 'test value 2',
			'value3'=>'test value 3'
		);
		*/

		self::postRestJson($url,$data,false);

		nxLogStop('Fire Event IFTTT '.$event,'EVENT');
	}

	static function fireEvent_integromat($key,$event,$subject,$body,$details=null,$cb=null) {
		nxLogStart('Fire Event Integromat '.$event,'EVENT');

		if($aev = explode('@',$event)>1)
		{
			$event = $aev[1].'_'.$aev[0];
		}

		$key2 = 'INTEGROMAT_KEY_'.$event;
 		if(defined($key2) && constant($key2))
		{
			$key = constant($key2);
		}
	
		$url = 	"https://hook.integromat.com/".$key;

		$data = $details + array(
			'value1' => $subject,  // subject
			'value2' => $body,
			'value3'=>json_encode($details,true)
			);

		self::postRestJson($url,$data);

		nxLogStop('Fire Event Integromat'.$event,'EVENT');
	}

	static function fireEvent_Presence($key,$event,$subject,$body,$details=null,$files) {
		nxLogStart('Fire Event Presence '.$event,'EVENT');

		if(substr( $key, 0, 4 ) === "http")
			$url = $key;
		else
		{
			$url = 'PRESENCE_EVENT_URL_'.$key;
			if(defined($url) && constant($url))
			{
				$url = constant($url);
			}
			else
		{
				$url = self::getPresenceURL();
			}
		}

		$url .= "&ev={$event}";

		$data = $details + array(
			'value1' => $subject,  // subject
			'value2' => $body,
			);

		if($files && count($files))
			$data['files'] = $files;

		self::postRestJson($url,$data,true);
		nxLogStop('Fire Event Presence '.$event,'EVENT');
	}

	static function getPresenceURL() {
		$ctxt = Framework::getContext();
		require_once(NX_LIB."login/NxLogin_keySite.inc");
		$login=new NxLogin_keySite($ctxt,null,false);
		$mkey = $login->getCurrentParamKey();

		return $_SERVER['HTTP_SITE_URL']."/workflow/event.php?{$mkey}";
	}

	// sending HTTP FORM
	static function postRestData($url, $params)
	{
		foreach ($params as $key => &$val) {
		  if (is_array($val)) $val = implode(',', $val);
			$post_params[] = $key.'='.urlencode($val);
		}
		$post_string = implode('&', $post_params);

		self::postRestAsync($url, $post_string,'application/x-www-form-urlencoded');
	}

	// sending REST
	static function postRestJson($url,$data,$async=false) {
		$data_string = json_encode($data);
		if($async)
			self::postRestAsync($url,$data_string,$ctype='application/json');
		else
			self::postRestSync($url,$data_string,$ctype='application/json');
	}

	static function postRestSync($url,$data_string,$ctype='application/json') {
	
		$ch = curl_init($url);

		curl_setopt($ch, CURLOPT_CUSTOMREQUEST, "POST");
		curl_setopt($ch, CURLOPT_POST, 1);
		curl_setopt($ch, CURLOPT_POSTFIELDS, $data_string);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_HEADER, true);
		curl_setopt($ch, CURLOPT_HTTPHEADER, array(
			"Content-Type:$ctype",
			'Content-Length: ' . strlen($data_string))
		);
		// curl_setopt($ch, CURLOPT_TIMEOUT, 5);
		curl_setopt($ch, CURLOPT_FRESH_CONNECT, true);
		curl_setopt($ch, CURLOPT_TIMEOUT, 2);

		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 5);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);

		//execute post
		$result = curl_exec($ch);
		if($result===false)
		{
			$error = curl_error($ch);
			nxError($error);
		}

		//close connection
		curl_close($ch);
	}

	static function postRestAsync($url, $post_string,$ctype='application/json')
	{
		$parts=parse_url($url);

		$fp = fsockopen($parts['host'], 
			isset($parts['port'])?$parts['port']:80, 
			$errno, $errstr, 30);

		$out = "POST ".$parts['path'].((isset($parts['query'])) ? "?" . $parts['query'] : '') ." HTTP/1.1\r\n";
		$out.= "Host: ".$parts['host']."\r\n";
		$out.= "Content-Type: $ctype\r\n";
		$out.= "Content-Length: ".strlen($post_string)."\r\n";
		$out.= "Connection: Close\r\n\r\n";
		if (isset($post_string)) $out.= $post_string;

		fwrite($fp, $out);
		fclose($fp);
	}
}

?>
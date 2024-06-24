<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// manages caching of pages with:
// - gzip encoding when browser supports it (at no CPU cost as the gz version is stored on disc),
// - plain text caching of page otherwise,
// - HTTP 1.0 caching $this->headers with expiration dates,
// - HTTP 1.1 Etag $this->header (Etag is a unique content id that should change when the content is updated).
class NxCache_Page
{
	var $props;
	var $resID;
	
//	function __construct(&$props,$resID=null,$gzip=true)
	function __construct(&$props,$gzip=true)
	{
		global $NX_LANG;
		
		if (!defined('NX_HTTP_Cache') || NX_HTTP_Cache!=true)
			return;
			
		$this->props  =& $props;
		
		/*if($resID!=null)
			$this->resID  =& $resID;		
		else
			$this->resID  = md5($_SERVER['REQUEST_URI']);
			$this->resID .= "__{$NX_LANG}_".NX_DEVICE_MAP;
		*/
		if (isset($GLOBALS['NX_CACHE_ID']))
			$this->resID  = $GLOBALS['NX_CACHE_ID'];		
		else
		{
			$this->resID  = md5($_SERVER['REQUEST_URI']);
			$this->resID .= "__{$NX_LANG}_".NX_DEVICE_MAP;
		}
					
		// enable gzip if client supports it
		if ($gzip && isset($_SERVER["HTTP_ACCEPT_ENCODING"]) && 
			strpos($_SERVER["HTTP_ACCEPT_ENCODING"],"gzip")!==false && extension_loaded("zlib"))
			$this->gzip = true;
		else
			$this->gzip = false;
			//$this->gzip = true;
	}
	
	// try to load the cache from a file or start buffering. 
	// Returns true if could get the cache or false otherwise.
	function loadCache($duration=null)
	{
		if (!defined('NX_HTTP_Cache') || NX_HTTP_Cache!=true)
			return false;
			
		if ($duration === null)
			if (isset($_GET["CACHE_TIME_OUT"]))
				$duration = $_GET["CACHE_TIME_OUT"];
			else
				$duration = $this->props->getProperty("page.cache.timeout","60");

		$this->fpath = $GLOBALS['NX_CACHE'].$this->props->getProperty('page.cache.path','').$this->resID;
		
		if ($this->gzip)
			$this->fpath .= '.gz';
		
		if( is_file($this->fpath) &&
			($ftime = @filemtime($this->fpath)) && 
			($expires = ($ftime+$duration)-time())>0 && 
			($size=filesize( $this->fpath)) &&
			!isset($_GET['nocache']))
		{
			// reuse cache ?
			
			// set http cache $this->headers (for web server, proxies,browser caching)
			$this->sendHTTPCacheHeaders($ftime,$expires);
			
			// zip optimisation
			if ($this->gzip)
			{
				// send gzip $this->header	(tbd. deflate or x-gzip ?)
				$this->header('Content-Encoding: gzip');
				$this->header('Vary: Accept-Encoding');
			}
			$this->header("Content-Length: $size");		
			
			// validate cached file if requested by proxy cache
			if ($this->check304($this->resID, $ftime)) 
			{
				$this->header('HTTP/1.0 304');
				die();
			}
			else {
				// output cached file
				$fp=fopen($this->fpath, "rb");
				fpassthru($fp);
				nxLog('send cache file:'.$this->fpath);
			}
			
			return true;
		}
		else
		{
			// create cache
			
			// send $this->headers
			$this->sendHTTPCacheHeaders(time(),$duration);
			// $this->gzip=false; // debug
			// open buffer
			if ($this->gzip)
			{
				ob_start();
				ob_start("ob_gzhandler");
				nxLog('ob_gzhandler','CACHE');
			}
			else
			{
				ob_start();
				nxLog('ob_start','CACHE');
			}
				
			ignore_user_abort(true);
			return false;
		}
	}
	
	function check304($etag, $lastModified) 
	{
		if (defined('NX_HTTP_304') && NX_HTTP_304)
		{
			nxLog("check 304:etag[$etag] last modif[$lastModified]",'CACHE');
			if (isset($_SERVER["HTTP_IF_NONE_MATCH"])) 
			{
				//nxLog("HTTP_IF_NONE_MATCH={$_SERVER["HTTP_IF_NONE_MATCH"]}",'CACHE');
				if ($etag == stripslashes($_SERVER["HTTP_IF_NONE_MATCH"])) 
					return true;
			}
			else if (isset($_SERVER["HTTP_IF_MODIFIED_SINCE"]))
			{
				//nxLog("HTTP_IF_MODIFIED_SINCE={$_SERVER["HTTP_IF_MODIFIED_SINCE"]}",'CACHE');
				if (strtotime($lastModified) <= strtotime($_SERVER["HTTP_IF_MODIFIED_SINCE"])) 
					return true;
			}
		}
		
		nxLog("check 304: not 304 => normal processing",'CACHE');
		return false;
	}


	function sendHTTPCacheheaders($modified,$expires)
	{
		// $expires = (function_exists('session_cache_expire') ? session_cache_expire() : 60 ) * 60;
		$gm_expires = gmdate('D, d M Y H:i:s', time()+$expires);
//			$this->header("Cache-control: private, max-age=$expires, pre-check=$expires"); 
		$this->header("Cache-control: public, max-age=$expires, pre-check=$expires"); 
		$this->header("Expires: $gm_expires GMT");
		
		// timed validator (NB. require If-Modified-Since protocol) HTTP 1.0
		$this->header("Last-Modified: " . gmdate("D, d M Y H:i:s",$modified) . " GMT");
		
		// etag validator : unique id for the page (HTTP 1.1)
		$this->header('ETag:'.$this->resID);
	}
	
	// get buffer content, update cache, and output to property or screen.
	function endCache()
	{
		$fs=$this->props->getPlugIn('FileSys');
		
		if (!defined("NX_HTTP_Cache") || NX_HTTP_Cache!=true)
			return;
		
		// open buffer
		if ($this->gzip)
		{
			ob_end_flush();
			// NB. ob_end_flush() automatically sends Content-Length $this->header here...
			
			$src=ob_get_contents();
			nxLog('ob_end_flush(zip+content len header) + get contents','CACHE');
		}
		else
		{
			$this->header("Content-Length: ".strlen($src=ob_get_contents()));
		}
		
		// save content
		if ($fp = @$fs->fopen($this->fpath,'wb+',true))
		{
			fputs($fp,$src);
			fclose($fp);
			nxLog('save file:'.$this->fpath,'CACHE');
		}
		
		ob_end_flush();
		nxLog('ob_end_flush','CACHE');
	}
	function header($h)
	{
		nxLog("header:$h",'CACHE');
		header($h);
	}
}

?>
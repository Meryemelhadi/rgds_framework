<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// manages caching of pages with:
// - gzip encoding when browser supports it (at no CPU cost as the gz version is stored on disc),
// - plain text caching of page otherwise,
// - HTTP 1.0 caching headers with expiration dates,
// - HTTP 1.1 Etag header (Etag is a unique content id that should change when the content is updated).
class NxCache_Page
{
	var $props;
	var $resID;
	
	function NxCache_Page(&$props,$resID=null,$gzip=true)
	{
		global $NX_LANG;
		
		if (!defined("NX_HTTP_Cache") || NX_HTTP_Cache!=true)
			return;
			
		$this->props  =& $props;
		
		if($resID!=null)
			$this->resID  =& $resID;		
		else
			$this->resID  = md5($_SERVER['REQUEST_URI']);
			
		$this->resID .= "__{$NX_LANG}_".NX_DEVICE_MAP;
		
		// enable gzip if client supports it
		if ($gzip && isset($_SERVER["HTTP_ACCEPT_ENCODING"]) && strpos($_SERVER["HTTP_ACCEPT_ENCODING"],"gzip")!==false && extension_loaded("zlib"))
			$this->gzip = true;
		else
			$this->gzip = false;
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
		
		$this->fpath = NX_CACHE.$this->resID;
		if ($this->gzip)
			$this->fpath .= ".gz";
		
		if( is_file($this->fpath) &&
			($ftime = @filemtime($this->fpath)) && 
			($expires = ($ftime+$duration)-time())>0 && 
			($size=filesize( $this->fpath)) &&
			!isset($_GET["nocache"]))
		{
			// reuse cache ?
			
			// validate cached file if requested by proxy cache
			if ($this->check304($this->resID, $ftime)) 
			{
				header( 'HTTP/1.1 304 Not Modified' );
				die();
			}
			
			// set http cache headers (for web server, proxies,browser caching)
			$this->sendHTTPCacheHeaders($ftime,$expires);
			
			// zip optimisation
			if ($this->gzip)
			{
				// send gzip header	(tbd. deflate or x-gzip ?)
				header("Content-Encoding: gzip");
				header("Vary: Accept-Encoding");
			}
			header("Content-Length: ".$size);		
			
			// output cached file
			$fp=fopen($this->fpath, "rb");
			fpassthru($fp);
			// readfile();				
			return true;
		}
		else
		{
			// create cache
			
			// send headers
			$this->sendHTTPCacheHeaders(time(),$duration);
			// $this->gzip=false; // debug
			// open buffer
			if ($this->gzip)
			{
				ob_start();
				ob_start("ob_gzhandler");
			}
			else
				ob_start();
				
			ignore_user_abort(true);
			return false;
		}
	}
	
	function check304($etag, $lastModified) 
	{
		if (isset($_SERVER["HTTP_IF_NONE_MATCH"])) 
		{
			if ($etag == stripslashes($_SERVER["HTTP_IF_NONE_MATCH"])) 
				return true;
		}
		else if (isset($_SERVER["HTTP_IF_MODIFIED_SINCE"]))
		{
			if (strtotime($lastModified) <= strtotime($_SERVER["HTTP_IF_MODIFIED_SINCE"])) 
				return true;
		}
		return false;
	}


	function sendHTTPCacheHeaders($modified,$expires)
	{
		// $expires = (function_exists('session_cache_expire') ? session_cache_expire() : 60 ) * 60;
		$gm_expires = gmdate('D, d M Y H:i:s', time()+$expires);
//			header("Cache-control: private, max-age=$expires, pre-check=$expires"); 
		header("Cache-control: public, max-age=$expires, pre-check=$expires"); 
		header("Expires: $gm_expires GMT");
		
		// timed validator (NB. require If-Modified-Since protocol) HTTP 1.0
		header("Last-Modified: " . gmdate("D, d M Y H:i:s",$modified) . " GMT");
		
		// etag validator : unique id for the page (HTTP 1.1)
		header('ETag:'.$this->resID);
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
			// NB. ob_end_flush() automatically sends Content-Length header here...
			
			$src=ob_get_contents();
		}
		else
		{
			header("Content-Length: ".strlen($src=ob_get_contents()));
		}
		
		// save content
		if ($fp = @$fs->fopen($this->fpath,'wb+',true))
		{
			fputs($fp,$src);
			fclose($fp);
		}
		
		ob_end_flush();
	}
}

?>
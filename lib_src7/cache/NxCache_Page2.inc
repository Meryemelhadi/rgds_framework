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
class NxCache_Page2
{	
	var $gzip,$file;
	
	function load($dir,$resID,$duration,$gzip=true)
	{				
		// enable gzip if client supports it
		if ($gzip && isset($_SERVER["HTTP_ACCEPT_ENCODING"]) && 
			strpos($_SERVER["HTTP_ACCEPT_ENCODING"],"gzip")!==false && extension_loaded("zlib"))
			$this->gzip = true;
		else
			$this->gzip = false;
	
		$this->fpath = $dir.$resID;		
		if ($this->gzip)
			$this->fpath .= '.gz';		
		
		if( is_file($this->fpath) &&
			($ftime = @filemtime($this->fpath)) && 
			($size=filesize( $this->fpath)))
		{
			// reuse cache 
			
			$expires = ($ftime+$duration)-time();
			
			// set http cache headers (for web server, proxies,browser caching)
			$this->sendHTTPCacheHeaders($resID,$ftime,$expires);
			
			// zip optimisation
			if ($this->gzip)
			{
				// send gzip header	(tbd. deflate or x-gzip ?)
				header('Content-Encoding: gzip');
				header('Vary: Accept-Encoding');
			}

			// page enveloppe: add content type and charset
			global $NX_MIME,$NX_CHARSET;
			header("Content-Type: {$NX_MIME}; charset={$NX_CHARSET}"); 

			// header("Content-Length: $size");		
			
			// validate cached file if requested by proxy cache
			if ($this->check304($resID, $ftime)) 
			{
				header('HTTP/1.0 304');
				die();
			}
			else {
				// output cached file
				$fp=fopen($this->fpath, "rb");
				fpassthru($fp);
			}
			
			return true;
		}
		else
		{
			// create cache
			
			// send headers
			$this->sendHTTPCacheHeaders($resID,time(),$duration);
			
			// open buffer
			ob_start();
			if ($this->gzip)
				ob_start("ob_gzhandler");
				
			ignore_user_abort(true);
			return false;
		}
	}

	// get buffer content, update cache, and output to property or screen.
	function endCache($cancel=false)
	{
		global $_NX_site;
		$fs=$_NX_site->getPlugIn('FileSys');
		
		// open buffer
		if ($this->gzip)
		{
			ob_end_flush();

			header('Content-Length: '.ob_get_length());
			$src=ob_get_contents();
		}
		else
		{
			header("Content-Length: ".strlen($src=ob_get_contents()));
		}
		
		if ($cancel!==true && $cancel!=='true')
		{
			// save content
			if ($fp = @$fs->fopen($this->fpath,'wb+',true))
			{
				fputs($fp,$src);
				fclose($fp);
			}
		}
		
		ob_end_flush();
	}
	
	function check304($etag, $lastModified) 
	{
		if (defined('NX_HTTP_304') && NX_HTTP_304)
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
		}
		
		return false;
	}


	function sendHTTPCacheheaders($resID,$modified,$expires)
	{
		// $expires = (function_exists('session_cache_expire') ? session_cache_expire() : 60 ) * 60;
		$gm_expires = gmdate('D, d M Y H:i:s', time()+$expires);
//			header("Cache-control: private, max-age=$expires, pre-check=$expires"); 
		header("Cache-control: public, max-age=$expires, pre-check=$expires"); 
		header("Expires: $gm_expires GMT");
		
		// timed validator (NB. require If-Modified-Since protocol) HTTP 1.0
		header("Last-Modified: " . gmdate("D, d M Y H:i:s",$modified) . " GMT");
		
		// etag validator : unique id for the page (HTTP 1.1)
		header('ETag: "'.$resID.'"');
	}
}

?>
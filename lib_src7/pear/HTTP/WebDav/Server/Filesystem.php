<?php

    require_once NX_PEAR."HTTP/WebDav/Server.php";
    require_once NX_PEAR."System.php";
    
    /**
     * Filesystem access using WebDAV
     *
     * @access public
     */
    class HTTP_WebDAV_Server_Filesystem extends HTTP_WebDAV_Server 
    {
        /**
         * Root directory for WebDAV access
         *
         * Defaults to webserver document root (set by ServeRequest)
         *
         * @access private
         * @var    string
         */
        var $base_dir;
                
        /**
         * MYSQL params
         *
         *
         * @access private
         * @var    string
         */
        var $dbParams;
        
		//! constructor. Sets filesystem paths.
	    function HTTP_WebDAV_Server_Filesystem($base_uri,$base_path_uri,$base_dir,$dbParams,$add_dir_slash=true) 
	    {
	    	parent::HTTP_WebDAV_Server($base_uri,$base_path_uri,$add_dir_slash);
	    	
	        // PHP messages destroy XML output -> switch them off
	        ini_set("display_errors", 0);
	        
	        //FY
	        $this->base_dir = rtrim($base_dir,'/');
	        
	        // set root directory, defaults to webserver document root if not set
	       // $this->base = $this->base_dir;
	       
	       // set db params
	       $this->dbParams=$dbParams;
	                        
	        ///FY
	    }
	    // }}}   
	    
	    //! map file uri to actual filesystem path.
	    function getRealPath($path_uri) 
	    {
	    	return $this->base_dir.$path_uri;
	    }
	    //! map file path to virtual path.
	    function getVirtPath($fpath) 
	    {
	    	return substr($fpath, strlen($this->base_dir));
	    }
	    
	     
	    /**
	     * Serve a webdav request
	     *
	     * @access public
	     * @param  string  
	     */
        function ServeRequest($path_uri=null)
        {
            // special treatment for litmus compliance test
            // reply on its identifier header
            // not needed for the test itself but eases debugging
            /*
            foreach(apache_request_headers() as $key => $value) {
                if (stristr($key,"litmus")) {
                    error_log("Litmus test $value");
                    header("X-Litmus-reply: ".$value);
                }
            }
            */
                
            // establish connection to property/locking db
            mysql_connect($this->dbParams['host'], $this->dbParams['user'], $this->dbParams['password']) or die(mysql_error());
            mysql_select_db($this->dbParams['database']) or die(mysql_error());
            // TODO throw on connection problems

            // let the base class do all the work
            parent::ServeRequest($path_uri);
        }

        /**
         * No authentication is needed here
         *
         * @access private
         * @param  string  HTTP Authentication type (Basic, Digest, ...)
         * @param  string  Username
         * @param  string  Password
         * @return bool    true on successful authentication
         */
        function check_auth($type, $user, $pass) 
        {
            return true;
        }


        /**
         * PROPFIND method handler
         *
         * @param  array  general parameter passing array
         * @param  array  return array for file properties
         * @return bool   true on success
         */
        function PROPFIND(&$options, &$files) 
        {
            nxLog("PROPFIND",'DAV');
            
           // get absolute fs path to requested resource
            $fspath = $this->getRealPath($options["path"]);
            
            // sanity check
            if (!file_exists($fspath)) {
				nxError("Error file $fspath doesnt exist",'DAV');
                return false;
            }
            
            switch($options["depth"])
            {
            	case "0":
	                $fileList=array($fspath);
	                break;
            	case "1":
	                $fileList=array_merge(array($fspath),$this->listDirFiles($fspath,1));
	                break;
            	case "1,noroot":
	                $fileList=$this->listDirFiles($fspath,1);
	                break;
            	case "infinity,noroot":
	                $fileList=$this->listDirFiles($fspath,50);
	                break;
            	case "infinity":
            	default:
	                $fileList=array_merge(array($fspath),$this->listDirFiles($fspath,50));
	                break;            		
            }

            // prepare property array
            $files["files"] = array();
            foreach ($fileList as $fspath)
            {
            	$path=$this->getVirtPath($fspath);
            	$files["files"][] = $this->fileinfo($path,$fspath);
            }

            // ok, all done
            return true;
        } 
        
        /**
         * Get properties for a single file/resource
         *
         * @param  string  resource path
         * @return array   resource properties
         */
        function fileinfo($path,$fspath) 
        {
            // map URI path to filesystem path
            // $fspath = $this->getRealPath($path);

            // create result array
            $info = array();
            $info["path"]  = $path;    
            $info["props"] = array();
            
            //$finfo=stat($fspath);
            
            // no special beautified displayname here ...
            $info["props"][] = $this->mkprop("displayname", strtoupper($path));
            $info["props"][] = $this->mkprop("href", $this->getURIPath($path));
            
            // creation and modification time
            $info["props"][] = $this->mkprop("creationdate",    filectime($fspath));
            $info["props"][] = $this->mkprop("getlastmodified", filemtime($fspath));

            // type and size (caller already made sure that path exists)
            if (is_dir($fspath)) {
                // directory (WebDAV collection)
                $info["props"][] = $this->mkprop("resourcetype", "collection");
                $info["props"][] = $this->mkprop("getcontenttype", "httpd/unix-directory");             
            } else {
                // plain file (WebDAV resource)
                $info["props"][] = $this->mkprop("resourcetype", "");
                if (is_readable($fspath)) {
                    $info["props"][] = $this->mkprop("getcontenttype", $this->_mimetype($fspath));
                } else {
                    $info["props"][] = $this->mkprop("getcontenttype", "application/x-non-readable");
                }               
                $info["props"][] = $this->mkprop("getcontentlength", filesize($fspath));
            }

            // get additional properties from database
            $query = "SELECT ns, name, value FROM properties WHERE path = '$path'";
            $res = mysql_query($query);
            while ($row = mysql_fetch_assoc($res)) {
                $info["props"][] = $this->mkprop($row["ns"], $row["name"], $row["value"]);
            }
            mysql_free_result($res);
            
            /*
            //fy add lock info
            $lock_xml=$this->supportedlock($path);
            if ($lock_xml!='')
            	$info["props"][]=$this->mkprop("supportedlock" ,$lock_xml);
            // /fy
			*/
			
            return $info;
        }
        
        /**
         * detect if a given program is found in the search PATH
         *
         * helper function used by _mimetype() to detect if the 
         * external 'file' utility is available
         *
         * @param  string  program name
         * @param  string  optional search path, defaults to $PATH
         * @return bool    true if executable program found in path
         */
        function _can_execute($name, $path = false) 
        {
            // path defaults to PATH from environment if not set
            if ($path === false) {
                $path = getenv("PATH");
            }
            
            // check method depends on operating system
            if (!strncmp(PHP_OS, "WIN", 3)) {
                // on Windows an appropriate COM or EXE file needs to exist
                $exts = array(".exe", ".com");
                $check_fn = "file_exists";
            } else { 
                // anywhere else we look for an executable file of that name
                $exts = array("");
                $check_fn = "is_executable";
            }
            
            // now check the directories in the path for the program
            foreach (explode(PATH_SEPARATOR, $path) as $dir) {
                // skip invalid path entries
                if (!file_exists($dir)) continue;
                if (!is_dir($dir)) continue;

                // and now look for the file
                foreach ($exts as $ext) {
                    if ($check_fn("$dir/$name".$ext)) return true;
                }
            }

            return false;
        }

        
        /**
         * try to detect the mime type of a file
         *
         * @param  string  file path
         * @return string  guessed mime type
         */
        function _mimetype($fspath) 
        {
            if (@is_dir($fspath)) {
                // directories are easy
                return "httpd/unix-directory"; 
            } else if (function_exists("mime_content_type")) {
                // use mime magic extension if available
                $mime_type = mime_content_type($fspath);
            } else if ($this->_can_execute("file")) {
                // it looks like we have a 'file' command, 
                // lets see it it does have mime support
                $fp = popen("file -i '$fspath' 2>/dev/null", "r");
                $reply = fgets($fp);
                pclose($fp);
                
                // popen will not return an error if the binary was not found
                // and find may not have mime support using "-i"
                // so we test the format of the returned string 
                
                // the reply begins with the requested filename
                if (!strncmp($reply, "$fspath: ", strlen($fspath)+2)) {                     
                    $reply = substr($reply, strlen($fspath)+2);
                    // followed by the mime type (maybe including options)
                    if (ereg("^[[:alnum:]_-]+/[[:alnum:]_-]+;?.*", $reply, $matches)) {
                        $mime_type = $matches[0];
                    }
                }
            } 
            
            if (empty($mime_type)) {
                // Fallback solution: try to guess the type by the file extension
                // TODO: add more ...
                // TODO: it has been suggested to delegate mimetype detection 
                //       to apache but this has at least three issues:
                //       - works only with apache
                //       - needs file to be within the document tree
                //       - requires apache mod_magic 
                // TODO: can we use the registry for this on Windows?
                //       OTOH if the server is Windos the clients are likely to 
                //       be Windows, too, and tend do ignore the Content-Type
                //       anyway (overriding it with information taken from
                //       the registry)
                // TODO: have a seperate PEAR class for mimetype detection?
                switch (strtolower(strrchr(basename($fspath), "."))) {
                case ".html":
                    $mime_type = "text/html";
                    break;
                case ".gif":
                    $mime_type = "image/gif";
                    break;
                case ".jpg":
                    $mime_type = "image/jpeg";
                    break;
                default: 
                    $mime_type = "application/octet-stream";
                    break;
                }
            }
            
            return $mime_type;
        }

        /**
         * GET method handler
         * 
         * @param  array  parameter passing array
         * @return bool   true on success
         */
        function GET(&$options) 
        {
            $fspath = $this->getRealPath($options["path"]);
			nxLog("GET $fspath",'DAV');

            // sanity check
            if (!file_exists($fspath)) return false;
            
            // is this a collection?
            if (is_dir($fspath)) {
                return $this->GetDir($fspath, $options);
            }
            
            // detect resource type
            $options['mimetype'] = $this->_mimetype($fspath); 
                
            // detect modification time
            // see rfc2518, section 13.7
            // some clients seem to treat this as a reverse rule
            // requiering a Last-Modified header if the getlastmodified header was set
            $options['mtime'] = filemtime($fspath);
            
            // detect resource size
            $options['size'] = filesize($fspath);
            
            // no need to check result here, it is handled by the base class
            $options['stream'] = fopen($fspath, "r");
            
            return true;
        }

        /**
         * GET method handler for directories
         *
         * This is a very simple mod_index lookalike.
         * See RFC 2518, Section 8.4 on GET/HEAD for collections
         *
         * @param  string  directory path
         * @return void    function has to handle HTTP response itself
         */
        function GetDir($fspath, &$options) 
        {
        	if ($this->add_dir_slash)
            	$path = $this->_slashify($options["path"]);
            	
            if ($path != $options["path"]) {
                header("Location: ".$this->getURIPath($path));
                exit;
            }

            // fixed width directory column format
            $format = "%15s  %-19s  %-s\n";

            $handle = @opendir($fspath);
            if (!$handle) {
                return false;
            }
            $fspath=$this->_slashify($fspath);
            	
            echo "<html><head><title>Index of ".$options['path']."</title></head>\n";
            
            echo "<h1>Index of ".$options['path']."</h1>\n";
            
            echo "<pre>";
            printf($format, "Size", "Last modified", "Filename");
            echo "<hr>";
            
            if ($path!='/')
            {
            	$parentPath=str_replace('\\','/',dirname($path));
            	$fullpath=$this->getRealPath($parentPath);
            	$uri=$this->getURIPath($parentPath);
                printf($format, 
                       '',
                       strftime("%Y-%m-%d %H:%M:%S", filemtime($fullpath)), 
                       "<a href='".htmlspecialchars($uri)."'>..</a>");            	
            }

            while ($filename = readdir($handle)) {
                if ($filename != "." && $filename != "..") {
                    $fullpath = $fspath.$filename;
                    $name = htmlspecialchars($filename);
                    if (is_dir($fullpath))
	                    printf("$format", 
	                           '',
	                           strftime("%Y-%m-%d %H:%M:%S", filemtime($fullpath)), 
	                           "<a href='$name'>[$name]</a>");
	                else
	                    printf($format, 
	                           number_format(filesize($fullpath)),
	                           strftime("%Y-%m-%d %H:%M:%S", filemtime($fullpath)), 
	                           "<a href='$name'>$name</a>");
                }
            }

            echo "</pre>";

            closedir($handle);

            echo "</html>\n";

            exit;
        }

        /**
         * PUT method handler
         * 
         * @param  array  parameter passing array
         * @return bool   true on success
         */
        function PUT(&$options) 
        {
           // map URI path to filesystem path
           $fspath = $this->getRealPath($options["path"]);
           
           nxLog("PUT $fspath",'DAV');

           if (!@is_dir(dirname($fspath))) {
                return "409 Conflict";
            }

            $options["new"] = ! file_exists($fspath);

            $fp = fopen($fspath, "w");

            return $fp;
        }


        /**
         * MKCOL method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function MKCOL($options) 
        {         
           // map URI path to filesystem path
           $fspath = $this->getRealPath($options["path"]);      	
        	  
           nxLog("MKCOL $fspath",'DAV');

           $parent = dirname($fspath);
           $name = basename($fspath);

            if (!file_exists($parent)) {
                return "409 Conflict";
            }

            if (!is_dir($parent)) {
                return "403 Forbidden";
            }

            if ( file_exists($parent."/".$name) ) {
                return "405 Method not allowed";
            }

            if (!empty($_SERVER["CONTENT_LENGTH"])) { // no body parsing yet
                return "415 Unsupported media type";
            }
            
            $stat = mkdir ($parent."/".$name,0777);
            if (!$stat) {
                return "403 Forbidden";                 
            }

            return ("201 Created");
        }
        
        
        /**
         * DELETE method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function DELETE($options) 
        {
           // map URI path to filesystem path
           $fspath = $this->getRealPath($options["path"]);      	
           nxLog("DELETE $fspath",'DAV');

            if (!file_exists($fspath)) return "404 Not found";

            if (is_dir($path)) {
                $query = "DELETE FROM properties WHERE path LIKE '".$options["path"]."%'";
                mysql_query($query);
                // System::rm("-rf $path");
                $this->rmDir($fspath);
            } else {
                unlink ($fspath);
            }
            $query = "DELETE FROM properties WHERE path = '$options[path]'";
            mysql_query($query);

            return "204 No Content";
        }


        /**
         * MOVE method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function MOVE($options) 
        {
           nxLog("MOVE",'DAV');         	
           return $this->COPY($options, true);
        }

        /**
         * COPY method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function COPY($options, $is_move=false) 
        {
            // TODO Property updates still broken (Litmus should detect this?)
         	nxLog("COPY",'DAV');
            
         	// inline content?
            if (!empty($_SERVER["CONTENT_LENGTH"])) { // no body parsing yet
                return "415 Unsupported media type";
            }

            // no copying to different WebDAV Servers yet
            if (isset($options["dest_url"])) {
                return "502 bad gateway";
            }

            // get source file path
            $source = $this->getRealPath($options["path"]);

            // get dest file path
            $dest = $this->getRealPath($options["dest"]);
            
            // source exists?
            if (!file_exists($source)) 
            	return "404 Not found";
            	
            $s_isdir=is_dir($source);

            // destination exists
            $t_exists = file_exists($dest);
            
            // destination collection uri doesn't exists
            $existing_col = false;
            
         	nxLog("COPY from $source to $dest ".($new?'(new)':'(existing or dir)'),'DAV');

         	//destination exists as a file or dir?
            if ($t_exists) 
            {
       			// delete source after move and target exists?
                if ($is_move && is_dir($dest)) 
                {
                    $dest = $this->_slashify($dest).basename($source);
                    if (file_exists($dest)) {
                        $options["dest"] .= $this->_slashify($options["dest"]).basename($source);
                        
	                	// overwrite allowed?
	                    if (!$options["overwrite"]) {
	                        return "412 precondition failed";
	                    }
	                    
                    } else {
                        $t_exists = false;
                        $existing_col = true;
                    }
                }
            }

            // if exists, remove the target before copying it
            // NB. not necessarly a good idea for a folder?
            if ($t_exists) {
            	// target exists => remove it
                if ($options["overwrite"]) {
                    $stat = $this->DELETE(array("path" => $options["dest"]));
                    if (($stat{0} != "2") && (substr($stat, 0, 3) != "404")) {
                        return $stat; 
                    }
                } else {                
                    return "412 precondition failed";
                }
            }

            if ($s_isdir && ($options["depth"] != "infinity")) {
                // RFC 2518 Section 9.2, last paragraph
                return "400 Bad request";
            }
                            
            if ($is_move) 
            {
            	// MOVE
            	
                if (!rename($source, $dest)) {
                    return "500 Internal server error";
                }
                if ($s_isdir) {
                    $destpath = $this->_slashify($options["dest"]);
                    $query = "UPDATE properties 
                                 SET path = REPLACE(path, '".$options["path"]."', '".$destpath."') 
                               WHERE path LIKE '".$options["path"]."%'";
                    mysql_query($query);
                }
                $query = "UPDATE properties 
                             SET path = '".$destpath."'
                           WHERE path = '".$options["path"]."%'";
                mysql_query($query);
            } 
            else 
            {
            	// COPY (deep copy)
         
                $files = array_reverse(System::find($source));
                
                if (!is_array($files)) {
                    return "500 Internal server error";
                }
                    
                foreach ($files as $file) {
                    $destfile = str_replace($source, $dest, $file);
                    if (is_dir($file)) {
                        if (!mkdir($destfile))
		                    return "500 Internal server error";                	
                    } else {
                        if (!copy($file, $destfile))
		                    return "500 Internal server error";                	
                    }
                }

                $query = "INSERT INTO properties SELECT ... FROM properties WHERE path = '".$options['path']."'";
            }

            return (!$exist && !$existing_col) ? "201 Created" : "204 No Content";         
        }

        /**
         * PROPPATCH method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function PROPPATCH(&$options) 
        {
            global $prefs, $tab;

            $msg = "";
            
            $path = $options["path"];
         	nxLog("PROPPATCH $path",'DAV');
            
            $dir = dirname($path)."/";
            $base = basename($path);
            
            foreach($options["props"] as $key => $prop) {
                if ($ns == "DAV:") {
                    $options["props"][$key][$status] = "403 Forbidden";
                } else {
                    if (isset($prop["val"])) {
                        $query = "REPLACE INTO properties SET path = '$options[path]', name = '$prop[name]', ns= '$prop[ns]', value = '$prop[val]'";
                    } else {
                        $query = "DELETE FROM properties WHERE path = '$options[path]' AND name = '$prop[name]' AND ns = '$prop[ns]'";
                    }       
                    mysql_query($query);
                }
            }
                        
            return "";
        }


        /**
         * LOCK method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function LOCK(&$options) 
        {
         	nxLog("LOCK",'DAV');
           if (isset($options["update"])) { // Lock Update
           	// todo: check update query..
				nxLog("Locks update '$options[path]'/'$options[update]'",'DAV');  
				
                $query = "UPDATE locks SET expires = ".(time()+300).
                		"WHERE path = '$options[path]'
                        AND token = '$options[update]'";
                		
                nxLog("sql: $query",'DAV');
                
                mysql_query($query);
                
                if (mysql_affected_rows()) {
                    $options["timeout"] = time()+300; // 5min hardcoded
//                    $options["timeout"] = 300; // 5min hardcoded
                    return true;
                } else {
                    return false;
                }
            }
            
            $options["timeout"] = time()+300; // 5min. hardcoded

            $query = "INSERT INTO locks
                        SET token   = '$options[locktoken]'
                          , path    = '$options[path]'
                          , owner   = '$options[owner]'
                          , expires = '$options[timeout]'
                          , exclusivelock  = " .($options['scope'] === "exclusive" ? "1" : "0")
                ;
            mysql_query($query);

            nxLog("Lock success '$options[path]'/'$options[locktoken]'",'DAV');  

            return mysql_affected_rows() ? "200 OK" : "409 Conflict";
        }

        /**
         * UNLOCK method handler
         *
         * @param  array  general parameter passing array
         * @return bool   true on success
         */
        function UNLOCK(&$options) 
        {
        	nxLog("UNLOCK",'DAV');
            $query = "DELETE FROM locks
                      WHERE path = '$options[path]'";
            
            // add tocken if specified (if not unlock forced..=> should this be allowed?)
            //  ending transaction?
            if (isset($options[token]) && trim($options[token])!='')
                    $query .= " AND token = '$options[token]'";
            else
	            nxLog("Forcing unlock on '$options[path]'...",'DAV');  
                        
            mysql_query($query);
			if (mysql_affected_rows())
			{
	            nxLog("Unlock success '$options[path]' token:'$options[token]'",'DAV');  
            	return "200 OK";
			}
			else{
				nxError("Error lock table: couldnt find the lock '$options[path]'/'$options[token]'",'DAV');
				return "409 Conflict";
			}
        }

        /**
         * checkLock() helper
         *
         * @param  string resource path to check for locks
         * @return bool   true on success
         */
        function checkLock($path) 
        {
            $result = false;
            
            $query = "SELECT owner, token, expires, exclusivelock
                  FROM locks
                 WHERE path = '$path'
               ";
            $res = mysql_query($query);

            if ($res) {
                $row = mysql_fetch_array($res);
                mysql_free_result($res);

                if ($row) {
                    $result = array( "type"    => "write",
                                                     "scope"   => $row["exclusivelock"] ? "exclusive" : "shared",
                                                     "depth"   => 0,
                                                     "owner"   => $row['owner'],
                                                     "token"   => $row['token'],
                                                     "expires" => $row['expires']
                                                     );
                }
            }

            return $result;
        }


        /**
         * create database tables for property and lock storage
         *
         * @param  void
         * @return bool   true on success
         */
        function create_database() 
        {
            // TODO
            return false;
        }
        
    // {{{ check_auth() 

    /**
     * check authentication
     *
     * overload this method to retrieve and confirm authentication information
     *
     * @abstract 
     * @param string type Authentication type, e.g. "basic" or "digest"
     * @param string username Transmitted username
     * @param string passwort Transmitted password
     * @returns bool Authentication status
     */
    
       function checkAuth($type, $username, $password) 
       {
       	// auth doesnt work if php in cgi mode (ie. doenst work with zend win enabler..)
       		//if (empty($username))
       		//	return false;
       			
       		return true;
       } 
    
    // }}}
    
    //! deep dir removal
	function rmDir($dirname){    
		// Simple delete for a file    
		if (is_file($dirname)) 
			return unlink($dirname);    

		if (!is_dir($dirname))
			return false;
			
		$this->chmod($dirname,0777);
		
		// Loop through the folder    
		$dir = opendir($dirname);
		while (false !== $entry = readdir($dir)) 
		{
			// Skip pointers        
			if ($entry == '.' || $entry == '..')
				continue;        
       
			// Deep delete directories              
			if (is_dir("$dirname/$entry"))
				$this->rmDir("$dirname/$entry");        
			else            
				unlink("$dirname/$entry");			    
		}
		
		// Clean up    
		closedir($dir);    
		return rmdir($dirname);
	}    
    // }}}        
        
	function listDirFiles($path,$levels=50)
	{
		$path = str_replace('//','/',$path);		

		$list=array();
		 if (!is_dir($path))
		 	return $list;
		
		 $path=$this->_slashify($path);
		 	
	     $handle=@opendir($path);
	     if ($handle === false)
	     {
	     	nxError("can't list file, unknown path:$path",'FS');
	     	return $list;
	     }
	     	
	     while($a=@readdir($handle)){
	         if(!preg_match('/^\./',$a)){
	         	$full_path="$path$a";
           		$list[]=$full_path; 
           	    
           		if(is_dir($full_path) && $levels>1)
               {
                   $recursive=$this->listDirFiles("{$full_path}/",$levels-1);
                   $list = array_merge($list,$recursive);
               }
	         }
	     }
	     closedir($handle);
	     return $list;
	}
    
    
    }


?>

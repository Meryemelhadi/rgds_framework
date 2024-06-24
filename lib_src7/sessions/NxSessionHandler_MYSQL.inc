<?php
/*
	Class NxSessionHandler
	
	description: implements a session handler based on mysql.
		this allows several servers to share the same session data,
		implementing a web server farm.
	
	Dependencies:
		NxProperties needs to be loaded before using this class.

	Database:
 		Create a new database in MySQL called "sessions" like so:
 
 		CREATE TABLE _sessions (
	       _sessionid char(32) not null,
	       _expiry int(11) unsigned not null,
	       _sessiondata text not null,
	       PRIMARY KEY (_sessionid)
  		);
  		
  	Usage:
		global $_NX_site;
		$sessionHandler=new NxSessionHandler($_NX_site);

*/


class NxSessionHandler {
    function __construct(&$props)
    {
		$this->error = new NxErrorCB();
		$this->db = & $props->getDB($this->error);
		
		if (!$this->error->isError())
		{
        	session_set_save_handler(
        		array(&$this,'openSession'),
        		array(&$this,'closeSession'),
        		array(&$this,'readSession'),
        		array(&$this,'writeSession'),
        		array(&$this,'destroySession'),
        		array(&$this,'gcSession')
        	);
		}
		
    }
    function openSession($sessionPath,$sessionName){
        return true;
    }

    function closeSession(){
        return true;
    }

    function readSession($sessionId)
    {
        // escape session ID
        if(!get_magic_quotes_gpc()){
            $sessionId=mysql_real_escape_string($sessionId);
        }
               
		$res=$this->db->queryWithData("SELECT _sessiondata FROM _sessions WHERE _sessionid='$sessionId' AND _expiry > ".time(),true,$this);
		if ($res!=null) {
            return $res['_sessiondata'];
        }
        
        // return empty string
        return "";
    }
    
    function writeSession($sessionId,$sessionData)
    {
        $expiry=time()+get_cfg_var('session.gc_maxlifetime')-1;
        
        // escape session ID & session data
        if(!get_magic_quotes_gpc()){
            $sessionId=mysql_real_escape_string($sessionId);
        }
        
        $result=$this->db->query("SELECT _sessionid FROM _sessions WHERE
_sessionid='$sessionId'",true,$this);
        
        // check if a new session must be stored or an existing  one must be updated 
        if ($result==false) {
			$this->db->query("INSERT INTO _sessions(_sessionid,_expiry,_sessiondata) VALUES	('$sessionId','$expiry','$sessionData')",true,$this);
        }
        else {
			$this->db->query("UPDATE _sessions SET _sessionid='$sessionId',_expiry='$expiry',_sessiondata='$sessionData' WHERE _sessionid='$sessionId'",false);
        }
        
        return true;
    }
    
    function destroySession($sessionId)
    {
        // escape session ID
        if(!get_magic_quotes_gpc()){
            $sessionId=mysql_real_escape_string($sessionId);
        }
        $this->db->query("DELETE FROM _sessions WHERE
_sessionid='$sessionId'",true,$this);
        return true;
    }
    
    function gcSession($maxlifetime){
        $this->db->query("DELETE FROM sessions WHERE _expiry < NOW()",true,$this);
        return true;
    }    
    
    // automatic table creation when error in query (managed by DB class)
    function createTableRecover($query,$view=null) 
	{
		$this->db->errorCB->clear();
		
        $this->db->query("CREATE TABLE _sessions (_sessionid char(32) not null,_expiry int(11) unsigned not null,_sessiondata text not null,PRIMARY KEY (_sessionid))",false);

        // check if success in creating the table
        return !($this->db->errorCB->isError());		
	}
   
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.'module/NxApplication.inc');

/** Application: manages a database table (creation/deletion)
  * properties:
  * $appProps = array(
  *		"table" => "News",
  *  	"databaseAdmin" => "databaseAdmin", // main console view
  * 	"errorMsg" => "errorMsg",  			// error message view
  *		"message"  => "message",			// message view
  *		"nbRecordPerPage" => 15);
  */
class NxApp_CreateDeleteTable extends NxApplication
{
	function NxApp_CreateDeleteTable($props,&$parentDesc)
	{
		$this->NxApplication("NxApp_CreateDeleteTable",$props,$parentDesc);	
		$this->db = $this->getPlugIn('DB');
	}
	
	function run()
	{	
		$table = $this->getProperty("table",null);
		$component = $this->getProperty("db.component",null);
		
		if ($table != null)
		{
			$name = $table;
			$label = "table";
			$createOp = "createTable";
			$deleteOp = "deleteTable";
		}
		else if ($component != null)
		{
			$name = $component;
			$label = "tables for component";
			$createOp = "createTables";
			$deleteOp = "deleteTables";
		}
		else	
		{
			// error: coming from a direct URL (bookmark or "previous/forward" button)
			$errorDesc = array(
				"msg" => "Error: unknown table or component name",
				"button.redirect" => "back",
				"button.url" => $this->fromPage
			);
			
			$this->runView("view.message",$errorDesc);
			$this->finish();
			return;
		}
		
		switch($this->operation)
		{
			// table creation
			case "createTable":
				switch($this->step)
				{
					// step 2 : execute creation
					case "create":
						// check if bookmark is used
						if ($this->previousStep != "confirm")
						{
							// error: coming from a direct URL (bookmark or "previous/forward" button)
							$msg = "Error in the sequence of operations. Creation of table ".$table." failed";
						}
						else
						{
							// ok, create the table
							$errorCB = new NxErrorCB();
							if( $this->db->executeDBQuery($name,"createSQLTable",$errorCB) )
							{
								$msg = "The table ".$name." was successfully created";
							}
							else
							{
								// error: database error
								$msg = "Creation of table ".$name." failed : " . $errorCB->getErrorMsg();
							}
						}
						$MessageProps = array(
							"msg" => $msg, 
							"button.redirect" => "back",
							"button.url" => $this->fromPage
						);				
						$this->runView("view.message",$MessageProps);
						$this->finish();
						return;

					// step 1 : confirm creation
					case "confirm":
					default:
						$MessageProps = array(
							"msg" => "Do you really want to create the table \"".$name."\"",
							"b2.redirect"=>"ok",
							"b2.url"=>$this->getUrl("create"),
							"b1.redirect"=>"cancel",
							"b1.url"=>$this->fromPage
						);				
						$this->runView("view.dialog2buttons",$MessageProps);
						break;
				}
				break;

			// table deletion
			case "deleteTable":
				switch($this->step)
				{
					// step 2 : execute creation
					case "delete":
						// check if bookmark is used
						if ($this->previousStep != "confirm")
						{
							// error: coming from a direct URL (bookmark or "previous/forward" button)
							$msg = "Error in the sequence of operations. Deletion of table ".$name." failed";
						}
						else
						{
							// ok, create the table
							$errorCB = new NxErrorCB();
							if( $this->db->executeDBQuery($name,"deleteSQLTable",$errorCB) )
							{
								$msg = "The table ".$name." was successfully deleted";
							}
							else
							{
								// error: database error
								$msg = "Deletion of table ".$name." failed : " . $errorCB->getErrorMsg();
							}
						}
						$MessageProps = array(
							"msg" => $msg, 
							"button.redirect" => "back",
							"button.url" => $this->fromPage
						);				
						$this->runView("view.message",$MessageProps);
						$this->finish();
						return;

					// step 1 : confirm deletion
					case "confirm":
					default:
						$MessageProps = array(
							"msg" => "Do you really want to delete the table \"".$name."\"", 
							"b2.redirect"=>"ok",
							"b2.url"=>$this->getUrl("delete"),
							"b1.redirect"=>"cancel",
							"b1.url"=>$this->fromPage
						);				
						$this->runView("view.dialog2buttons",$MessageProps);
						break;
				}
				break;
			// table creation
			case "createTables":
				switch($this->step)
				{
					// step 2 : execute creation
					case "create":
						// check if bookmark is used
						if ($this->previousStep != "confirm")
						{
							// error: coming from a direct URL (bookmark or "previous/forward" button)
							$msg = "Error in the sequence of operations. Creation of tables for component ".$component." failed";
						}
						else
						{
							// ok, create the table
							$errorCB = new NxErrorCB();
							if( $this->db->executeDBQuery("component","createSQLTables",$errorCB,true) )
							{
								$msg = "Tables for component ".$component." were successfully created";
							}
							else
							{
								// error: database error
								$msg = "Creation of tables for component ".$component." failed : " . $errorCB->getErrorMsg();
							}
						}
						$MessageProps = array(
							"msg" => $msg, 
							"button.redirect" => "back",
							"button.url" => $this->fromPage
						);				
						$this->runView("view.message",$MessageProps);
						$this->finish();
						return;

					// step 1 : confirm creation
					case "confirm":
					default:
						$MessageProps = array(
							"msg" => "Do you really want to create tables for component \"".$name."\"",
							"b2.redirect"=>"ok",
							"b2.url"=>$this->getUrl("create"),
							"b1.redirect"=>"cancel",
							"b1.url"=>$this->fromPage
						);				
						$this->runView("view.dialog2buttons",$MessageProps);
						break;
				}
				break;

			// table deletion
			case "deleteTables":
				switch($this->step)
				{
					// step 2 : execute creation
					case "delete":
						// check if bookmark is used
						if ($this->previousStep != "confirm")
						{
							// error: coming from a direct URL (bookmark or "previous/forward" button)
							$msg = "Error in the sequence of operations. Deletion of tables for component ".$component." failed";
						}
						else
						{
							// ok, create the table
							$errorCB = new NxErrorCB();
							if( $this->db->executeDBQuery("component","deleteSQLTables",$errorCB) )
							{
								$msg = "Tables for component ".$component." were successfully deleted";
							}
							else
							{
								// error: database error
								$msg = "Deletion of tables for component ".$component." failed : " . $errorCB->getErrorMsg();
							}
						}
						$MessageProps = array(
							"msg" => $msg, 
							"button.redirect" => "back",
							"button.url" => $this->fromPage
						);				
						$this->runView("view.message",$MessageProps);
						$this->finish();
						return;

					// step 1 : confirm deletion
					case "confirm":
					default:
						$MessageProps = array(
							"msg" => "Do you really want to delete the table \"".$table."\"", 
							"b2.redirect"=>"ok",
							"b2.url"=>$this->getUrl("delete"),
							"b1.redirect"=>"cancel",
							"b1.url"=>$this->fromPage
						);				
						$this->runView("view.dialog2buttons",$MessageProps);
						break;
				}
				break;
								
			// default operation : display console and list of operations
			case "console":
			default:
				$adminProps = array(
					"table" => $table,
					"msg" => "Please select the operation you want to apply on the database",

					// create operation
					"b1.url"  => $this->getUrl("confirm",$createOp),
					"b1.redirect"  => "create",
					
					// delete operation
					"b2.url"  => $this->getUrl("confirm",$deleteOp),
					"b2.redirect"  => "delete"
				);
				$this->runView("view.dialog2buttons",$adminProps);
				break;
		}

		// store current state in session
		$this->saveState();
	}
}

?>
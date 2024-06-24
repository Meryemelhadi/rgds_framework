<?php
/* Database Backup Utility 1.0 By Eric Rosebrock, http://www.phpfreaks.com
Written: July 7th, 2002 12:59 AM

If running from shell, put this above the <?php  "#! /usr/bin/php -q"  without the quotes!!!

This script is dedicated to "Salk". You know who you are :)

This script runs a backup of your database that you define below. It then gzips the .sql file
and emails it to you or ftp's the file to a location of your choice. 

It is highly recommended that you leave gzip on to reduce the file size.

You must chown the directory this script resides in to the same user or group your webserver runs
in, or CHMOD it to writable. I do not recommend chmod 777 but it's a quick solution. If you can setup
a cron, you can probably chown your directory! 

IMPORTANT!!! I recommend that you run this outside of your
web directory, unless you manually want to run this script. If you do upload it inside your web
directory source tree, I would at least apply Apache access control on that directory. You don't 
want people downloading your raw databases!

This script is meant to be setup on a crontab and run on a weekly basis
You will have to contact your system administrator to setup a cron tab for this script
Here's an example crontab:

0 0-23 * * * php /path/to/thisdirectory/dbsender.php > dev/null

*/

// ========= START CONFIG =============

// configure your database variables below:
$dbhost = 'localhost'; // Server address of your MySQL Server
$dbuser = ''; // Username to access MySQL database
$dbpass = ''; // Password to access MySQL database
$dbname = ''; // Database Name

// Optional Options You May Optionally Configure

$use_gzip = "yes";  // Set to No if you don't want the files sent in .gz format
$remove_sql_file = "yes"; // Set this to yes if you want to remove the .sql file after gzipping. Yes is recommended.
$remove_gzip_file = "no"; // Set this to yes if you want to delete the gzip file also. I recommend leaving it to "no"
 
// Configure the path that this script resides on your server.

$savepath = "/var/www/nx/bak"; // Full path to this directory. Do not use trailing slash!

$send_email = "yes";  // Do you want this database backup sent to your email? Fill out the next 2 lines
$to      = "backup@nxfrontier.com";  // Who to send the emails to
$from    = "backup@nxfrontier.com"; // Who should the emails be sent from?

$senddate = date("j F Y");

$subject = "MySQL Backup [$dbname] - $senddate"; // Subject in the email to be sent.
$message = "Your MySQL database [$dbname] has been backed up and is attached to this email"; // Brief Message.


$use_ftp = "yes"; // Do you want this database backup uploaded to an ftp server? Fill out the next 4 lines
$ftp_server = ""; // FTP hostname
$ftp_user_name = ""; // FTP username
$ftp_user_pass = ""; // FTP password
$ftp_path = "/bak_db/"; // This is the path to upload on your ftp server!

// paths to scripts
$mysqldump="/usr/bin/mysqldump";
$tar="/bin/tar";
$rm="/bin/rm";
$ncftpput="/usr/bin/ncftpput";

// ========== END CONFIG =============

// Do not Modify below this line! It will void your warranty!

	$date = date("mdy-hia");
	$filename = "$savepath/$dbname-$date.sql";	
	passthru("$mysqldump --opt -h$dbhost -u$dbuser -p$dbpass $dbname >$filename");
	
	if($use_gzip=="yes"){
		$zipline = "$tar -czf ".$dbname."-".$date."_sql.tar.gz $dbname-$date.sql";
		shell_exec($zipline);
	}
	if($remove_sql_file=="yes"){
		exec("$rm -r -f $filename");
	}
	
	if($use_gzip=="yes"){
		$filename2 = "$savepath/".$dbname."-".$date."_sql.tar.gz";
	} else {
		$filename2 = "$savepath/$dbname-$date.sql";
	}
	
	
	if($send_email == "yes" ){
		$fileatt_type = filetype($filename2);
		$fileatt_name = "".$dbname."-".$date."_sql.tar.gz";
		
		$headers = "From: $from";
		
		// Read the file to be attached ('rb' = read binary)
		$file = fopen($filename2,'rb');
		$data = fread($file,filesize($filename2));
		fclose($file);
	
		// Generate a boundary string
		$semi_rand = md5(time());
		$mime_boundary = "==Multipart_Boundary_x{$semi_rand}x";
	
		// Add the headers for a file attachment
		$headers .= "\nMIME-Version: 1.0\n" ."Content-Type: multipart/mixed;\n" ." boundary=\"{$mime_boundary}\"";
	
		// Add a multipart boundary above the plain message
		$message = "This is a multi-part message in MIME format.\n\n" ."--{$mime_boundary}\n" ."Content-Type: text/plain; charset=\"iso-8859-1\"\n" ."Content-Transfer-Encoding: 7bit\n\n" .
		$message . "\n\n";
	
		// Base64 encode the file data
		$data = chunk_split(base64_encode($data));
	
		// Add file attachment to the message
		$message .= "--{$mime_boundary}\n" ."Content-Type: {$fileatt_type};\n" ." name=\"{$fileatt_name}\"\n" ."Content-Disposition: attachment;\n" ." filename=\"{$fileatt_name}\"\n" ."Content-Transfer-Encoding: base64\n\n" .
		$data . "\n\n" ."--{$mime_boundary}--\n";
	
		// Send the message
		$ok = @mail($to, $subject, $message, $headers);
		if ($ok) {
			echo "<h4><center>Database backup created and sent! File name $filename2</center></h4>";
		} else {
			echo "<h4><center>Mail could not be sent. Sorry!</center></h4>";
		}
	}
	
	// /usr/bin/ncftpput -u franck.yvetot -p caen1803 -d debsender_ftplog.log -e dbsender_ftplog2.log -a -E -V ftpperso.free.fr /.bak/a24/nxsites/ /var/www/nx/bak/NxSites_-_nxsites-092005-1119pm_sql.tar.gz
	if($use_ftp == "yes"){
		$ftpconnect = "$ncftpput -u $ftp_user_name -p $ftp_user_pass -d debsender_ftplog.log -e dbsender_ftplog2.log -a -m -E -V $ftp_server $ftp_path $filename2";
		echo($ftpconnect);
		system($ftpconnect);
		echo "<h4><center>$filename2 Was created and uploaded to your FTP server!</center></h4>";
	
	}
	
	if($remove_gzip_file=="yes"){
		exec("$rm -r -f $filename2");
	}

?>

<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// ------------------------------------------------------------------------- //
// Classe pour l'envoi d'emails avec pièces jointes. //
// ------------------------------------------------------------------------- //


class NxMail_MimeMessage 
{
	var $parts;
	var $to;
	var $from;
	var $headers;
	var $subject;
	var $body;
	
	// constructeur 
	function __construct() 
	{
		$this->parts = array();
		$this->to = "";
		$this->from = "";
		$this->subject = "";
		$this->body = "";
		$this->headers = "";
	}
	
	// attache un fichier au message
	function attach(&$message, $name, $ctype = '', $encode=null) 
	{
		// type de contenu non defini
		if (empty($ctype)) 
		{
			// on essaie de reconnaitre l'extension
			switch(strrchr(basename($name), ".")) 
			{
				case ".gz":  $ctype = "application/x-gzip"; break;
				case ".tgz": $ctype = "application/x-gzip"; break;
				case ".zip": $ctype = "application/zip"; break;
				case ".pdf": $ctype = "application/pdf"; break;
				case ".png": $ctype = "image/png"; break;
				case ".gif": $ctype = "image/gif"; break;
				case ".jpg": $ctype = "image/jpeg"; break;
				case ".txt": $ctype = "text/plain"; break;
				case ".htm": $ctype = "text/html"; break;
				case ".html": $ctype = "text/html"; break;
				case ".doc": $ctype = "application/msword"; break;
				default: $ctype = "application/octet-stream"; break;
			}	
		}
	
		$this->parts[] = array ("ctype" => $ctype,
								"message" => $message,
								// "encode" => $encode,
								"name" => $name
								);
	}
	
	// fonction utilisee pour contruire le message MIME
	// utilisee depuis build_multipart()
	function build_message($part) 
	{
		// $message = $part["message"];
		$message = chunk_split(base64_encode($part["message"]));
		$encoding = "base64";
		
		return "Content-Type: ".$part[ "ctype"].
				($part[ "name"]? "; name = \"".$part[ "name"]. "\"" : "").
				"\nContent-Transfer-Encoding: $encoding\n\n$message\n\n";	
	}
	
	// compose le message MIME
	// utilisee depuis send()
	function build_multipart() 
	{
		$boundary = "b".md5(uniqid(time()));
		$multipart = 'Content-Type: multipart/mixed; boundary = '.$boundary;
		$multipart .= "\n\nThis is a MIME encoded message.\n\n--".$boundary;
		
		for ($i = sizeof($this->parts) - 1; $i >= 0; $i--) 
			$multipart .= "\n".$this->build_message($this->parts[$i]).'--'.$boundary;
		
		return $multipart.= "--\n";
	}
	
	function file_get_contents($f) {
	   ob_start();
	   $retval = @readfile($f);
	   if (false !== $retval) { // no readfile error
	     $retval = ob_get_contents();
	   }
	   ob_end_clean();
	  return $retval;
	}

	function attach_file($full_path,$filename)
	{
		$fp = fopen($full_path, "rb");
		if ( $fp ) 
		{
			$size = filesize( $full_path );
			// 'nomdufichier' peut etre un nom complètement différent du fichier réel
			$contenu_fichier = fread( $fp, $size);
			fclose ($fp);
			
			//$contenu_fichier = $this->file_get_contents($full_path);
			/*
			echo "reading file: " . $full_path;
			echo "<ul><li>file size = " . $size;
			echo "<li>read size" . strlen($contenu_fichier); 
			echo "</ul><pre>"; 
			// echo chunk_split(base64_encode($contenu_fichier));
			echo "</pre>";
			
			$fw = fopen("c:\\webs\\tests\\".$filename, "ab");
			fwrite($fw,base64_decode(base64_encode($contenu_fichier)),$size);
			fclose ($fw);
			*/
			
			$this->attach($contenu_fichier, $filename);
		}
	}

	// envoie le message
	// derniere fonction a appeler 
	function send() 
	{
		$mime = 'Subject: '.$this->subject."\n";
		
		// parametres optionnels
		if (!empty($this->from)) 
			$mime .= '"From: '.$this->from. "\n";
			
		if (!empty($this->headers)) 
			$mime .= $this->headers. "\n";
			
		if (!empty($this->body)) 
			$this->attach($this->body, '', 'text/plain');
		
		// entete MIME
		$mime .= "MIME-Version: 1.0\n".$this->build_multipart();

		echo "<pre>";
		echo $mime;
		echo "</pre>";
		// envoi du message
		@mail( $this->to, $this->subject, '', $mime );	
	}

}

// ------------------------------------------------------------------------- //
// Exemple d'utilisation //
// ------------------------------------------------------------------------- //
/*
$file_name = 'fichier.txt';

@$fp = fopen( $file_name, "r");

if ( $fp ) {

$contenu_fichier = fread( $fp, filesize( $file_name ));

$mail = new NxMail_MimeMessage();
$mail->from = 'toto@email.com';
$mail->subject = 'Message de test';
$mail->body = ' ';
// 'nomdufichier' peut etre un nom complètement différent du fichier réel
$mail->attach( $contenu_fichier, 'nomdufichier');
$mail->to = 'toto@email.fr';
$mail->send();
}
*/
?>

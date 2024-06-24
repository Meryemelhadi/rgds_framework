<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// ------------------------------------------------------------------------- //
// Classe pour l'envoi d'emails avec pi�ces jointes. //
// ------------------------------------------------------------------------- //


class NxPlug_Mail 
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
	function attach($message, $name, $ctype = '',$char_encode='utf-8') 
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
				default: $ctype = "application/octet-stream"; break;
			}	
		}
	
		$this->parts[] = array ("ctype" => $ctype,
								"message" => $message,
								"encode" => $encode,
								"name" => $name
								);
	}
	
	// fonction utilisee pour contruire le message MIME
	// utilisee depuis build_multipart()
	function build_message($part) 
	{
		$message = $part["message"];
		$message = chunk_split(base64_encode($message));
		$encoding = "base64";
		
		return "Content-Type: ".$part[ "ctype"].
				($part[ "name"]? "; name = \"".$part[ "name"]. "\"" : "").
				"\nContent-Transfer-Encoding: $encoding\n\n$message\n";	
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
	
	// envoie le message
	// derniere fonction a appeler 
	function send() 
	{
		$mime = 'Subject: '.$this->subject."\n";
		
		// parametres optionnels
		if (!empty($this->from)) $mime .= '"From: '.$this->from. "\n";
		if (!empty($this->headers)) $mime .= $this->headers. "\n";
		if (!empty($this->body)) $this->attach($this->body, '', 'text/plain');
		
		// entete MIME
		$mime .= "MIME-Version: 1.0\n".$this->build_multipart();
		
		// envoi du message
		mail( $this->to, $this->subject, '', $mime );	
	}

}

// ------------------------------------------------------------------------- //
// Exemple d'utilisation //
// ------------------------------------------------------------------------- //

$file_name = 'fichier.txt';

@$fp = fopen( $file_name, "r");

if ( $fp ) {

$contenu_fichier = fread( $fp, filesize( $file_name ));

$mail = new mime_mail();
$mail->from = 'toto@email.com';
$mail->subject = 'Message de test';
$mail->body = ' ';
// 'nomdufichier' peut etre un nom compl�tement diff�rent du fichier r�el
$mail->attach( $contenu_fichier, 'nomdufichier');
$mail->to = 'toto@email.fr';
$mail->send();
}

?>

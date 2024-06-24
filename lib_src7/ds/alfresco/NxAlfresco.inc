<?php 

include (NX_LIB.'NxConfig.inc');

class NxAlfresco extends NxConfig
{
public $host='';  
public $port='';
public $user=""; 
public $pwd="";
public $ticket="";


public function __construct(&$ctxt,$instance='default')
{
	parent::__construct($ctxt,'alfresco',$instance);

	$this->host = $this->getProperty('alfresco.host','127.0.0.1');
	$this->port = $this->getProperty('alfresco.port','8080');
	$this->user = $this->getProperty('alfresco.user','admin');
	$this->password = $this->getProperty('alfresco.password','admin');
}

// fonction qui permet de verifier si le login ($user) , le mot de passe ($pwd) sont correctes , retourner true et initialiser la propriete ticket si vrais , si nn retourner false
public function login($user=null,$pwd=null)
{
	if ($this->ticket)
		return true;

	if (!$user)
		$user = $this->user;

	if (!$password)
		$password = $this->password;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/api/login?';
	$url .='u='.$user;
	$url .='&pw='.$pwd;
	try
	{
		$ticket= @simplexml_load_file($url);
	}
	catch(Exception $e)
	{
		return false;
	}

	if ($ticket!="")
	{
		$this->ticket=$ticket;
		$this->user = $user;
		$this->password = $password;
		return true;
	}
	else 
		return false;
}
	 
// fonction qui permet de verifier 	si un utilisateur est connecté ou pas	
public function isConnected() 
{
	return ($this->ticket=="")?true:false; 
}
	
public function close()
{
	$this->user="";
	$this->pwd="";
	$this->ticket="";
}
	
// fonction qui permet  de récupérer le "ticket" de la connexion d'un utilisateur alfresco 

public function getTicket($user=null,$pwd=null)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/api/login?';
	$url .='u='.$user;
	$url .='&pw='.$pwd;

	try
	{
		$ticket= @simplexml_load_file($url);
	}
	catch(Exception $e)
	{
		return 0;
	}
	if ($ticket!="")
	{
		return $ticket;
	}
	else 
		return 0;	
}

// fonction qui permet de tester si un utilisateur  ($utilisateur) est admin ou pas
	
public function isAdmin($utilisateur) 
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/isAdmin?';
	$url .='nom='.$utilisateur;
	$url .='&alf_ticket='.$this->ticket;

	try
	{
		$admin= @simplexml_load_file($url);
	}
	catch(Exception $e)
	{	 
	} 

	return ($admin=="true")?true:false;
}
	
// fonction qui permet de tester si l'utilisateur  ($utilisateur) est l'utilisateur Guest ou pas

public function isGuest($utilisateur) 
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/isGuest?';
	$url .='nom='.$utilisateur;
	$url .='&alf_ticket='.$this->ticket;

	try
	{
		$admin= @simplexml_load_file($url);
	}
	catch(Exception $e)
	{	 
	} 

	return ($admin=="true")?true:false;
}
	
// fonction qui permet de lister tous les utilisateurs alfresco
	
public function getUsers()
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/api/people?';
	$url .='alf_ticket='.$this->ticket;
	try
	{
		$users =json_decode(@file_get_contents($url));
	}
	catch(Exception $e)
	{
	}
	return $users;

	/*foreach($json->people as $user)
			{
		echo $user->userName.'<br/>';  	
	}*/
}	
	
// fonction qui permet de lister tous les groups alfresco
public function getGroups()
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/api/rootgroups?';
	$url .='alf_ticket='.$this->ticket;
	try
	{
		$json =json_decode(@file_get_contents($url));
	}
	catch(Exception $e)
	{
	}

	return $json;

	/*foreach($json->data as $group)
			{
		echo $group->Name.'<br/>';  	
	}*/
}


public function getFolders()
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/doc?';
	$url .='alf_ticket='.$this->ticket;

	try
	{
		$xml= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}

/*foreach ($xml->dossier as $dossier){
echo $dossier->name.' '.$dossier->createur.'<br/>'; 
}*/
}
	
public function getFiles()
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/doccument?';
	$url .='alf_ticket='.$this->ticket;
	try
	{
		$xml= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}
/*foreach ($xml->dossier as $dossier)
{
echo $dossier->createur.'<br/>'; 
}*/
 
}
	
public function addUser($login,$nom,$prenom,$email,$pwd,$enable)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/ajouterutilisateur?';
	$url .='login='.$login;
	$url .='&nom='.$nom;
	$url .='&prenom='.$prenom;
	$url .='&email='.$email;
	$url .='&pwd='.$pwd;
	$url .='&enable='.$enable;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}

	return $resultat;
}
	
public function deleteUser($login)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/supprimerUtilisateur?';
	$url .='login='.$login;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}

	return $resultat;
}
			
public function setPassword($login,$nouveau)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/setPassword?';
	$url .='login='.$login;
	$url .='&nouveau='.$nouveau;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
	try
	{
	  $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}

	return $resultat;
}

public function updatePassword($login,$ancien,$nouveau)
{
	if (!$this->login())
		return null;
	
	 if ($ancien!="nouveau")    //  if ($ancien!=$this->pwd)  
		return false;
	 else 
		 return ($this->setPassword($login,$nouveau)=="true")?true:false;

}
	
public function addGroup($nom)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/ajouterGroup?';
	$url .='nom='.$nom;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
		echo "ERREUR : $e";
	}
	return $resultat;
}
	
public function deleteGroupe($nom)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/supprimerGroupe?';
	$url .='nom='.$nom;
	$url .='&alf_ticket='.$this->ticket;

	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}

	return $resultat;
}
	
// fonction qui permet d'ajouter un utilisateur ($user) à un groupe ($groupe) 
public function addAuthority($user,$groupe)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/addAuthority?';
	$url .='user='.$user;
	$url .='&grp='.$groupe;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
		try
			{
				 $resultat= @simplexml_load_file($url); 
			}
		catch(Exception $e)
			{
			}
	return $resultat;
}
	
// fonction qui permet supprimer un utilisateur ($user) d'un groupe ($groupe)
public function deleteAuthority($user,$groupe)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/removeAuthority?';
	$url .='user='.$user;
	$url .='&grp='.$groupe;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
		try
			{
				 $resultat= @simplexml_load_file($url); 
			}
		catch(Exception $e)
			{
			}
	return $resultat;
}

public function search($key)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/sharch?';
	$url .='key='.$key;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;

	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
	}

	return  $resultat;
}

public function addSpace($nom,$chemin)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/creerEspace?';
	$url .='nom='.$nom;
	$url .='&chemin='.$chemin;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
		echo "ERREUR : $e : $resultat";
	}
	return $resultat;
}
	
public function deleteSpace($chemin)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/supprimerEspace?';
	$url .='chemin='.$chemin;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
	try
	{
		 $resultat= @simplexml_load_file($url); 
	}
	catch(Exception $e)
	{
		echo "ERREUR : $e";
	}
	return  $resultat;
}
	
public function copySpace($chemin,$destination)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/supprimerEspace?';
	$url .='chemin='.$chemin;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
		try
			{
				 $resultat= @simplexml_load_file($url); 
			}
		catch(Exception $e)
			{
			}
	return  $resultat;
}
	
public function sendEmail($user,$chemin)
{
	if (!$this->login())
		return null;

	$url='http://'.$this->host.':'.$this->port.'/alfresco/service/sendusermail?';
	$url .='user='.$user;
	$url .='chemin='.$chemin;
	$url .='&alf_ticket='.$this->ticket;
	//echo $url;
		try
			{
				 $resultat= @simplexml_load_file($url); 
			}
		catch(Exception $e)
			{
				echo "ERREUR : $e";
			}
	return  $resultat;
}
		
}

?>
<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

class NxPlug_Password
{
	var $app;
	function __construct(&$app)
	{
		$this->app =& $app;
	}
	
	function createPassword($count)
	{
		// using a predetermined character set
		// in this example the characters ! $ 0-9 @ A-Z a-z
		mt_srand((double)microtime()*1000000);
		$key = "";
		for ($i=0; $i<$count; $i++)
		{
			$rndchr=mt_rand(0,64);
			if ($rndchr==0) 
				$c=chr(33);
			elseif ($rndchr==1)
				$c=chr(36);
			elseif ($rndchr<=11) 
				$c=chr(46+$rndchr);
			elseif ($rndchr<=38) 
				$c=chr((52+$rndchr));
			else 
				$c=chr(58+$rndchr);
			
			$key .= $c;
		}
		return $key;
	}

	// ------------------------------------------------------------------------- //
	// Génère des id plus faciles à lire et à retenir : //
	// Ex: sinquinko, qeufetoi, rindinpon, tatenson, etc. //
	// ------------------------------------------------------------------------- //
	function pandore($syllabes)
	{
		
		return $mot;
	}
	
	function createEasyPassword($count)
	{
		$syllabes = $count/2+1;
		$consonnes = explode(",","b,c,d,f,g,gu,h,j,k,l,m,n,p,qu,r,s,t,v,w,x,z");
		$voyelles = explode(",","a,e,i,oo,ou,y,en,an,in,ing,eu,oy,er");
		
		mt_srand((double)microtime()*1000000);
		$mot = "";
		
		for ($i = 0; $i < $syllabes; $i++) 
		{
			$mot .= $consonnes[mt_rand() % count($consonnes)];
			$mot .= $voyelles[mt_rand() % count($voyelles)];
		}

		return substr($mot, 0, $count);
	}
}

?>

<?php

include_once('application_crud.inc');

class application_api extends application_crud
{
	function page($package,$path,$role,$application,$skin,$fpath,$lib,$node)
	{
		$src= '<?php
/* file automatically generated from NXL source ['.$lib->context->getFileURI().']. Do not edit it. */
		
';	
		$lib->getAttribute($perms,$node,'permissions','default');

		// 'protected' => 'no',
		$lib->getAttribute($isprot,$node,'protected','no');
		switch(strtolower($isprot))
		{
			case '':
			case 'false':
			case 'public':
			case 'no':
				break;
			case 'true':
			case 'yes':
				$src .= "\$loginPerm='$perms';
\$loginService='user';
";
				break;
			default:
				$src .= "\$loginPerm='$perms';
\$loginService='{$isprot}';
";
		}
		
		if ($lib->getBool($isprot))
			$src .= '$loginPerm="default";
';
	    	
		// pop skin
		$lib->context->getLastResource('skin',true);

		// get mime and encoding
		$lib->getAttribute($type,$node,'type','html');
		switch(strtolower($type))
		{
			case 'js':
			case 'javascript':
				$mime='application/x-javascript';
				break;
			case 'css':
				$mime='text/css';
				break;
			case 'xml':
				$mime='text/xml';
				break;
			case 'html':
			default:
				$mime='text/html';
				break;
		}
		
		// get mime and encoding
		$lib->getAttribute($mime,$node,'mime',$mime);
		
		$src.="global \$NX_MIME;\n\$NX_MIME='$mime';\n";
		
		// get site conf path
		//$path=$lib->getLastResource('path',false);
		$sitePath = $lib->getSitePath($path).'NxSite.inc';
		$src.="require_once('$sitePath');";
			
		$OPENPHP='<?php';
		$CLOSEPHP='?>';

		$content = <<<EOH

if(isset(\$_REQUEST['op']))
{
	/* application */
	if(!isset(\$_REQUEST['u'])&&!isset(\$_REQUEST['ukey']))
	{
		if(isset(\$_REQUEST['ukey']))
			// mode session avec encoded u/p
			\$_GET['mkey'] = \$_REQUEST['ukey'];

		// mode session classique
		\$loginPerm='default';
		\$loginService='user';
	}
	// else : moe avec u/p fournis ou ukey

	require_once('../../../nx/NxSite.inc');
	require_once(GENERIS_PACKAGES.'workflow/lib/api/APIBase.inc');

	/** MODIFIER CES VALEURS (si besoin)**/
	\$package = '{$package}';
	\$app  = '{$application}';
	\$role = '{$role}';
	\$skin = '{$skin}';
	/** FIN MODIF **/

	\$app=new APIBase(
		array(
			'nx.package'=>\$package,
			'application.id'=>"\$app@\$package",
			'application.role'=>\$role,
			'application.dir'=>'application'
		),
		\$siteDesc);
	\$app->run();
}
else
{
    header('Content-Type: text/html; charset=utf-8');
$CLOSEPHP
<html>
<body>
<p>Vacancies API for <?php echo INTRANET_TITLE; ?>. Please fill the following fields, or call the page as web service in REST/JSON mode with params names (names are those displayed in brackets here below ex. [p] for "p" parameter, etc.</p>
<form action="#" method="POST">
	<table>
		<tr><td>user: [u]</td><td><input name="u" type="text" value="" /></td></tr>
		<tr><td>password: [p]</td><td><input name="p" type="password" value="" /></td></tr>
		<tr><td title="operation">Operation: [op=list]</td><td><input name="op" type="text" value="list" /></td></tr>
		<tr><td title="job">Job ID</td><td><input name="JobId" type="text" value="" /></td></tr>
		<tr><td>format [fmt=json|xml] (default:json)</td><td><select name="fmt"><option value="json">json</option><option value="xml">xml</option></select></td></tr>
		<tr><td>entity ID: [int]</td><td><input name="entity" type="text" value="32" /></td></tr>
		<tr><td>datetime: [datetime] (optional, use for "since" operation)</td><td><input name="since" type="text" value="" /></td></tr>
	</table>
	<button>Execute</button>
</form>
</body>
</html>
	$OPENPHP
}

EOH;

		$package=$lib->context->getLastResource('package',true);

		$src.= "\n$content?>";
		$src = $lib->cleanupSrc($src);
		$lib->context->saveSrc($fpath,$src);

		return $src;
	}
}

?>
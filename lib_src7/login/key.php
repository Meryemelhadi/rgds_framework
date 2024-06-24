<?php

// mode session classique
$loginPerm='default';
$loginService='user';

require_once('../../../nx/NxSite.inc');

require_once(NX_LIB."login/NxLogin_key.inc");
$login=new NxLogin_key($site);
echo $login->getCurrentKey($what=$_GET['what']);

?>

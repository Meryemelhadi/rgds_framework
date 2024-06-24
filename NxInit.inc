<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

/*
 defines a set of contants used by the API.
 Most of these constants can be overloaded in NxSite before including this file.
 The only required constants are:
 	+ NX_SITE: path to site settings.
 	+ NX_DOC_ROOT: path to web doc root (defaults to $_SERVER['DOCUMENT_ROOT']
	+ NX_DIS: path to this distribution (default to directory including this file),
	+ NX_HOST_ROOT: base for paths that need to be created automatically

*/

// ====== SITE SPECIFIC FILES (server side) =======
// NX root for this site (generated files)
//define('NX_SITE',dirname(__FILE__).'/');


if (!function_exists("json_decode"))
	define('NX_PHP_VER','');
else
	define('NX_PHP_VER','_PHP5');	

define('NX_CONF',NX_SITE.'conf/');

if (!defined('NX_GEN'))
	define('NX_GEN',NX_SITE.'gen/');

define('NX_VIEW', NX_CONF . 'view/');
define('NX_LOGIC', NX_CONF . 'logic/');

define('NX_DB',   NX_CONF . 'db/');
define('NX_DATA', NX_CONF . 'data/');

// complement http variables
if(isset($_SERVER['HTTPS']) && $_SERVER['HTTPS']=='on')
	$_SERVER['HTTP_PROTOCOL']='https';
else
	$_SERVER['HTTP_PROTOCOL']=strtolower(rtrim($_SERVER['SERVER_PROTOCOL'],'/1.02'));

$_SERVER['HTTP_SITE_URL']="{$_SERVER['HTTP_PROTOCOL']}://{$_SERVER['SERVER_NAME']}";
$_SERVER['HTTP_SITE_URL_INTERNAL']="http://{$_SERVER['SERVER_NAME']}";

if (!defined('NX_PROT'))
	define('NX_PROT', $_SERVER['HTTP_PROTOCOL']);

// logs
if (!defined('NX_BASE_LOG'))
	define('NX_BASE_LOG',NX_GEN.'logs/');
	
if (!defined('NX_LOG_FILE'))
	define('NX_LOG_FILE','phperrors.log');
	
if (!defined('NX_SESSIONS_HANDLER'))
	define('NX_SESSIONS_HANDLER',false);
//	define('NX_SESSIONS_HANDLER','MYSQL');
	
// current url
global $NX_CURRENT_URL;
$NX_CURRENT_URL=$_SERVER['REQUEST_URI'];


// ============ USER LANGUAGE ==================
if (!defined('NX_DEFT_LANG'))
	define('NX_DEFT_LANG','en');

if(!isset($NX_DEFT_LANG))
	$NX_DEFT_LANG='fr';
	
if(!isset($NX_LANG))
	$NX_LANG = getLg('en');

if(!isset($NX_CHARSET))
	$NX_CHARSET='utf-8';
	
if(!isset($NX_MIME))
	$NX_MIME='text/html';

// ====== NX DISTRIB =======
if (!defined('NX_DIS_CONF'))
	define('NX_DIS_CONF',NX_DIS.'conf/');

if (!defined('NX_LIB'))
	define('NX_LIB', NX_DIS.'lib_src/');

if (!defined('NX_XLIB'))
	define('NX_XLIB',NX_DIS.'conf/');

if (!defined('NX_XTPL'))
	define('NX_XTPL',NX_XLIB.'lib/');

if (!defined('GENERIS_PACKAGES'))
	if (defined('GENERIS_ROOT'))
		define('GENERIS_PACKAGES',GENERIS_ROOT.'packages/');

define('NX_DIS_VIEW', NX_DIS_CONF . 'view/');
define('NX_DIS_LOGIC', NX_DIS_CONF . 'logic/');
define('NX_DIS_CONTENT', NX_DIS_CONF . 'content/');

define('NX_PEAR', NX_LIB . 'pear/');
define('NX_EXT_LIB', NX_DIS . 'ext/');


// ===== CONTENT ========

// content source
define('NX_CONTENT', NX_CONF . 'content/');

// compiled views directory
if (!defined('NX_CONTENT_COMP'))
{
	if (isset($NX_DOMAIN))
		// generate content per site (specially useful for cached content from DB..)
		// define('NX_CONTENT_COMP',NX_GEN."cache/{$NX_DOMAIN}/content/");
		$GLOBALS['NX_CONTENT_COMP']=NX_GEN."cache/{$NX_DOMAIN}/content/";
	else
		//define('NX_CONTENT_COMP',NX_GEN.'content/');
		$GLOBALS['NX_CONTENT_COMP']=NX_GEN.'content/';
}

// compilation mode for content
if (!defined('NX_CHECK_CONTENT'))
	define('NX_CHECK_CONTENT','build');

if (!defined('NX_ENCODE_DATE'))
	define('NX_ENCODE_DATE',false);
	
// ====== TEMPLATE COMPILATION =======
/* compile: compile everything, all the time
   build: check dates and compile only when things have changed
   once: compile only when resources are missing
 */
if (!defined('NX_CHECK_TPL'))
	define('NX_CHECK_TPL','once');
	
/* compress js and css files */
if (!defined('NX_COMPRESS_RES'))
	define('NX_COMPRESS_RES',false);
	
// Rich text editor
if (!defined('NX_EDITOR'))
	define('NX_EDITOR','ck_editor');

// ====== WEB SERVER FILES/URLs =======
// site url path
if (!defined('NX_BASE_URL'))
	define('NX_BASE_URL','/');

if (!defined('NX_RES'))
	define('NX_RES',NX_DOC_ROOT.'nx/');

if (!defined('NX_RES_URL'))
	define('NX_RES_URL',NX_BASE_URL.'nx/');

define('NX_JS',NX_RES_URL); // obsolete ?
	
// ====== STATIC CONTENT ======
if (!defined('NX_CONTENT_IMAGES'))
{
	define('NX_CONTENT_IMAGES',NX_RES.'images/');
	define('NX_CONTENT_IMAGES_URL',NX_RES_URL.'images/');
}

// ======= USER SPACE (client files extc.)=========
// user space
if (!defined('NX_PUBLIC_USERS'))
	define('NX_PUBLIC_USERS',NX_DOC_ROOT.'users/');

if (!defined('NX_PUBLIC_USERS_URL'))
	define('NX_PUBLIC_USERS_URL',NX_BASE_URL.'users/');
	
define('NX_SITE_URL', $_SERVER['HTTP_PROTOCOL']."://{$NX_DOMAIN}");

// ======= SERVER SKINS =========
// server skin: template and views generating pages
if (!defined('NX_SKIN_SERVER'))
	if (isset($_REQUEST['skin']))
		define('NX_SKIN_SERVER',$_REQUEST['skin']);
	else
		define('NX_SKIN_SERVER','default');

// current skin root
if (!defined('NX_SKIN_ROOT'))
	define('NX_SKIN_ROOT',NX_CONF.'view/'.NX_SKIN_SERVER.'/');

// default skin root
if (!defined('NX_DEFT_SKIN_ROOT'))
	define('NX_DEFT_SKIN_ROOT',NX_CONF.'view/default/');

// distrib skin root
if (!defined('NX_DIS_SKIN_ROOT'))
	define('NX_DIS_SKIN_ROOT',NX_DIS_CONF.'view/default/');
	
// compiled views directory
if (!defined('NX_SKIN_COMP'))
	define('NX_SKIN_COMP',NX_GEN.'view/'.NX_SKIN_SERVER.'/');
	
//if (!defined('NX_DEFT_SKIN_COMP'))
//	define('NX_DEFT_SKIN_COMP',NX_GEN.'view/default/');

// ======= CLIENT SIDE SKINS =========
// client skin (css and images)
//if (!defined('NX_SKIN'))
//	if (isset($_REQUEST['cskin'])) define('NX_SKIN',$_REQUEST['cskin']);
//	else if (isset($_REQUEST['skin']))define('NX_SKIN',$_REQUEST['skin']);
//	else 
	define('NX_SKIN',NX_SKIN_SERVER);

// --- URLs for skins
define('NX_BASE_SKINS_URL',NX_RES_URL.'skins/');

define('NX_SKIN_URL',NX_BASE_SKINS_URL.NX_SKIN.'/');
define('NX_DEFT_SKIN_URL',NX_BASE_SKINS_URL.'default/');
if (!defined('NX_IMAGES'))
	define('NX_IMAGES',NX_SKIN_URL.'images/');

//---  deployment of resources on server
// current skin
define('NX_SKIN_RES',NX_RES.'skins/'.NX_SKIN_SERVER.'/');
// default skin
define('NX_DEFT_SKIN_RES',NX_RES.'skins/default/');

//images
	
// ===== FILE CACHES ====

if (!defined('NX_HTTP_Cache'))
	define('NX_HTTP_Cache',false);
else if (NX_HTTP_Cache)
{
	// location of cached html files
	// define('NX_CACHE',NX_GEN.'cache/');
	$NX_CACHE=NX_DOC_ROOT."cache/$NX_DOMAIN/";
	$NX_CACHE_ID=null;

	// desactivate default cache headers
	session_cache_limiter('none');
}

if (!defined('NX_SECTION_Cache'))
	define('NX_SECTION_Cache',true);

// ======== CHOICE LISTS (category fields)
// separator for category tree keys, ex. '?UK-WALES'
if (!defined('NX_CATEGORY_SEP'))
{
	define('NX_CATEGORY_ROOT','?');
	define('NX_CATEGORY_SEP','-');
}

//=====  basic functions (language, errors, logs) =========
// NB. to be relocated somewhere else ?

function __($s,$bundle=null) {
	global $siteDesc;
	return $siteDesc->getString($s,null,$bundle);
}

function __start($t) {
	global $siteDesc;
	if($siteDesc)
		return $siteDesc->timer()->start($t);
}

function __stop($t) {
	global $siteDesc;
	if($siteDesc)
		return $siteDesc->timer()->stop($t);
}

function getLg($d='en')
{
   
	if (isset($_GET['lang'])) {
        setcookie('lang', $d=$_GET['lang'],time()+60*60*24*30);    
        return $d;
    }
    if (isset($_COOKIE['lang']))return $_COOKIE['lang'];
	if (!$d && isset($_SERVER["HTTP_ACCEPT_LANGUAGE"]) && 
		($al=$_SERVER["HTTP_ACCEPT_LANGUAGE"]) &&
		($ls=@explode(',',$al))!==FALSE &&
		($l=@explode(';',$ls[0]))!==FALSE &&
		($lM=@explode('-',$l[0]))!==FALSE)
	 	return $lM[0];
		
	return $d;
}

function setLang($lang)
{
	global $NX_LANG;
	global $NX_CHARSET;
	$lang=$NX_LANG=getLg($lang);
	
	if ($lang=='fr')
		if ($NX_CHARSET=='utf-8')
			setlocale ( LC_ALL, "fr_FR","fr-FR", "fr_FR.utf8", "fr_FR.ISO_8859-1","fra");
		else
			setlocale ( LC_ALL, "fr_FR","fr-FR", "fr_FR.ISO_8859-1", "fr_FR.utf8","fra");
	else
		setlocale ( LC_ALL, $lang);

//	nxLog("set locale to $lang:".strftime ("%A %e %B %Y", time()));
	nxLog("set locale to $lang");
}

class NxErrorCB
{
	var $errStr='';

	function getErrorMsg()
	{
		return $this->errStr;
	}
	function toString()
	{
		return $this->errStr;
	}
	
	/**
	 * @return void
	 * @param msg string
	 * @desc add an error message
	 */
	function addError($msg,$details=null,$level='error')
	{
		if ($details != null)
		{
			$this->errStr .= '<div class="'.$level.'">'.$msg.'</div>';
			nxError("Error msg:[$msg]\n Error details:[$details]",'NxErrorCB');
		}
		else
		{
			$this->errStr .= '<div class="'.$level.'">'.$msg.'</div>';
			nxError("Error msg:[$msg]",'NxErrorCB');
		}
	}
	
	function addOtherError($errorCB)
	{
		$this->errStr.=$errorCB->errStr;
	}
	
	function isError(){	return ($this->errStr != ''); }
	function clear() {$this->errStr = '';}
	function fatal($msg,$line,$file)
	{
		$err = "FATAL ERROR: {$msg} at line:{$line} in file:{$file}";
		nxError("$err ",'NxErrorCB');
		// todo : log and or email
		die($err);
		return false;
	}
}
	
include(NX_LIB.'log/NxLog'.NX_LOG.NX_PHP_VER.'.inc');	
require_once(NX_LIB."NxProperties.inc");
require_once(NX_LIB."module/NxModule.inc");
require_once(NX_LIB."framework.inc");


class NxPortal {
	static $space;
	static function load(&$site) {
	}

	static function startSession(&$site) {
		if (NX_SESSIONS_HANDLER=='MYSQL')
		{
			require(NX_LIB."sessions/NxSessionHandler_MYSQL.inc");
			$sessionHandler=new NxSessionHandler($site);
		}
		
		//$userPlug =& $_NX_site->getPlugIn("User");
		//$userPlug->login($loginPerm);

		/*
		$sn=session_name();
		if(isset($_GET[$sn])) if(strlen($_GET[$sn])!=32) unset($_GET[$sn]);
		if(isset($_POST[$sn])) if(strlen($_POST[$sn])!=32) unset($_POST[$sn]);
		if(isset($_COOKIE[$sn])) if(strlen($_COOKIE[$sn])!=32) unset($_COOKIE[$sn]);
		if(isset($PHPSESSID)) if(strlen($PHPSESSID)!=32) unset($PHPSESSID);
		*/
		session_start();

		// avoid session capture
/*		if (!isset($_SESSION['initiated']))
		{
			session_regenerate_id();
			$_SESSION['initiated'] = true;
		}
*/

	}

	static function login(&$site,$user_var,$loginService) {

		if (!isset($_SESSION[$user_var]) || !$_SESSION[$user_var] || isset($_REQUEST['mkey']))
		{
			if (isset($_REQUEST['ukey']))
			{
				// load user context from key
				require_once(NX_LIB."login/NxLogin_key.inc");
				$login=new NxLogin_key($site,null,false,true);
				if(($_SESSION[$user_var]=$login->loadUserContext($_REQUEST['ukey'],$loginService))==null)
					exit();			
			}
			elseif (isset($_REQUEST['mkey']))
			{
				// load user context from key
				require_once(NX_LIB."login/NxLogin_key.inc");
				$login=new NxLogin_key($site);
				if(($_SESSION[$user_var]=$login->loadUserContext($_REQUEST['mkey'],$loginService))==null)
					exit();			
			}
			elseif (isset($_REQUEST['skey']))
			{
				// load user context from key
				require_once(NX_LIB."login/NxLogin_keySite.inc");
				$login=new NxLogin_keySite($site,null,false);
				if(($_SESSION[$user_var]=$login->loadUserContext($_REQUEST['skey'],$loginService))==null)
					exit();			
			}
			elseif (isset($_REQUEST['dkey']))
			{
				// load user context from key
				require_once(NX_LIB."login/NxLogin_keyDir.inc");
				$login=new NxLogin_keyDir($site,null,true,false);
				if(($_SESSION[$user_var]=$login->loadUserContext($_REQUEST['dkey'],$loginService))==null)
					exit();			
			}
			else 
            {
                if(preg_match('%Microsoft Office (Excel|Word)%i',$_SERVER['HTTP_USER_AGENT'])&& !isset($_GET['incall']))
                {
                    $_SESSION['incall']='office';
                    header("Location:{$_SERVER['HTTP_SITE_URL']}{$_SERVER['REQUEST_URI']}&incall=office");
                    exit();
                }
                elseif(isset($_GET['incall']) && !isset($_GET['incallend']) || (isset($_SESSION['incall'])))
                {
                    echo '<html><title>redirecting</title><head><script>window.location="'.$_SERVER['REQUEST_URI'].'&incallend=1";</script></head><body>redirecting...</body><html>';
                    exit();
                }
                    
				$page=urlencode($_SERVER['REQUEST_URI']);
				if(NX_AUTH_METHOD=='kerberos')
					header("Location:{$_SERVER['HTTP_SITE_URL']}/$loginService/login.php?nxs=kerberos&page={$page}");
				else
					header("Location:{$_SERVER['HTTP_SITE_URL']}/$loginService/login.php?page={$page}");
				exit();
			}
		}
		elseif(isset($_SESSION['NX_PAGE_KEY']))
		{
			// we are logged by page key
			require_once(NX_LIB."login/NxLogin_key.inc");
			$login=new NxLogin_key($site,null,false,isset($_REQUEST['ukey']));
			if (!$login->checkKeyAccess($_SESSION['NX_PAGE_KEY'],$loginService))
			{
				// outside the box => login
				session_destroy();
				$page=urlencode($_SERVER['REQUEST_URI']);
				if(NX_AUTH_METHOD=='kerberos')
					header("Location:{$_SERVER['HTTP_SITE_URL']}/$loginService/login.php?nxs=kerberos&page={$page}");
				else
					header("Location:{$_SERVER['HTTP_SITE_URL']}/$loginService/login.php?page={$page}");
				exit();
			}
		}
		elseif(isset($_SESSION['NX_SITE_KEY']))
		{
			// we are logged by page key
			require_once(NX_LIB."login/NxLogin_keySite.inc");
			$login=new NxLogin_keySite($site,null,false);
			if (!$login->checkKeyAccess($_SESSION['NX_SITE_KEY'],$loginService))
			{
				// outside the box => login
				session_destroy();
				$page=urlencode($_SERVER['REQUEST_URI']);
				if(NX_AUTH_METHOD=='kerberos')
					header("Location:{$_SERVER['HTTP_SITE_URL']}/$loginService/login.php?nxs=kerberos&page={$page}");
				else
					header("Location:{$_SERVER['HTTP_SITE_URL']}/$loginService/login.php?page={$page}");
				exit();
			}
		}
		
		// stop recursion in session
		$_SESSION[$user_var]->root=null;

		return $GLOBALS['_NX_user']=$GLOBALS[$user_var]=clone $_SESSION[$user_var];
	}

	static function space($prop=null) {
		return null;
	}

	static function checkPerm(&$site,&$user,$perm,$credit=null) {
//		if (!$GLOBALS[$user_var]->checkPerm($loginPerm,$GLOBALS[$user_var]->getProperty('user.groups','user')))

		if (!$user->checkPerm($perm,$credit))
		{
			echo $site->getString('prohibited page',null,'user');
			nxError("permission[$loginPerm] not granted for user[".$user->getProperty('user.userID')."]");
			die(0);
		}
	}

}

// ------------ register site properties and manage session, user etc. -----------
function & NxSite_init($siteProps)
{
	global $_NX_site;
	global $withSession;
	
	$dummyDesc = null;	
	$_NX_site = new NxModule($siteProps,$dummyDesc);
	
	// obsolete ?
	// $_NX_site->setProperty("fileStore",new NxFileStore($siteDesc));

	if (defined('NX_HTTP_Cache') && NX_HTTP_Cache)
		$withSession=false;
	else if(!isset($withSession))
		$withSession=true;

	// init lang (nx and locale)
	// if (function_exists('date_default_timezone_set'))
		date_default_timezone_set($_NX_site->getProperty('time_zone','GMT'));
	
	global $NX_LANG;
	setLang($_NX_site->getProperty('lang',$NX_LANG));

	$portalClass = $_NX_site->getProperty('portal.class','NxPortal');
	$portal=new $portalClass();
		
	global $loginPerm;
	if (isset($loginPerm))
	{
		global $loginService;
		if (!isset($loginService))
		{
			$loginService='login';
			$user_var='_NX_user';
		}
		else
			$user_var="_NX_$loginService";

		$portal->startSession($_NX_site);
		$user = $portal->login($_NX_site,$user_var,$loginService);
		$portal->checkPerm($_NX_site,$user,$loginPerm,null);
		
		$_NX_site->addProperties($GLOBALS[$user_var]->props);
		
		$withSession=true;
	}
	else if($withSession)
	{
		$portal->startSession($_NX_site);
		$portal->load($_NX_site);
	}

	// expand app param
	if (isset($_REQUEST['nxp']))
		list($_REQUEST['nxo'],$_REQUEST['nxs'])=explode('.',$_REQUEST['nxp']);
	
	// stack management
	if (!defined('NX_STACK_RESET'))
		define('NX_STACK_RESET',false);

	if (!defined('NX_STACK_ENABLE'))
		define('NX_STACK_ENABLE',false);
		
	// navigation stack
	if (NX_STACK_ENABLE) {
		require_once(NX_LIB."module/NxFrameStack.inc");
		global $NX_STACK;
		$NX_STACK = new NxFrameStack($_NX_site,$withSession,NX_STACK_RESET);
	}

	// no need to write to session...
	if(defined('KEEP_SESSION') && KEEP_SESSION==false)
		session_write_close();

	return $_NX_site;
}

// path list for finding XML files
if (!defined('NXL_ROOTS'))
	define('NXL_ROOTS',NX_HOST_ROOT.'nxl/,'.NX_DIS.'nxl/,'.NX_DIS.'taglib/');

// current skin root
if (defined('NX_AUTO_BUILD_NXL') && NX_AUTO_BUILD_NXL)
{
	$url=$_SERVER['SCRIPT_NAME'];
	$baseName=ltrim(preg_replace('/[.]php?.*$/i','',$url),'/');

	
	$php_file=NX_DOC_ROOT.$baseName.'.php';
	$nxl_file=NX_HOST_ROOT.'nxl/'.$baseName.'.nxl';

	if ((!is_file($php_file) && is_file($nxl_file)) || @filemtime($nxl_file) > @filemtime($php_file))
	{
		$_GET['init']='create';
		$_GET['deploy']='yes';
		$_GET['url']=$baseName.'.nxl';

		include NX_TAG_LIB.'compile.php';
		exit();
	}
}

// ============ SECURITE =========

/* protection against XSS */
if (isset ($_SERVER['ORIG_PATH_INFO']) && $_SERVER['ORIG_PATH_INFO'] != $_SERVER['PHP_SELF']) {
    $_SERVER['PATH_INFO'] = $_SERVER['ORIG_PATH_INFO'];
}

// Security measure, to avoid XSS exploit.
if (!empty ($_SERVER['PATH_INFO']) && strrpos ($_SERVER['PHP_SELF'], $_SERVER['PATH_INFO'])) {
    $_SERVER['PHP_SELF'] = substr ($_SERVER['PHP_SELF'], 0, -(strlen ($_SERVER['PATH_INFO'])));
}

function XfilterPar($k) {if(isset($_GET[$k])) {	$_REQUEST[$k]=$_GET[$k]=preg_replace('/[^a-z0-9_A-Z:@]+/','_*_',$_GET[$k]);}}
function XfilterParOid($k) {if(isset($_GET[$k])) {	$_REQUEST[$k]=$_GET[$k]=preg_replace('/[^0-9_-]+/','_*_',$_GET[$k]);}}
XfilterPar('nxo');
XfilterPar('nxs');
XfilterPar('oid');
?>
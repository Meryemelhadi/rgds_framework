<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
// ====== DEVICE SENSITIVE SETTINGS =======
if (!isset($NX_MAP_DEVICE))
	$NX_MAP_DEVICE=array('GK'=>1,'IE'=>1,'IE55'=>1,'OP'=>1,'IE7'=>1,'iPhone'=>1,'IE60'=>1,'IE70'=>1,'IE80'=>1);

if (!defined('NX_DEVICE'))
	if (isset($_REQUEST['device']))
	{
		define('NX_DEVICE',$_REQUEST['device']);
		define('NX_DEVICE_VER','');
	}
	else if ($dev=strstr($_SERVER['HTTP_USER_AGENT'], 'Opera'))
	{
		define('NX_DEVICE','OP');
		define('NX_DEVICE_VER','');
	}
	else if ($dev=strstr($_SERVER['HTTP_USER_AGENT'], 'MSIE'))
	{
		define('NX_DEVICE','IE');
		if (($ver_h=substr($dev,5,1))>='8')
			define('NX_DEVICE_VER','80');
		elseif (($ver_h=substr($dev,5,1))>='7')
			define('NX_DEVICE_VER','70');
		elseif (($ver_h=substr($dev,5,1))>='6')
			define('NX_DEVICE_VER','60');
		elseif ((($ver_h=substr($dev,5,1))>='5' || ($ver_h == '5' && substr($dev,7,1)>'5')))
			define('NX_DEVICE_VER','55');
		else
			define('NX_DEVICE_VER','40');
	}
	else if (strstr($_SERVER['HTTP_USER_AGENT'], 'Gecko'))
	{
		define('NX_DEVICE','GK');
		define('NX_DEVICE_VER','');
	}
	else if (strstr($_SERVER['HTTP_USER_AGENT'], 'Mozilla'))
	{
		define('NX_DEVICE','N4');
		define('NX_DEVICE_VER','');
	}
	else if (strstr($_SERVER['HTTP_USER_AGENT'], 'Wap'))
	{
		define('NX_DEVICE','WAP');
		define('NX_DEVICE_VER','');
	}
	else if (strstr($_SERVER['HTTP_USER_AGENT'], 'iPhone'))
	{
		define('NX_DEVICE','iPhone');
		define('NX_DEVICE_VER','');
	}
	else
	{
		define('NX_DEVICE','');
		define('NX_DEVICE_VER','');
	}

define("NX_DEVICE_FULL",NX_DEVICE.NX_DEVICE_VER);
if (isset($NX_MAP_DEVICE[NX_DEVICE_FULL]) && $NX_MAP_DEVICE[NX_DEVICE_FULL])
{
	define('NX_MAP','full_map');
	define('NX_DEVICE_MAP',NX_DEVICE_FULL);
	define('_NX_DEVICE_MAP','_'.NX_DEVICE_FULL);
}
elseif (isset($NX_MAP_DEVICE[NX_DEVICE]) && $NX_MAP_DEVICE[NX_DEVICE])
{
	define('NX_MAP','device_only_map');
	define('NX_DEVICE_MAP',NX_DEVICE);
	define('_NX_DEVICE_MAP','_'.NX_DEVICE);
}
else
{
	define('NX_MAP','no_map');
	define('NX_DEVICE_MAP','');
	define('_NX_DEVICE_MAP','');
}

class NxApp_View extends NxProperties 
{
	var $viewName;
	var $ui;
	var $rootPack;
	
	function __construct($viewName,$props,&$parentDesc,$propertyBase="")
	{
		parent::__construct($props,$parentDesc);

		// get name@$root
		$_name = explode('@',$viewName);
		if(count($_name)>1)
		{
			$viewName=$_name[0];
			$root="packages/{$_name[1]}/";
		}
		elseif($package=$this->getProperty('nx.package'))
			$root="packages/{$package}/";
		else
			$root='';
		
		$this->rootPack = $root;

		$this->viewName = $viewName;
		$this->ui = $this;
		
		// property base path for this view (prefix for all property names in templates)
		$this->objectBase=$propertyBase;

		$skin=$this->getProperty('page.skin',NX_SKIN_SERVER);
		$skinDeft=$this->getProperty('page.skin.default','default');
		
		// load skin paths
		$this->initSkin($skin,$skinDeft);
	}
	
	function & getParam($name,$deft=null) {
		return $this->getProperty($name,$deft,false);
	}
	
	/* run/display a view. Search its source from local site specific views and
	   then in nx prebuilt views. If device is specified (auto or IE55 for ex.)
	   the view name is postfixed by device code (IE,IE55,N4,GK,WP,XML).
	   If skin is specified, local path is prefixed by the skin name found in 
	   $NX_SKIN_SERVER.
	*/
	function run($device="auto",$skin="auto",$ispage=false) 
	{
		$viewName = $this->viewName;
		
		// if page, send headers
		$this->ispage=$ispage;
		
		$this->setProperty('page.view',$this->viewName=$actualView = $this->getProperty($viewName,$viewName,true));

		// recompose local path
		$path = strtr($actualView,'.','/');
		
		// setup local variables for view (check which ones are actually needed ?)
		$module		 =& $this;
		$application =& $this->parent;
		$properties  =& $this->props;

		// check device
		if ($device=="auto")
			$device=_NX_DEVICE_MAP;
		elseif ($device!="") 
			$device = "_$device";
            
        $props = $this->props;
        $parent = $this->parent;
     
		// get file from include path (skin and default skin)
 		$f=$this->getViewFile($path,$device);
		if ($f)
		{
			return $this->exec_view($f,$ispage);
		}
		elseif ($f=$this->getViewFile('page/default',$device))
		{
			nxError("cant run view:$viewName mapped to $actualView, use alternative: page.default","NxApp_ViewModule");
			return $this->exec_view($f,$ispage);
		}
		else {
			nxError("cant run view:$viewName mapped to $actualView, NB. no default page provided","NxApp_ViewModule");
			return null;
		}				
	}
	
	function exec_view($f,$ispage) {
		nxLog("run view: $f","NxApp_ViewModule",NX_DEBUG);
		
		// if page, send headers
		if ($ispage) {
			$this->getHeaders();
		}

		include($f);

		if(isset($properties))
			return $properties;		
		else
			return null;
	}
	
	function getHeaders() {
		global $NX_MIME,$NX_CHARSET;
		header("Content-Type: {$NX_MIME}; charset={$NX_CHARSET}"); 		
	}
	
	function getViewFile($path,$device)
	{
		global $NX_SKIN_COMP,$NX_SKIN_PATH;
		
		// search for views in view repository except if we want to check tpl file anytime
		if (NX_CHECK_TPL!='compile')
			// the "one" is in his skin
			if (is_file($f=$NX_SKIN_COMP.$path."$device.inc"))
				return $f;

		// load skin paths for building views
		$this->initSkinPaths();

		$tplPlugdft = $this->parent->getPlugin('Template','NxPlug_','mvc/TPL');				 

		// file not found => search it in source directories and compile it
		//$rootPaths= $NX_SKIN_ROOT.';'.$NX_DEFT_SKIN_ROOT.';'.$NX_DIS_SKIN_ROOT;
		
		// resources needed by the view, starts from html template
		$resources[] = array('name'=>$path,"type"=>"tpl","device"=>$device,"isDevice"=>true,"isSkin"=>true,
			"propBase"=>$this->objectBase,'ispage'=>$this->ispage);
		
		// search tpl file in rootPaths and compile it (or copy .inc filer), and then process dependencies.
		// NB. each processed file adds up its own dependencies in the ressources array.
		$i=0;
		while(isset($resources[$i]))
		{
			$desc = $resources[$i];
			$desc["device"]=$device;

			// get a template handler for this type of resource
			if ($tplPlug = $this->parent->getPlugin('Template_'.$desc['type'],'NxPlug_','mvc/TPL'))
			{
				$dest=$tplPlug->build($this,$NX_SKIN_PATH,$NX_SKIN_COMP,$desc,$resources,NX_CHECK_TPL);
				$resources[$i]["dest"]=$dest;
			}
			else 
			{
				// no template handler => only deploy required files.
				$dest=$tplPlugdft->deployRes($desc);
				$resources[$i]["dest"]=$dest;
			}
			$i++;
		}
				
		// return the top view
		return $resources[0]['dest'];
	}

	// create a button to be used in a view
	/**
	 * @return html code for the button
	 * @param name button name
	 * @param url = null link url (default: get property $name.url
	 * @param urlParams = null url parameters to add to the url (url encoded)
	 * @desc add a button link
	 */
	function getButton($name,$url=null,$urlParams=null)
	{
		return '';
	}
	
	function getFormHeader(&$recDesc, $nextStepUrl, $nxs=null, $nxo=null)
	{			
		return '';
	}
	
	function getStyle($element=null,$className=null,$deft="")
	{		
		return '';
	}

	// adds the resource full name (w/ skin,device and extension) to the list of resources in 
	// page.head.$ext.req
	function requireRes($ext,$name,$isSkin=true,$isDevice=false,$propName="",$isStatic=false)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		
		nxLog("RequireRes: isSkin=$isSkin, Default skin=$NX_DEFT_SKIN_URL, current skin=$NX_SKIN_URL",'APP_VIEW');
		
		if ($isSkin)
			$skin=$NX_SKIN_URL;
		else
			$skin=$NX_DEFT_SKIN_URL;
			
		if ($isDevice=="true")
			$suffix=_NX_DEVICE_MAP;
		else
			$suffix="";
			
		if ($ext == "csx")
			; //$ext="css";
		elseif ($ext=="jsx")
			$ext="js";
		
		if ($propName=="")
			$propName="page.head.$ext.req";
			
		if ($isStatic=="true" || $isStatic===true)
			$propName.=".static";
			
		// add fully specified css path
		$p = $propName;
		nxLog("Required resource:$name.$ext, device=$isDevice skin=$isSkin static=$isStatic prop=$p => {$skin}{$name}{$suffix}.{$ext}","VIEW");
		nxLog("Required resource:setRootProperty:$p = ".$this->getProperty($p,'').",{$skin}{$name}{$suffix}.{$ext}","VIEW");
		$this->setRootProperty($p,$this->getProperty($p,'').",{$skin}{$name}{$suffix}.{$ext}",true,true);
	}

	function requireCss($name,$isSkin=true,$isDevice=false,$propName="page.head.css",$isStatic=false)
	{
		$this->requireRes("css",$name,$isSkin,$isDevice,$propName,$isStatic);
	}

	// deprecated
	function getSyleSheets($propName="",$isSkin=true)
	{
		return $this->getStyleSheets($propName,$isSkin);
	}
	
	function getStyleSheets($propName='',$isSkin=true)
	{
		global $NX_SKIN_URL,$NX_DEFT_SKIN_URL;
		
		if ($propName=='')
			$propName="page.head.css";
			
		if ($isSkin)
			$skin=$NX_SKIN_URL;
		else
			$skin=$NX_DEFT_SKIN_URL;
					
		$css_sheets=array();
		// add css that were in page.head.css prop
		foreach (explode(",",$this->getProperty($propName,'')) as $u)
			if ($u!='')
				$css_sheets[]=$skin.$u;
				
		// add css that were in page.head.css.req prop
		// add static css that were in page.head.css.req.static prop
		$css_sheets += 
			explode(',',
				trim(
					$this->getProperty($propName.'.req','').$this->getProperty($propName.'.req.static',''),',')
				);

		// cleanup static list to avoid doing it twice
		$this->setRootProperty($propName.'.req.static','');
		
		return $this->_getStyleSheets(array_unique($css_sheets));	
	}

	function requireJs($name,$isSkin=true,$isDevice=false,$propName="page.head.js",$isStatic=false)
	{
		$this->requireRes('js',$name,$isSkin,$isDevice,$propName,$isStatic);
	}
	
	function getScripts($propName="",$path=null)
	{
		if ($propName=='')
			$propName="page.head.js";		
		
		if ($path == null)
			$path = NX_JS;
			
		$scripts=array();
		
		// add scripts that were in page.head.css prop
		foreach (explode(",",$this->getProperty($propName,"")) as $u)
			if ($u!='')
				$scripts[]=$path.$u;
				
		// add scripts that were in page.head.css.req prop
		// add scripts that were in page.head.css.req.static prop
		$scripts += 
			explode(',',
				trim(
					$this->getProperty($propName.'.req','').$this->getProperty($propName.'.req.static',''),',')
				);

		// cleanup static list to avoid doing it twice
		$this->setRootProperty($propName.'.req.static','');
		
		return $this->_getScripts(array_unique($scripts));
	}

	function getHeads($propName="",$path=null)
	{
		if ($propName=="")
			$propName="page.modules.heads";		
	
		$s = implode($this->root->getProperty($propName,array(),false),"\n");
			
		return $s;
	}
	
	function getForm(&$record, $recProp, $recsProp)
	{
		static $form_id=0;
		
		if ($recProp!='')
		{
			$propName = $recProp;
			$submitButton=$this->_getSubmitButton($recProp.'.form.submit');
		}
		else
		{
			$propName=$recsProp;
			if ($recsProp=='records')
			{
				// for compatibility with old applications...
				$submitButton=$this->getProperty('submitButton');
				if ($submitButton==null)
					$submitButton=$this->_getSubmitButton('submit');
			}
			else
				// new way to describe the submit button
				$submitButton=$this->_getSubmitButton($this->viewName.'.form.submit');
		}
		
		if (!($method=$this->getProperty($this->viewName.".form.method")))
			$method=$this->getProperty("submit.form.method","POST");
					
		$action=$this->getProperty($recProp.".form.action.url",
				$this->getProperty($this->viewName.".form.action",
					$this->getProperty('submit.url',
						$this->getProperty('search.url',"#"))));
		$this->setProperty('submit.url',$action);

		if (!$record)
		{
			nxError('no record in getForm');
			return '';
		}
			
		$recDesc = $record->getRecDesc();
		$formName = "form_" . strtr($recDesc->getProperty('name','',false),'.-','__');

		if ($form_id++>0)
		{
			$formName .= (string)$form_id;
		}

/*
		if (!isset($this->formID))
			$this->formID = 0;
		else
		{
			$this->formID++;
			$formName .= (string)$this->formID;
		}
*/
		$this->setProperty('submit.id',$formName);

		//ME
		$s ="";

		$s .= "<script>\n  window.check_".$formName."=function(frm) {\nvar error = \"\"; ";

		// scan fields:
		//  + get total max "file" size,
		//  + get hidden fields
		//  + get "help" in fields
		//  + add onSubmit handler for each field
		$fileSize = 0;
		$hiddenFields = "";
		$isHelp=false;
		$isRequired=false;
		$urlKey='';
		$errorFound =  false;
		foreach($record->fields as $fname=>$f)
	 	{
			$fdesc = & $f->desc;
			if (isset($fdesc->props["isFile"]) && $fdesc->props["isFile"] == true)
			{
				$fileSize += (int)$fdesc->getProperty("maxFileUploadSize", 1000000);
			}

			$check = $f->onSubmitCheck("frm");
			if ($check!='')
			{
				$s .= "error += ".$check.";";
				$errorFound = true;
			}
			
			if ($f->isHidden())
				$hiddenFields .= $f->toForm();
			else
			{
				if ($fdesc->getProperty("help",null,false)!=null)
					$isHelp=true;
					
				if ($fdesc->getProperty("isRequired",false,false)!=null)
					$isRequired=true;
			}
				
			if ($f->isKey())
				$urlKey .= '&'.$f->toUrlParam();		
		}
		
		// add record key to action (=> unique url per record and easier to use keys in DML)
		$action .= $urlKey;

		$s .= "if (error!=\"\") { alert(Nx_res.getString(\"error, please check following fields\")+\":\"+ error); return false;} return true;\n}</script>\n";

		//ME -> nécessite du css, à vérifier...
		// $s .= "if (error!=\"\") {document.getElementById('modal_body').innerHTML = \"<strong>\"+Nx_res.getString(\"error, please check following fields\")+\":\"+\"</strong><br>\"+ error ; document.getElementById('modal').style.display= 'block'; document.getElementById('modal').classList.add('in'); return false;} return true;\n}</script>\n";
		
		if ($errorFound){
			$s .='<div class="modal fade" id="modal" style="display:none">
				<div class="modal-dialog" role="document">
					<div class="modal-content">
						<div class="modal-header">
							<button type="button" class="close" data-dismiss="modal" aria-label="Close" onclick="document.getElementById(\'modal\').style.display=\'none\'">
								<span aria-hidden="true">&times;</span>
							</button>
							<h4 class="modal-title">Attention</h4>
						</div>
						<div class="modal-body" id="modal_body"></div>
						<div class="modal-footer">
							<button type="button" class="btn btn-warning" onclick="document.getElementById(\'modal\').style.display=\'none\'">OK</button>
						</div>
					</div>
				</div>
			</div>';
		}
		$s .= "<form name=\"{$formName}\" id=\"{$formName}\" onsubmit=\"return check_{$formName}(this);\" action=\"{$action}\" method=\"{$method}\"";
//		if (($method=strtoupper($method))=="POST" && $fileSize > 0)
		if (($method=strtoupper($method))=="POST")
		{
			$s .= " ENCTYPE=\"multipart/form-data\">";
			// workaround php4 bug: MAX_FILE_SIZE not handled properly. 
			// Besides IE6 doesn't support it either... => set it to 100MB
			// $fileSize=500000000;
			$fileSize=$this->getProperty('maxFileUploadSize',500000000);
			$s .= "<input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"".$fileSize."\" /";
		}
		
		$s .= ">";
		if($method=='GET')
		{
			if ($nxs=$this->getProperty('form.nxs'))
				$s .= '<input type="hidden" name="nxs" value="'.$nxs.'"/>';
			if ($nxo=$this->getProperty('form.nxo'))
				$s .= '<input type="hidden" name="nxo" value="'.$nxo.'"/>';

			$params = $this->url2Array($action);
			$params['mode'] = 'page';
			foreach($params as $k=>$v)
			{
				$s .= "<input type=\"hidden\" name=\"{$k}\" value=\"{$v}\" />\n";
			}
		}


		$s .= '<input type="hidden" name="_form_method_" value="'.$method.'"/>';

		// add hasChanged functionality
$hasChanged = <<<EOF
<script>
  var currentForm = document.getElementById('{$formName}');
  if(currentForm) {
	  currentForm.etatinitial = currentForm.cloneNode(true);
	  currentForm.haschanged = function() {
	   if (typeof this.etatinitial == "undefined" ) { return false; }
	   var initialform = this.etatinitial;
	   for(var i = 0; i < initialform.elements.length; i++ )
		if( initialform.elements[i].value != this.elements[i].value )
		 return true;
	   return false;
	  }
  }
 </script>
EOF;

		$form = array('header'=>$s, 'hiddenFields'=>$hiddenFields, 
					  'isHelp'=>$isHelp, 'isRequired'=>$isRequired, 'submit'=>$submitButton,'name'=>$formName,'hasChanged'=>$hasChanged);	
			
		return $form;
	}
	
	function url2Array($url){
		//Extract the Querystr pos after ? mark
		$startpos = stripos($url, '?');
		if(!$startpos)
			$startpos = stripos($url, '#');

		//Extract the Querystr alone
		$qryStr=substr($url, $startpos+1) ;

		//Split the querystring into & pairs
		$qryvalues=explode('&', str_replace('&amp;','&',$qryStr));

		foreach($qryvalues as  $value)
		{
			//Split the single data into two i.e data   |  value
			$buffer=explode('=', $value);

			// Urlencode only the value now , Change it to rawurlencode if necessary
			$par[$buffer[0]]=$buffer[1];
		}

		return $par;
	}

	// get a record from DB and sets it to a property
	function & getRecords($view=null,$propName=null,$media='DB')
	{
		// get data source plugin
		$ds =& $this->getPlugIn($media);
		
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","MODULE");
			return null;
		}
		
		// get view
		if ($view===null)
			$view=$this->view;

		// get records
		$records = & $ds->getRecords($view, $this->error);

		// return record collection if ok and set property
		if ($records != null) // && $records->count() > 0)
			$this->setProperty($propName,$records);

		return $records;
	}	
	
	// static function that initialises the view skin
	function initSkin($skin,$skinDeft) {
		global $NX_SKIN,$NX_SKIN_DFT,$NX_SKIN_COMP,$NX_SKIN_PATH,$NX_SKIN_URL,$NX_DEFT_SKIN_URL,
				$NX_IMAGES,$NX_SKIN_RES,$NX_DEFT_SKIN_RES,$NX_SKIN_SAVE;

		if (isset($NX_SKIN))
			return;

		$NX_SKIN_SAVE=null;

		if (defined('NX_AUTO_SKIN') && NX_AUTO_SKIN)
		{
			if (isset($_GET['skin']))
			{
				$NX_SKIN=$_COOKIE['skin']=$_SESSION['skin']=$_GET['skin'];
			}
			elseif (isset($_SESSION['skin']))
				$NX_SKIN=$_SESSION['skin'];
			elseif (isset($_COOKIE['skin']))
			{
				$NX_SKIN=$_SESSION['skin'];
			}
			else
				$NX_SKIN=$skin;
		}
		else
			$NX_SKIN=$skin;
			
		$NX_SKIN_DFT=$skinDeft;
		
//		$NX_SKIN=$this->getProperty('page.skin',NX_SKIN_SERVER);
//		$NX_SKIN_DFT=$this->getProperty('page.skin.default','default');
		$NX_DIS_SKIN_ROOT=NX_DIS_CONF.'view/default/';

		// compiled views storage
		$NX_SKIN_COMP=NX_GEN."view/{$NX_SKIN}/";

		// --- URLs for skins
		$NX_SKIN_URL=NX_RES_URL."skins/{$NX_SKIN}/";
		$NX_DEFT_SKIN_URL=NX_RES_URL."skins/{$NX_SKIN_DFT}/";
		$NX_IMAGES="{$NX_SKIN_URL}images/";
	
		//---  deployment of resources on server
		// current skin
		$NX_SKIN_RES=NX_RES."skins/{$NX_SKIN}/";
		// default skin
		$NX_DEFT_SKIN_RES=NX_RES."skins/{$NX_SKIN_DFT}/";	

	}
				
	function initSkinPaths() {
		global $NX_SKIN,$NX_SKIN_DFT,$NX_SKIN_COMP,$NX_SKIN_PATH,$NX_SKIN_URL,$NX_DEFT_SKIN_URL,
				$NX_IMAGES,$NX_SKIN_RES,$NX_DEFT_SKIN_RES,$NX_SKIN_PACKAGE,$NX_SKIN_SAVE;

		if (isset($NX_SKIN_PACKAGE) && $NX_SKIN_PACKAGE==$this->rootPack && $NX_SKIN_SAVE==$NX_SKIN)
			return;

		$NX_SKIN_PACKAGE=$root=$this->rootPack;
		$NX_SKIN_SAVE=$NX_SKIN;

		$skin = $NX_SKIN;
		$skinDeft = $NX_SKIN_DFT;

		// standard paths
		if (defined('NX_SKIN_ROOTS'))
			$skinRoots = explode(',',NX_SKIN_ROOTS);
		else
			$skinRoots[] = NX_CONF;

		// $skinRoot=NX_CONF.'view/';	
		$skins = explode('.',$skin);
		$skins[]='default';

		// search first package paths
		if ($root)
		{
			foreach($skins as $sk)
			{
				foreach($skinRoots as $skinRoot)
				{
					$skinRootV=$skinRoot."view/";
					$skinRootP=$skinRoot."{$root}view/";
					$skinRootP2=$skinRoot."{$root}conf/view/";
					$paths[] = array(
						"{$skinRootP}{$sk}/",	// package : pack/view
						"{$skinRootP2}{$sk}/",	// package : pack/conf/view
						"{$skinRootV}{$sk}/",	// main config : conf/view/
					);
				}
			}
		}
		else {
			// not in package
			foreach($skins as $sk)
			{
				foreach($skinRoots as $skinRoot)
				{
					$skinRootV=$skinRoot."view/";
					$paths[] = array(
						"{$skinRootV}{$sk}/"	// current skin root
					);
				}
			}
		}

		$paths[] = array(
			NX_XTPL, // ORM LIB
			NX_DIS_CONF."view/{$NX_SKIN_DFT}/" // distrib skin root
		);
		if ($NX_SKIN_PATH)
			$paths[] = $NX_SKIN_PATH;

		$NX_SKIN_PATH=call_user_func_array('array_merge',$paths);
	}


}

?>
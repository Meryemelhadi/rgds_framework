<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
class NxApp_ViewModule extends NxProperties 
{
	var $viewName;
	
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
		else
			$root='';
		
		$this->viewName = $viewName;
		$ui='NxUI_'.$this->getProperty('view.type','XHTML');
		if (!class_exists($ui))
			include_once("$ui.inc");

		$this->ui = new $ui($this);
		
		// property base path for this view (prefix for all property names in templates)
		$this->objectBase=$propertyBase;

		$skin=$this->getProperty('page.skin',NX_SKIN_SERVER);
		$skinDeft=$this->getProperty('page.skin.default','default');
		
		// load skin paths
		$this->initSkin($skin,$skinDeft,$root);
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
	function & run($device="auto",$skin="auto",$ispage=false) 
	{
		$viewName = $this->viewName;
		$this->ispage=$ispage;
		
		// add default category to view name if none (obsolete)
		/*
		if (!is_integer(strpos($viewName, '.'))) 
		   // no prefix => add default (for backward compatibility...)
			$viewName = "view.".$viewName;
		*/	
		$this->setProperty('page.view',$this->viewName=$actualView = $this->getProperty($viewName,$viewName,true));

		// recompose local path
		$path = strtr($actualView,'.','/');
		
		// setup local variables for view (check which ones are actually needed ?)
		$module		 =& $this;
		$application =& $this->parent;
		$ui = $this->ui;
		$properties  =& $this->props;

		// check device
		if ($device=="auto")
			$device=_NX_DEVICE_MAP;
		elseif ($device!="") 
			$device = "_$device";

		// get file from include path (skin and default skin)
		if ($f=$this->getViewFile($path,$device))
		{
			nxLog("run view: $f","NxApp_ViewModule",NX_DEBUG);
			include($f);
			return $properties;
		}
		elseif ($f=$this->getViewFile('page/default',$device))
		{
			nxError("cant run view:$viewName mapped to $actualView, use alternative: page.default","NxApp_ViewModule");
			nxLog("run view: $f","NxApp_ViewModule",NX_DEBUG);
			include($f);
			return $properties;
		}
		else {
			nxError("cant run view:$viewName mapped to $actualView, NB. no default page provided","NxApp_ViewModule");
			return null;
		}				
	}
	
	function getViewFile($path,$device)
	{
		global $NX_SKIN_COMP,$NX_SKIN_PATH;
		
		// search for views in view repository except if we want to check tpl file anytime
		if (NX_CHECK_TPL!='compile')
			// the "one" is in his skin
			if (is_file($f=$NX_SKIN_COMP.$path."$device.inc"))
				return $f;
					 
		// file not found => search it in source directories and compile it
		//$rootPaths= $NX_SKIN_ROOT.';'.$NX_DEFT_SKIN_ROOT.';'.$NX_DIS_SKIN_ROOT;
		
		// resources needed by the view, starts from html template
		$resources[] = array('name'=>$path,"type"=>"tpl","device"=>$device,"isDevice"=>true,"isSkin"=>true,
			"propBase"=>$this->objectBase,'ispage'=>$this->ispage);
		
		// search tpl file in rootPaths and compile it (or copy .inc filer), and then process dependencies.
		// NB. each processed file adds up its own dependencies in the ressources array.
		$i=0;
		$tplPlugdft =& $this->parent->getPlugin('Template','NxPlug_','mvc/TPL');
		while(isset($resources[$i]))
		{
			$desc = $resources[$i];
			$desc["device"]=$device;

			// get a template handler for this type of resource
			if ($tplPlug =& $this->parent->getPlugin('Template_'.$desc['type'],'NxPlug_','mvc/TPL'))
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
		return $resources[0]["dest"];
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
		return $this->ui->getButton($name,$url,$urlParams,$this);
	}
	
	function getFormHeader(&$recDesc, $nextStepUrl, $nxs=null, $nxo=null)
	{			
		return $this->ui->getFormHeader($recDesc,$nextStepUrl,$nxs,$nxo);
	}
	
	function getStyle($styleKey=null,$deft=null)
	{		
		return $this->ui->getStyle($styleKey,$deft,$this);
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
			$ext="css";
		elseif ($ext=="jsx")
			$ext="js";
		
		if ($propName=="")
			$propName="page.head.$ext.req";
			
		if ($isStatic=="true" || $isStatic===true)
			$propName.=".static";
			
		// add fully specified css path
		$p = $propName;
		nxLog("Required resource:$name.$ext, device=$isDevice skin=$isSkin static=$isStatic prop=$p => {$skin}{$name}{$suffix}.{$ext}","VIEW");
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
		foreach (explode(",",$this->getProperty($propName,"")) as $u)
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
		
		return $this->ui->getStyleSheets(array_unique($css_sheets));	
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
		
		return $this->ui->getScripts(array_unique($scripts));
	}

	function getHeads($propName="",$path=null)
	{
		if ($propName=="")
			$propName="page.modules.heads";		
					
		$s = implode($this->root->getProperty($propName,array(),false),"\n");
			
		return $s;
	}
	
	function & getForm(&$record, $recProp, $recsProp)
	{
		static $form_id=0;
		
		if ($recProp!='')
		{
			$propName = $recProp;
			$submitButton=$this->ui->getSubmitButton($recProp.'.form.submit');
		}
		else
		{
			$propName=$recsProp;
			if ($recsProp=='records')
			{
				// for compatibility with old applications...
				$submitButton=$this->getProperty('submitButton');
				if ($submitButton==null)
					$submitButton=$this->ui->getSubmitButton('submit');
			}
			else
				// new way to describe the submit button
				$submitButton=$this->ui->getSubmitButton($this->viewName.'.form.submit');
		}
		
		if (!($method=$this->getProperty($this->viewName.".form.method")))
			$method=$this->getProperty("submit.form.method","POST");
					
		$action=$this->getProperty($recProp.".form.action.url",
				$this->getProperty($this->viewName.".form.action",
					$this->getProperty('submit.url',
						$this->getProperty('next.url',"#"))));
		$this->setProperty('submit.url',$action);

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

		$s = "<script>\nfunction check_".$formName."(frm) {\nvar error = \"\"; ";

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

		$s .= "<form name=\"{$formName}\" id=\"{$formName}\" onsubmit=\"return check_{$formName}(this);\" action=\"{$action}\" method=\"{$method}\"";
		if (($method=strtoupper($method))=="POST" && $fileSize > 0)
		{
			$s .= " ENCTYPE=\"multipart/form-data\">";
			// workaround php4 bug: MAX_FILE_SIZE not handled properly. 
			// Besides IE6 doesn't support it either... => set it to 100MB
			$fileSize=100000000;
			$s .= "<input type=\"hidden\" name=\"MAX_FILE_SIZE\" value=\"".$fileSize."\" /";
		}
		
		$s .= ">";
		if($method=='GET')
		{
			if ($nxs=$this->getProperty('form.nxs'))
				$s .= '<input type="hidden" name="nxs" value="'.$nxs.'"/>';
			if ($nxo=$this->getProperty('form.nxo'))
				$s .= '<input type="hidden" name="nxo" value="'.$nxo.'"/>';
		}
		$s .= '<input type="hidden" name="_form_method_" value="'.$method.'"/>';

		$form = array('header'=>$s, 'hiddenFields'=>$hiddenFields, 
					  'isHelp'=>$isHelp, 'isRequired'=>$isRequired, 'submit'=>$submitButton,'name'=>$formName);	
			
		return $form;
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
	function initSkin($skin,$skinDeft,$root='') {
		global $NX_SKIN,$NX_SKIN_DFT,$NX_SKIN_COMP,$NX_SKIN_PATH,$NX_SKIN_URL,$NX_DEFT_SKIN_URL,
				$NX_IMAGES,$NX_SKIN_RES,$NX_DEFT_SKIN_RES,$NX_SKIN_PACKAGE;

		if (isset($NX_SKIN_PACKAGE) && $NX_SKIN_PACKAGE==$root)
			return;
		$NX_SKIN_PACKAGE=$root;

		if (isset($NX_SKIN))
			return;
		
		$NX_SKIN=$skin;
		$NX_SKIN_DFT=$skinDeft;
		
//		$NX_SKIN=$this->getProperty('page.skin',NX_SKIN_SERVER);
//		$NX_SKIN_DFT=$this->getProperty('page.skin.default','default');
		$NX_DIS_SKIN_ROOT=NX_DIS_CONF.'view/default/';

		// compiled views storage
		$NX_SKIN_COMP=NX_GEN."view/{$NX_SKIN}/";
				
		// standard paths
		$skinRoot=NX_CONF.'view/';		
		$NX_SKIN_PATH=array(
			"{$skinRoot}{$NX_SKIN}/",	// current skin root
			"{$skinRoot}{$NX_SKIN_DFT}/", // default skin root
			NX_DIS_CONF."view/{$NX_SKIN_DFT}/" // distrib skin root
		);

		// search first package paths
		if ($root)
		{
			$skinRoot=NX_CONF."{$root}view/";
			
			$NX_SKIN_PATH=array_merge(array(
				"{$skinRoot}{$NX_SKIN}/",	// current skin root
				"{$skinRoot}{$NX_SKIN_DFT}/", // default skin root
			),$NX_SKIN_PATH);
		}
					
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


}

?>
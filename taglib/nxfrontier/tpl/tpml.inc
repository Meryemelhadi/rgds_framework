<?PHP

// library for the creation of html or xml views
include_once(NX_TAG_LIB.'w3/1999/xsl/Transform.inc');

class taglib_nxfrontier_tpl_tpml extends taglib_w3_1999_xsl_Transform
{	
	// list of preprocessed files
	var $preprocess=array();
	
	function __construct(&$ctx)
	{
		parent::__construct($ctx);
	}
	
	function _views(&$node,$level)
	{
		// push skin for resource deployment
		$this->getAttribute($skin,$node,'skin','default');		
		$this->context->addResource($skin,'skin');

		$this->getAttribute($suffix,$node,'suffix','tpl');		
		$this->context->addResource($suffix,'suffix');		

		$this->getAttribute($skin_path,$node,'skin_path',$skin);		
		$this->context->addResource($skin_path,'skin_path');

		$this->getAttribute($path,$node,'path','/');
		$this->context->addResource($path,'tpl:path');

		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
		
		$this->loadConfig($node);
	}
	
	function views() 
	{
		// pop resources
		$this->context->getLastResource('skin',true);
		$this->context->getLastResource('skin_path',true);
		$this->context->getLastResource('tpl:path',true);
		$this->context->getLastResource('package',true);
		$this->context->getLastResource('suffix',true);
	}
	
	// aliase for views
	function _screens(&$node,$level) {
		return $this->_views($node,$level);
	}
	
	function screens() {
		return $this->views();		
	}
	
	function _view(&$node,&$level)
	{
		// decode package from attributes name and package.
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
		
		// get name (without package)
		if ($this->getAttribute($name,$node,'name')==null)
			return $this->fatal('no name for view');

		// inherit path and skin
		if (!$this->getAttribute($path,$node,'path',null))
		{
			// get path from tpl:views ancestor
			$path=$this->context->getLastResource('tpl:path',false);
			if ($path===null)
				$path='/';
			$node->setAttribute('path',$path);
		}

		if (!$this->getAttribute($suffix,$node,'suffix'))
		{
			$suffix=$this->context->getLastResource('suffix',false);
			if ($suffix==null)
				$suffix='tpl';				
			$node->setAttribute('suffix',$suffix);
		}

			
		if (!$this->getAttribute($skin,$node,'skin'))
		{
			$skin=$this->context->getLastResource('skin',false);
			if ($skin==null)
				$skin='default';
			$node->setAttribute('skin',$skin);
		}

/*		if (isset($parent)&&($skin=$parent->getAttribute('skin'))!==null)
			$node->setDeftAtt('skin',$skin=$this->parseVal($skin));
		else
			$node->setDeftAtt('skin',$skin='default'); */
		
		// push skin for resource deployment
		// $this->getAttribute($skin,$node,'skin');
		$this->context->addResource($skin,'skin');
					
		// app file path w/o extension
		$node->setAttribute('fpath',trim($path,'/\\').'/'.ltrim(str_replace('.','/',$name),'/\\'));		
		$level=0;
	}
	
	function view($node,$content,$level,$pad)
	{
		// get root directory for views (and pop package)
		$package=$this->context->getLastResource('package',true);
		if ($package=='root'||$package=='')
			$rootPath=NX_CONF.'view/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/view/';
		else
			$rootPath=NX_CONF."packages/{$package}/view/";
		
		$this->getAttribute($suffix,$node,'suffix','tpl');

		$fullpath=ltrim($node->getAttribute('fpath'),'/');
		// $skin=$node->getAttribute('skin');
		$skin=$this->context->getLastResource('skin',false);
		$basepath="{$rootPath}{$skin}/{$fullpath}";
		$path="{$basepath}.$suffix";

		// cleanup tpl and tpml
		// $this->context->removeFile("$basepath}.tpml");
		// $this->context->removeFile("$basepath}.tpl");

		if (($inherit=$node->getAttribute('inherit'))!==null)
		{
			$inherit=trim($this->parseVal($inherit),"' ");
			$content.="\n{$pad}{include:$inherit}";
		}
		
		// parseVal ?
		$content=$this->parseVal($content);

		$this->context->saveSrc($path,$content);
		
		// pop skin
		$this->context->getLastResource('skin',true);
		
		if ($this->getAttribute($trace,$node,'trace','no') && $this->getBool($trace))
		{
			$src=nl2br(htmlspecialchars($content));
			nxltrace("TPL VIEW: [$fullpath] at $path<br/><div class=\"listing\">".$src."</div>",'compil');
		}
		else
			nxltrace("TPL VIEW: [$fullpath] at $path",'compil');
			
		return '';
	}

	// alias for view
	function _screen(&$node,&$level) {
		return $this->_view($node,$level);
	}
	function screen($node,$content,$level,$pad) {
		return $this->view($node,$content,$level,$pad);
	}
	
	function include_($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($fpaths,$node,'file',null,true))
		{
			return $this->fatal('no file attribute');
		}

		$pre_suffix='';
		if ($this->getAttribute($suffix,$node,'suffix',null,true))
		{
			$suffixes=explode("|",$suffix);
			if (count($suffixes)==2)
			{
				$pre_suffix=".{$suffixes[0]}";
				$suffix=".{$suffixes[1]}";
			}
			else
				$suffix=".{$suffix}";
		}
		else
		{
			$suffix='';
		}

		$istrace=$this->getAttribute($trace,$node,'trace','no',true) && $trace!=''&& $this->getBool($trace);

		$fpaths=explode(';',preg_replace('/[;, ]+/',';',$fpaths));
		$res='';
		foreach ($fpaths as $fpath)
		{
			$fpath1=$fpath.$pre_suffix;
			$fpath.=$suffix;

			// if relative path: add working dirs in stack	
			if ($fpath[0]!='/')
				$libPath = array_merge(
						array_reverse($this->getResources('doc_wd')),
						$this->context->tagLibPath);
			else
				$libPath = $this->context->tagLibPath;
			
			// skin path
			if ($this->getAttribute($skinPath,$node,'skin_path') && trim($skinPath,' ,;')!='')
			{
				// very likely to be in skin
				$skins=explode(',',preg_replace('/[; ]/',',',$skinPath));
				$this->context->addResource($skinPath,'skin_path');
				$pushSkins=true;
			}
			else
			{
				// no skin path, use the current ones defined in tpl:views or tpl:view
				$skinPath=$this->getLastResource('skin_path');
				$skins=explode(',',preg_replace('/[; ]/',',',$skinPath));
				$pushSkins=false;

				/*
				$skins=array_reverse($this->getResources('skin'));
				$skins[]='default';
				$skins=array_unique($skins);
				*/
			}

			// add paths to skin directories
			foreach ($skins as $skin)
			{
				foreach ($libPath as $path)
				{
					$libPath2[]="{$path}{$skin}/";
					$libPath2[]="{$path}views/{$skin}/";
				}
			}
			
			foreach ($libPath as $path)
			{
				$libPath2[]="{$path}";
			}
			
			$libPath=array_unique($libPath2);
			
			/*
			if ($fpath[0]!='/')
			{
				// relative path: add working dirs in stack	
				$libPath[]=$this->context->getWorkingDir();
				$doc_uri=$this->context->getLastResource('doc_uri',false);
				if (isset($doc_uri))
					$libPath[]=dirname($doc_uri).'/';
			}
			*/
			
			$isfound=false;
			foreach ($libPath as $rootPath)
			{
				$path=$rootPath.ltrim($fpath,'/\\');
				$path1=$rootPath.ltrim($fpath1,'/\\');
				
				// pre-process
				if ($pre_suffix && !isset($this->preprocess[$path1]) && is_file($path1))
				{
					$this->preprocess[$path1]=$path;
					
					// set xsl variables
					$this->context->addResource(preg_replace(array('/[.]html?/','#[\/]+#'),array('','_'),$fpath1),'_view_id_');
					$this->context->addResource($fpath1,'_view_');
					$this->context->addResource($rootPath,'_path_');
					
					// get included source	
					nxltrace("<b>&lt;prepocess file=\"$path1\" to=\"$path\" skinpath=\"$skinPath\"&gt;</b>",'compil');
					
					// deactivate tpl lib and map html to html4tp
					$store_lib=&$this->context->mapURI('http://www.nxfrontier.com/tpl/tpml',
						'http://www.nxfrontier.com/xo/copy');

					// $store_lib2=&$this->context->mapURI('http://www.w3.org/TR/REC-html40','http://www.nxfrontier.com/tpl/html');
						
					$src=$this->context->processFragment($path1,null,$level,$rootPath);
					$len=strlen($src);
					nxltrace("<b>&lt;prepocessed file=\"$path1\" length=\"$len\" /&gt;</b>",'compil');
					$this->context->saveSrc($path,$src,true);								

					// reactivate libs
					$this->context->restoreLibUri('http://www.nxfrontier.com/tpl/tpml',$store_lib);
					// $this->context->restoreLibUri('http://www.w3.org/TR/REC-html40',$store_lib2);

					// restore xsl variables
					$this->context->getLastResource('_view_',true);
					$this->context->getLastResource('_path_',true);

					nxltrace("<b>&lt;/preprocess file=\"$path1\"&gt;</b>",'compil');					
				}
				
				// process
				if (is_file($path))
				{
					if($this->getAttribute($xpath,$node,'xpath','',true))
					{
						$pushXpath=true;
						$this->context->addResource($xpath,'xpath');	
					}
					else
						$pushXpath=false;

					// set xsl variables
					$this->context->addResource(preg_replace(array('/[.]html?/','#[\/]#'),array('','_'),$fpath),'_view_id_');
					$this->context->addResource($fpath,'_view_');
					$this->context->addResource($rootPath,'_path_');
			
					// get included source	
					nxltrace("<b>&lt;include file=\"$path\" xpath=\"$xpath\" skinpath=\"$skinPath\"&gt;</b>",'compil');
					$res.=$this->context->processFragment($path,null,$level,$rootPath);
					
					// trace
					if ($istrace)
					{
						$src=nl2br(htmlspecialchars($res));
						nxltrace("INCLUDE: [$path] content:<br/><div class=\"listing\">".$src."</div>",'compil');
					}
								
					nxltrace("<b>&lt;/include file=\"$path\"&gt;</b>",'compil');

					// restore xsl variables
					$this->context->getLastResource('_view_',true);
					$this->context->getLastResource('_path_',true);
					if ($pushXpath)
						$this->context->getLastResource('xpath',true);
					
					// ok worked, load other files
					$isfound=true;
					break;	
				}
			}
				
			// pop skin path
			if ($pushSkins)
				$this->context->getLastResource('skin_path',true);
			
			if ($isfound==false)
				return $this->fatal("include file doesn't exist:[$fpath], in skin path:[ $skinPath ]. Tried paths:<ul>".implode('<li>',$libPath).'</ul>');
		}
		return $res;
	}

	function content($node,$content,$level,$pad)
	{			
		$name=$this->parseVal($node->setDeftAtt('name','page.content'));
		$node->setDeftAtt('path','');
		
		if ($content=='')
		{
			$this->getAttribute($deft,$node,'default');
			if ($deft!==null)
				$content=$deft;
			else
				$content='&nbsp;';
		}
		
		return "\n{content:$name}{$content}{/content}\n";
	}
	
	// alias for content
	function zone($node,$content,$level,$pad){
		return $this->content($node,$content,$level,$pad);
	}
	
	function set($node,$content,$level,$pad)
	{			
		return $this->content($node,$content,$level,$pad);
	}
	
	/*
	function button($node,$content,$level,$pad)
	{			
		if(!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no "name" attribute');
			
		if($this->getAttribute($params,$node,'params'))
		{
			$params=':'.trim($params," \n\t#\r");
		}			
					
		return "{button:{$name}{$params}}\n";
	}
	*/
	function simple_button($node,$content,$level,$pad)
	{			
		// name for label or href
		if(!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no "name" attribute');
					
		// class
		if (!$this->getAttribute($className,$node,'class'))
			$this->getAttribute($className,$node,'type','button');
		
		// content label
		if (trim($content)=='')
			$label='<?php echo $this->getString($this->getProperty("'.$name.'.label","'.$name.'")); ?>';
		else
		{
			$label=$content;
			if(preg_match('/[$]/',$label) && !preg_match('/ echo /',$label))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($label,$node,'label',null,true,true,false);
				$label="<?php echo $label; ?>";
			}
			else
				$label=$content;
		}
		
		// target
		if ($this->getAttribute($target,$node,'target'))
			$target=" target=\"$target\" ";
		else
			$target='';

		// url
		if (!$this->getAttribute($u,$node,'href'))
			$this->getAttribute($u,$node,'url','#');

		if(preg_match('/[$]/',$u) && !preg_match('/ echo /',$u))
		{
			// if contains php, rewrite it with PHP 
			$u="<?php echo $u; ?>";
		}

		$link = '<a href="'.$u.'"'.$target.'>'.$label.'</a>';
		
		if ($className=='(none)')
			return $link;
		else
			return '<span class="'.$className.'">'.$link.'</span>';
	}
	
	function button_text($node,$content,$level,$pad)
	{			
		// get root directory for views (and pop package)
		$package=$this->context->getLastResource('package',false);
		if ($package=='root'||$package=='')
			$rootPath=NX_CONF;
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/view/';
		else
			$rootPath=NX_CONF."packages/{$package}/";

		if(!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no "name" attribute');
					
		$this->getAttribute($className,$node,'type','button');

		if($this->getAttribute($tooltip,$node,'tooltip','',true))
		{
			$tooltip=" title=\"$tooltip\" ";
		}

		if(!$this->getAttribute($label,$node,'label',null,true))
		{
			$label='<?php echo $this->getString($this->getProperty("'.$name.'.label","'.$name.'")); ?>';
		}
		else 
		{
			if(preg_match('/[$]/',$label) && !preg_match('/ echo /',$label))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($label,$node,'label',null,true,true,false);
				$label="<?php echo $label; ?>";
			}
			else
				$label='{string:"'.$label.'"}';
		}
		
		if ($this->getAttribute($url,$node,'url',null,true,false,false) && $url!='')
		{
			if(preg_match('/[#]/',$url) && preg_match('/[$]/',$url))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($url,$node,'url',null,true,true,false);
			}
			else if(!preg_match('/[$]/',$url))
				$url="'$url'";
		}
		else		
		{
			$url='$this->getProperty("'.$name.'.url")';
		}

		if($this->getAttribute($params,$node,'param',null,true,true)||$this->getAttribute($params,$node,'params',null,true,true))
		{
			
			if (preg_match('/#((record|field)[.:][^#]+)#/',$params))
			{
				
				$params= preg_replace(
					array('/#((record|field)[.:][^#]+)#/'),
					array('{$1}'),
					$params);
				$u="<?php echo \$url; ?>$params";
			}
			else
			{
				$params=$this->parseVal($params,false);
				$u="<?php echo \$url.{$params}; ?>";
			}
		}
		else
				$u="<?php echo \$url; ?>";
			
		// target
		if ($this->getAttribute($target,$node,'target'))
		{
			$target=" target=\"$target\"";
		}
		else 
		{
			$target='';
		}
	
		if($this->getAttribute($desc,$node,'template',null,true)
			|| (defined('NX_AUTO_BUTTON') && NX_AUTO_BUTTON!=false && ($desc='../images/buttons/'.NX_AUTO_BUTTON)))
		{
			$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
			$resDir=$rootPath."view/$skin/res/";
			
			// load the template
			$descbase=basename($desc);
			
			// deploy the whole desc dir including images and desc
			$this->context->deployDir($desc,'*.*',$rootPath."view/$skin/res/images/buttons/$descbase/");
			
			// get file extension from template
			$ext='png';
			if (is_file($desc))
			{
				include($desc);
				if (isset($properties['background-image']))
					$ext=preg_replace('/^.+\.([^.]+)$/','$1',$properties['background-image']);
			}
			
			// existing button
			$image='{@@NX_SKIN_URL}images/buttons/'.$descbase.'/'.$label.".$ext";
				
			$this->getAttribute($className,$node,'type','button_image');
			$content="<img border='0' skin='true' src='$image' alt='$label'/>";

			// set class
			$className=$descbase;

			$res= "<?php \$url={$url};if(\$url!=''){?>".
					'<span class="'.$className.'" id="image"'.$tooltip.'><a href="'.$u.'"'.$target.'>'.$content.'</a></span>'.
				'<?php } ?>';
		}
		else
		{
			$res= "<?php \$url={$url};if(\$url!=''){?>".
					'<span class="'.$className.'" id="text"'.$tooltip.'><a href="'.$u.'"'.$target.'>'.$label.'</a></span>'.
				'<?php } ?>';
		}
		
		return $res; // "{button:$name$params}";
	} 
	
	function button($node,$content,$level,$pad)
	{		
		// get root directory for views (and pop package)
		$package=$this->context->getLastResource('package',false);
		if ($package=='root'||$package=='')
			$rootPath=NX_CONF;
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/view/';
		else
			$rootPath=NX_CONF."packages/{$package}/";
	
		if(!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no "name" attribute');
							
		if(!$this->getAttribute($label,$node,'label',null,true))
		{
			$label='<?php echo $this->getString($this->getProperty("'.$name.'.label","'.$name.'")); ?>';
		}
		else 
		{
			if(preg_match('/[$]/',$label) && !preg_match('/ echo /',$label))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($label,$node,'label',null,true,true,false);
				$label="<?php echo $label; ?>";
			}
		}
		
		if ($this->getAttribute($url,$node,'url',null,true,false,false) && $url!='')
		{
			if(preg_match('/[$]/',$url))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($url,$node,'url',null,true,true,false);
				$url="<?php echo $url; ?>";
			}
		}
		else		
		{
			$url='<?php echo $this->getProperty("'.$name.'.url"); ?>';
		}
		
		if($this->getAttribute($params,$node,'param')||$this->getAttribute($params,$node,'params'))
		{
			$url.= preg_replace(
				array('/#((record|field)[.:][^#]+)#/'),
				array('{$1}'),
				$params);
		}			
		
		if ($this->getAttribute($target,$node,'target'))
		{
			$target=" target=\"$target\"";
		}
		else 
		{
			$target='';
		}

		// if image button, try to find the button description and apply it.
		if($this->getAttribute($image,$node,'image',null,true))
		{
			$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
			$resDir=$rootPath."view/$skin/res/";	
			if ($this->context->deployFile($image,$rootPath."view/$skin/res/$image"))
			{
				// existing button
				$image='{@@NX_SKIN_URL}images/'.preg_replace('#^.*/([^/.]+\.[^.]+)$#','$1',$image);
			}			
				
			$this->getAttribute($className,$node,'type')||$this->getAttribute($className,$node,'class','button_image');
			
			$content="<img border='0' skin='true' src='$image' alt='$label'/>";
			$res= '<span class="'.$className.'" id="image"><a href="'.$url.'"'.$target.'>'.$content.'</a></span>';
		}
		elseif($this->getAttribute($desc,$node,'template',null,true)
			|| (defined('NX_AUTO_BUTTON') && NX_AUTO_BUTTON!=false && ($desc='../images/buttons/'.NX_AUTO_BUTTON)))
		{
			$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
			$resDir=$rootPath."view/$skin/res/";
			
			// load the template
			$descbase=basename($desc);
			
			// deploy the whole desc dir including images and desc
			$this->context->deployDir($desc,'*.*',$rootPath."view/$skin/res/images/buttons/$descbase/");
			
			// get file extension from template
			$ext='png';
			if (is_file($desc))
			{
				include($desc);
				if (isset($properties['background-image']))
					$ext=preg_replace('/^.+\.([^.]+)$/','$1',$properties['background-image']);
			}
			
			// existing button
			$image='{@@NX_SKIN_URL}images/buttons/'.$descbase.'/'.$label.".$ext";

			// set class
			$this->getAttribute($className,$node,'type','button_image');
			$className=$descbase;
			
			$content="<img border='0' skin='true' src='$image' alt='$label'/>";
			$res= '<span class="'.$className.'" id="image"><a href="'.$url.'"'.$target.'>'.$content.'</a></span>';
		}
		else
		{
			$this->getAttribute($className,$node,'type')||$this->getAttribute($className,$node,'class','button_text');
			$res= '<span class="'.$className.'" id="text"><a href="'.$url.'"'.$target.'>'.$label.'</a></span>';
		}
				
		return $res; // "{button:$name$params}";
	} 
	
	function submit($node,$content,$level,$pad)
	{	
		// get root directory for views (and pop package)
		$package=$this->context->getLastResource('package',false);
		if ($package=='root'||$package=='')
			$rootPath=NX_CONF;
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/view/';
		else
			$rootPath=NX_CONF."packages/{$package}/";
					
		if(!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no "name" attribute');
		
		if(!$this->getAttribute($label,$node,'label',null,true))
		{
			$label='<?php echo $this->getProperty("'.$name.'.label"); ?>';
		}
		else 
		{
			if(preg_match('/[$]/',$label) && !preg_match('/ echo /',$label))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($label,$node,'label',null,true,true,false);
				$label="<?php echo $label; ?>";
			}
		}

		// TODO: check if this is needed? used in pecr search form?
		$action = '<?php echo " onClick=\"onSubmitForm(\'".$form["name"]."\',\'".$this->getProperty("'.$name.'.url","null")."\')\" ";?>';
				
		if ($this->getAttribute($url,$node,'url',null,true,false,false) && $url!='')
		{
			if(preg_match('/[$]/',$url))
			{
				// if contains php, rewrite it with PHP and string wrapper
				$this->getAttribute($url,$node,'url',null,true,true,false);
				$url="<?php echo $url; ?>";
			}
		}
		else		
		{
			$url='<?php echo $this->getProperty("'.$name.'.url"); ?>';
		}
		
		if($this->getAttribute($params,$node,'param')||$this->getAttribute($params,$node,'params'))
		{
			// defer record and field expressions to tpl macro engine {record.xxx}
			$url.= preg_replace(
				array('/#((record|field)[.:][^#]+)#/'),
				array('{$1}'),
				$params);
		}			

		if($this->getAttribute($image,$node,'image',null,true))
		{
			$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
			$resDir=$rootPath."view/$skin/res/";	
			if ($this->context->deployFile($image,$rootPath."view/$skin/res/$image"))
			{
				$image='{@@NX_SKIN_URL}images/'.preg_replace('#^.*/([^/.]+\.[^.]+)$#','$1',$image);
			}

			$this->getAttribute($className,$node,'type','button_image');
			$res='<input '.$action.' skin="true" class="'.$className.'" id="submit" type="image" alt="'.$label.'" src="'.$image.'"/>';
		}
		elseif($this->getAttribute($desc,$node,'template',null,true)
			|| (defined('NX_AUTO_BUTTON') && NX_AUTO_BUTTON!=false && ($desc='../images/buttons/'.NX_AUTO_BUTTON)))
		{
			$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
			$resDir=$rootPath."view/$skin/res/";
			
			// load the template
			$descbase=basename($desc);
			
			// deploy the whole desc dir including images and desc
			$this->context->deployDir($desc,'*.*',$rootPath."view/$skin/res/images/buttons/$descbase/");
			
			// get file extension from template
			$ext='png';
			if (is_file($desc))
			{
				include($desc);
				if (isset($properties['background-image']))
					$ext=preg_replace('/^.+\.([^.]+)$/','$1',$properties['background-image']);
			}
			
			// existing button
			$image='{@@NX_SKIN_URL}images/buttons/'.$descbase.'/'.$label.".$ext";
				
			$this->getAttribute($className,$node,'type','button_image');

			// set class
			$className=$descbase;

			$content="<img border='0' skin='true' src='$image' alt='$label'/>";
			$res='<input '.$action.' skin="true" class="'.$className.'" id="submit" type="image" alt="'.$label.'" src="'.$image.'"/>';
		}
		else
		{
			$this->getAttribute($className,$node,'type','button_text');
			$res='<input '.$action.' class="'.$className.'" id="submit" type="submit" value="'.$label.'"/>';
		}
				
		return $res; // "{button:$name$params}";
	} 
	
	// TPL include: included by TPL on runtime when building the view
	function insert($node,$content,$level,$pad)
	{
		if (($view=$node->getAttribute('view'))==null)
			return $this->fatal('no view for insert');
		
		return $this->parseVal("{include:{$view}:skin and device}\n");
	}
	
	// alias
	function load($node,$content,$level,$pad) {
		return $this->insert($node,$content,$level,$pad);	
	}
	
	// <property name="article.content" default="string:'empty document'" />
	function property($node,$content,$level,$pad)
	{			
		if(!$this->getAttribute($name,$node,'name',null,true,true))
			return $this->fatal('no "name" attribute');

		$this->getAttribute($deft,$node,'default','',true,true);
		
		$s="\$this->getProperty($name,$deft)";
		if ($this->getAttribute($parse,$node,'parser'))
		{
			if (strtolower($parse)!=='tpl')
				return $this->fatal('unknown tpl parser for property '.$name);
				
			$s= "preg_replace(array('/{@?(?:=|property:)([^}]+)}/ie',),
				{$pad}	array('\$this->getProperty(\"$1\")'),
				{$pad}	{$s},\$this)";
		}
		
		return "<?php echo $s; ?>";
	}
	
	function records($node,$content,$level,$pad)
	{
		$from=$node->getAttribute('from','');
		if ($from!='')
			$from=$this->parseVal(":$from");
		
		$no_record=$this->context->getResources('tpl:no_record',"\n",true);
		return "\n{$pad}{records$from}{$content}\n{$pad}{/records}\n$no_record";		
	}
	
	function no_record($node,$content,$level,$pad)
	{
		$this->getAttribute($str,$node,'value',$content,true);
		$this->context->addResource("\n{$pad}{no_record}{$str}\n{$pad}{/no_record}",'tpl:no_record');
		return '';
	}
		
	function record($node,$content,$level,$pad)
	{
		$from=$node->getAttribute('from','');
		if ($from!='')
			$from=$this->parseVal(":$from");
		
		return "\n{$pad}{record$from}{$content}\n{$pad}{/record}";
	}
	
	function form($node,$content,$level,$pad)
	{
		$s='<?php
		';
		
		$this->getAttribute($rec,$node,'from','page.record',true,true);
		
		$act=$node->getAttribute('action','');
		if ($act!='')
		{
			$act=preg_replace('/\{(=|property:)([^}]+)\}/','#property:$2#',$act);
			$act=$this->parseVal($act,true,false);
			
			$s.="\$this->setProperty('submit.url',$act);";
		}
		
/*		
		if ($this->getAttribute($act,$node,'action',null,true,true))
		{
			$act=preg_replace('/\{(?:=|property:)([^}]+)\}/','#property:$1#',$act);
			$s.="\$this->setProperty('submit.url',$act);";
		}
*/
		if ($this->getAttribute($method,$node,'method',null,true,true))
//			$s.="\$this->setProperty($rec.'.form.action',$act);";
			$s.="\$this->setProperty(\$this->viewName.'.form.method',$method);";
			
		return $s.'
		// <form>
		$fmethod = \'toForm\';
		$_nx_isViewForm=true;
		$_nx_recIndex=0;
		$_nx_record=$this->getProperty($_nx_record_prop='.$rec.',null,true);
			if ($_nx_record!=null)
			{
			$_nx_records_prop="";
			$_nx_fieldCount=$_nx_record->fieldCount();
			$_nx_record_key=$_nx_record->getUrlKey(); // record.url_key
			
			$form = $this->getForm($_nx_record, $_nx_record_prop, $_nx_records_prop);
			echo $form["header"];
		?>'.$content.'<?php
		echo $form["hiddenFields"].\'</form>\';
		} // </form>
?>';
	}
		
	function fields($node,$content,$level,$pad)
	{
		return "\n{$pad}{fields}{$content}\n{$pad}{/fields}";
	}
	
	function get_field($node,$content,$level,$pad)
	{
		$name=$node->getAttribute('name','');
		if ($name!='')
			$name=$this->parseVal(":$name");
			
		return "\n{$pad}{get_field{$name}}{$content}\n{$pad}{/get_field}";
	}

	
	function field($node,$content,$level,$pad)
	{
		$name=$node->getAttribute('name','');
		if ($name!='')
			$name=$this->parseVal(":$name");
			
		$format=$node->getAttribute('format','');
		if ($format!='' && $format!='auto')
			$format=$this->parseVal("$format");
		else
			$format='';
            
        if($format)
            $format=".$format";
						
		// display options
		if ($this->getAttribute($opts,$node,'options',null,true,false))
			$opts=":$opts";
		else
			$opts='';
		
		return "{field{$format}{$name}{$opts}}";
	}

	function string($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($str,$node,'value',null,true) &&
			!$this->getAttribute($str,$node,'p0',$content,true))
			return $this->fatal('no value attribute or content');

		if (!$this->getAttribute($pack,$node,'package',null,true) &&
			!$this->getAttribute($pack,$node,'p1',null,true))
			$pack='';

		$ret='<?php echo $this->getString(\''.$str.'\',null,\''.$pack.'\'); ?>';
		nxltrace("tpl string($str,$pack)=[$ret]",'walker',0,true);
		return $ret;
	}
	
	// <deploy dir="images/flags" match="img_*.gif" levels="1" skin="my"/>
	// <deploy dir="images/flags" regex="%img_[^.]*[.]gif%" levels="1" skin="my"/>
	// <deploy file="images/banner.jpg" />
	function deploy($node)
	{
		// get root directory for views (and pop package)
		$package=$this->context->getLastResource('package',false);
		if (!($package=='root'||$package==''))
		{
			if (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
				$rootPath=GENERIS_PACKAGES.$package.'/conf/';
			else
				$rootPath=NX_CONF."packages/{$package}/";

			$this->deployTo($node,$rootPath);
		}

		$rootPath=NX_CONF;
		return $this->deployTo($node,$rootPath);

	}

	function deployTo($node,$rootPath)
	{
		$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
		$this->getAttribute($type,$node,'type','resource');

		// set destination dirs for deployment
		switch($type)
		{
			case 'font':
				$resDir=$rootPath;
				break;
			default:
				$resDir=$rootPath."view/$skin/res/";
				break;
		}
	
		if ($this->getAttribute($f,$node,'file'))
		{
			$this->context->deployFile($f,$resDir.$f);
			nxltrace("<b>&lt;/tpl:deploy file=\"$file\" &gt;</b>",'compil');
			return '';
		}
		else if (!$this->getAttribute($dir,$node,'dir'))
			return $this->fatal('no file or dir attribute');
		
		$regex=null;
		if (!$this->getAttribute($regex,$node,'regex',null) &&
			$this->getAttribute($match,$node,'match',null))
		{
			$regex='#'.preg_replace(array('/[.]/','/[*]/',),
				array('\.','[^.]*',),$match).'#i';
		}
		
		$this->getAttribute($levels,$node,'levels','1');
			
		// deploy dir
		$this->context->deployDir($dir,$regex,$resDir,0+$levels,$this->context->getCurrentDir());
		
		nxltrace("<b>&lt;/tpl:deploy dir=\"$dir\" regex=\"$regex\" resDir=\"$resDir\" levels=\"$levels\" &gt;</b>",'compil');
		return '';
	}

	// <clear_cache dir="images/flags" match="img_*.gif" levels="1" skin="my"/>
	// <clear_cache dir="images/flags" regex="%img_[^.]*[.]gif%" levels="1" skin="my"/>
	// <clear_cache file="images/banner.jpg" />
	function clear_cache($node)
	{
		
		$this->getAttribute($skin,$node,'skin',$this->context->getLastResource('skin',false));
		$this->getAttribute($type,$node,'type','all');
		$this->getAttribute($dir,$node,'dir');

		$regex=null;
		if (!$this->getAttribute($regex,$node,'regex',null) &&
			$this->getAttribute($match,$node,'match',null))
		{
			$regex='#'.preg_replace(array('/[.]/','/[*]/',),
				array('\.','[^.]*',),$match).'#i';
		}
		
		$this->getAttribute($levels,$node,'levels','1');
		
		nxltrace("<b>&lt;/tpl:clear_cache type=\"$type\" skin=\"$skin\" dir=\"$dir\" match=\"$match\" regex=\"$regex\" levels=\"$levels\" &gt;</b>",'compil');
		
		// set destination dirs for deployment
		switch($type)
		{
			case 'doc_root':
				$resDir=NX_RES."skins/$skin/";
				if ($this->getAttribute($dir,$node,'dir'))
					$resDir.="$dir/";
				// clean dir
				$this->context->cleanDir($resDir,$regex,0+$levels);
				break;
			case 'gen':
				$resDir=NX_GEN."view/$skin/";
				if ($dir)
					$resDir.="res/$dir/";
				// clean dir
				$this->context->cleanDir($resDir,$regex,0+$levels);
				break;
			default:
				$resDir=NX_RES."skins/$skin/";
				if ($this->getAttribute($dir,$node,'dir'))
					$resDir.="$dir/";
				// clean dir
				$this->context->cleanDir($resDir,$regex,0+$levels);

				$resDir=NX_GEN."view/$skin/";
				if ($this->getAttribute($dir,$node,'dir'))
					$resDir.="res/$dir/";
				// clean dir
				$this->context->cleanDir($resDir,$regex,0+$levels);
				break;
		}
							
		nxltrace("<b>&lt;/tpl:clear_cache resDir=\"$resDir\" regex=\"$regex\" levels=\"$levels\" &gt;</b>",'compil');
		return '';
	}
	
	
	function out($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($str,$node,'value',$content,true,true))
			return $this->fatal('no value attribute or content');
		
		return '<?php echo '.$str.'; ?>';
	}

	function code($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($str,$node,'value',$content,true,false))
			return $this->fatal('no value attribute or content');
		
		return '<?php '.$str.'; ?>';
	}
	
	
	function echo_($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($str,$node,'value',$content,true,true))
			return $this->fatal('no value attribute or content');
		
		return '<?php echo '.$str.'; ?>';
	}
	
	// default handler
	function comment_($node,$content,$level,$pad)
	{	
		if (@$content[0]!='[')
			return '';
			
		if ($this->getAttribute($v,$node,'value'))
			$content=$v.trim($content,'#!|[]');

		return "

{$pad}{!--{$content} --}
";		
	}
	
	function _if_($node,$level)
	{
		$this->context->declareBlock($node,true);
	}
/*
	function if_($node,$content,$level,$pad)
	{
		$content='?>'.$content.'<?php ';
		
		// collection
		if (($condition=$node->getAttribute('condition'))===null &&
			($condition=$node->getAttribute('test'))===null)
			return $this->fatal('error in nxl:if : no "condition" or "test" attribute');
			
		$condition=$this->parseVal($condition,false,false);
		
		$condition=preg_replace(array('/(\sGT\s)|([&]gt[;])/i','/\sGTE\s/','/\sLT\s|&lt;/','/\sLTE\s/','/\sEQ\s/','/\sNEQ\s/'),
								array(' > ',' >=' ,' < ',' <= ',' === ',' != '),
								$condition);
				
		return '<?php'."\n{$pad}if({$condition})
{$pad}\{{$content}
{$pad}}".'?>';
	}
*/
	
	function if_($node,$content,$level,$pad)
	{		
		// collection
		if (($condition=$node->getAttribute('condition'))===null &&
			($condition=$node->getAttribute('test'))===null)
			return $this->fatal('error in nxl:if : no "condition" or "test" attribute');
					
		$condition=preg_replace(array('/[\\\\]#/','/#/','/(\sGT\s)|([&]gt[;])/i','/\sGTE\s/','/\sLT\s|&lt;/','/\sLTE\s/','/\sEQ\s/','/\sNEQ\s/'),
								array('#','\#',' > ',' >=' ,' < ',' <= ',' === ',' != '),
								$condition);
				
		return "\n{$pad}{if:{$condition}}
{$pad}{$content}
{$pad}{/if}";
	}

	function _else_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}
	
	function else_($node,$content,$level,$pad)
	{
		$content='?>'.$content.'<?php ';
		return '<?php'."\n{$pad}else \{{$content}
{$pad}}".'?>';
	}
	
	// <tpl:require name="imageDialog" type="js" byskin="yes" bydevice="yes" />	
	function require_($node,$content,$level,$pad)
	{
		// name
		if (!$this->getAttribute($name,$node,'name',$content,true))
			return $this->fatal('no name attribute or content');
			
		$name=preg_replace('|[.]|','/',$name);
			
		// type
		if (!$this->getAttribute($ext,$node,'type',$content,true))
			return $this->fatal('no type attribute or content');		
		$ext=strtolower($ext);
		
		
		// subdir
		switch($ext)
		{
			case 'gif':case 'jpeg':case 'png':case 'swf':
			case 'jpg':case 'tiff':case 'tiff':
				$subdir='image';
				break;
			case 'css': case 'csx':
				$subdir='css';
				$ext='csx';
				break;
			case 'js': case 'jsx':
				$ext='jsx';
				$subdir='js';
				break;
			default:
				return $this->fatal("unknown resource type:[$ext]");
		}
		
		return "{@require:$subdir/$name.$ext:skin and device}";
	}
		
	// we process tags for resource deployment but dont output them
	function _ignore($node,$level,$pad)
	{
		return;
	}	
	function ignore($node,$content,$level,$pad)
	{
		return '';
	}	
	
	function skin_path($node,$content,$level,$pad)
	{
		return "{const:NX_BASE_SKINS_URL}{property:page.skin}/";
	}

	//============= expression fucntions ===================
	// tpl::skin()
	// returns current skin
	function _func_skin()
	{
		$skin=$this->context->getLastResource('skin',false);
		if ($skin==null)
			return 'default';

		return $skin;
	}

	// tpl::field('image','html','url')
	function _func_field($node,$args,$level=0)
	{
		if(($fname=isset($args[0])?trim($args[0],'\''):null)==null)
			return null;
			
		$format=isset($args[1])?trim($args[1],'\''):'html';
		$opts=isset($args[2])?trim($args[2],'\''):'';
		if ($opts!='')
			$opts=':'.$opts;
		
		return "{field.$format:{$fname}{$opts}}";
		
	}
	
	// tpl::field_expr('field.html','tree')
	function _func_field_expr($node,$args,$level=0)
	{
		$f='$_nx_field';
		
		if(($expr=isset($args[0])?trim($args[0],'\''):null)==null)
			return null;
			
		$opts=isset($args[1])?trim($args[1],'\''):'';

		if ($expr!='')
		{
			$findex='$_nx_fieldIndex';
			$patterns = array(
				'field.is_first',
				'field.is_last',
				'field.is_even_index',
				'field.is_odd_index',
				'field.index',
				'field.error',
				'field.ok',
				'field.name',
				'field.label',
				'field.required',
				'field.format',
				'field.html',
				'field.object',
				'field.string',
				'field.db',
				'field.url_param',
				'field.alias',
				'field.form',
				'field.is_help',
				'field.help',
				'field.type',
				'field.has_attribute', // ex. field.has_attribute("is_link")
				'$_FLD_','$_OPTS'
				);
				
			$replace = array(
				'($_nx_fieldIndex===0)',
				'($_nx_fieldIndex+1==$_nx_fieldCount)',
				'($_nx_fieldIndex%2===0)',
				'($_nx_fieldIndex%2===1)',
				'$_nx_fieldIndex',
				'$_FLD_->error()',
				'$_FLD_->isOk()',
				'$_FLD_->getName()',
				'$_FLD_->getLabel()',
				'$_FLD_->isRequired()',
				'$_FLD_->toFormat($_OPTS)',
				'$_FLD_->toHTML($_OPTS)',
				'$_FLD_->toObject($_OPTS)',
				'$_FLD_->toString($_OPTS)',
				'$_FLD_->toDB()',
				'$_FLD_->toUrlParam()',
				'$_FLD_->getAlias()',
				'$_FLD_->toForm()',
				'$_FLD_->getProperty("help",null,false)!=null',
				'$_FLD_->getProperty("help",null,false)',
				'$_FLD_->desc->getType()',
				'null!=$_FLD_->getProperty',
				$f,"'$opts'"
				);
			$s=str_replace($patterns,$replace,$expr);
		}
		else
		{
			$s='$_nx_field->$fmethod('."'$opts'".')';
		}
		return $s;
	}
	
	// #tpl::if_record_even_index('even','odd')#
	function if_record_even_index($node)
	{
		if (!$this->getAttribute($ifeven,$node,'p0'))
			$this->getAttribute($ifeven,$node,'ifeven','');
			
		if (!$this->getAttribute($ifodd,$node,'p1'))
			$this->getAttribute($ifodd,$node,'ifodd','');

		return "(((\$_nx_recIndex%2===0))?($ifeven):($ifodd))";
	}

	// tpl::record_expr('record.url_key')
	function _func_record_expr($node,$args,$level=0)
	{
		if(($expr=isset($args[0])?trim($args[0],'\''):null)==null)
			return null;

		$patterns = array(
			'is_form',
			'is_help',
			'form.is_help',
			'form.is_required',
			' and ',
			' or ',
			'not ',
			'record.url_key',
			'record.index',
			'record.is_first',
			'record.is_last',
			'record.is_even_index',
			'record.is_odd_index',
			'record.nb_fields',
			'records.count',
			
			// paged list of records
			'pages.several_pages',
			'pages.nb_records',
			'pages.first_record',
			'pages.last_record',
			'pages.nb_pages',
			'page.is_first',
			'page.is_last',
			'page.is_current',
			'page.is_previous',
			'page.is_next',
			'page.index',
			'page.url',		
			);
			
		$replace = array(
			'isset($form)',
			'isset($form["isHelp"]) && $form["isHelp"]',
			'isset($form["isHelp"]) && $form["isHelp"]',
			'isset($form["isRequired"]) && $form["isRequired"]',
			'&&',
			'||',
			' !',
			'$_nx_record_key',
			'$_nx_recIndex',
			'($_nx_recIndex==0)',
			'($_nx_recIndex+1==$_nxvar_records->count())',
			'($_nx_recIndex%2===0)',
			'($_nx_recIndex%2===1)',
			'$_nx_fieldCount',
			'$_nxvar_records->count()',
			
			'($_nxvar_nbpages>1)',
			'$_nxvar_context["totalRecords"]',
			'$_nxvar_context["firstRecord"]',
			'$_nxvar_context["lastRecord"]',
			'$_nxvar_nbpages',
			'($_nxvar_page==0)',
			'($_nxvar_page==$_nxvar_nbpages-1)',
			'($_nxvar_page_index==$_nxvar_page)',
			'($_nxvar_page_index-1==$_nxvar_page)',
			'($_nxvar_page_index+1==$_nxvar_page)',
			'($_nxvar_page+1)',
			'($_nxvar_page_url)'
			);
		$s= str_replace($patterns,$replace,$expr);
		return $s;
	}
	
	function _func_current_url($node,$args,$level=0)
	{
		return "htmlentities(urlencode(\$_SERVER['REQUEST_URI']))";
	}
	
	function parseVal($v,$quotes=false,$wrapPhp=false,$dft='',$prop='$this')
	{
		return parent::parseVal($v,$quotes,$wrapPhp,$dft,$prop);
	}

	function _script_(&$node,$level)
	{
		$this->getAttribute($language,$node,'language');
		switch($language)
		{
			case 'php':
			case 'nxl':
				$this->context->declareBlock($node,true);
				break;
		}
	}

	function script_($node,$content,$level,$pad)
	{
		$this->getAttribute($language,$node,'language');		
		switch($language)
		{
			case 'php':
			case 'nxl':
				return "\n<?php\n{$content}\n?>";
			default:
				return $this->tag_default($node,$content,$level,$pad,'','script');
		}
	}


}

?>
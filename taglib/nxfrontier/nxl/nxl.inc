<?php

// core library for nxl language
/**
 * Core library for the NXL language. It defines tags fro creating applications:
 * - application structure
 * - code instructions
 * - trace functions
 *
 */
class taglib_nxfrontier_nxl_nxl extends NxTagLib
{	
	var $app=null;
	var $module=null;
	var $op=null;
	
	function header()
	{
		return "// import library NXL";
	}
	
	function _nxl($node,$level) {
		if (!$this->getAttribute($comp,$node,'component')) 
			if ($this->getAttribute($path,$node,'path'))
			{
				if (strstr($path,'nxl/'))
				{
					$comp=preg_replace('#^.*nxl[/](.*)[.]nxl#','$1',$path);
					$this->setProperty('component',$comp);
				}
			}
			
		// decode package
//		$package=$this->decodePackage($node,'root');	
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
			
        // get database if specified
        $db=$this->context->getLastResource('db');
        $this->getAttribute($db,$node,'database',$db);
        if ($db)
            $this->context->addResource($db,'db');            
            
		nxltrace("Component=[$comp], Package=[$package]",'compil');	
	
		$this->loadConfig($node);
	}
	
	function nxl($node,$content,$level,$pad) {
		// pop package
		$this->context->getLastResource('package',true);
        
        // pop database if specified
        $this->context->getLastResource('db',true);

		$output=$this->context->getResources('output',"\n");
		if ($output!='')
			return $output;
			
		return "compilation success:<br><ul><li>".$this->context->getResources('compil',"<li>").'</ul>';
	}
	
	//============ function management ============
	function _func_decl($node,$content,$level,$pad)
	{
		$this->context->declareBlock($node,true);
	}
	
	function func_decl($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no name attribute');
			
		$args=$this->context->getResources("func_{$name}_arg",',',true);
				
		return "\n  function $name($args) {\n$content\n  }";
	}
	
	function func_arg($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no name attribute');
			
		$this->context->addResource($name,"func_{$name}_arg");

		return '';
	}
	
	function func_call($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name'))
			return $this->fatal('no name attribute');
			
		$args=$this->context->getResources("call_{$name}_arg",',',true);
		
		return "\n{$pad}\$this->{$name}($args)";
	}
	
	function func_with_arg($node,$content,$level,$pad)
	{
		$this->getAttribute($val,$node,'value',$content);
		$this->context->addResource($val,"call_{$name}_arg");
		return '';
	}
	
	//=========== output =============
	function echo_($node,$content,$level,$pad)
	{
		$this->getAttribute($v,$node,'value',$content,true,false);
		return "echo $v;";
	}
	
	function output($node,$content,$level,$pad)
	{
		switch ($node->getAttribute('method','xml2text'))
		{
			case 'xml2text':
				$indent="false";
				$src=nl2br(htmlspecialchars($content));
				break;
			case 'text':
				$indent="false";
				$src=$content;
				break;				
			case 'php':
				$indent="true";
				$src='<?php'.$content.' ?>';
				break;				
			case 'html':
				$indent="true";
				$src="echo <<<EOH\n".$content."\nEOH;\n";
				return $src;
				break;				
			default:
				$this->getAttribute($ident,$node,'indent',"true");				
				$src=$content;
				break;
		}

		$this->getAttribute($indent,$node,'indent',$indent);		
		if (!$this->getBool($ident))
		{
			$src=preg_replace('/^\s+/m','',$src);
		}
		
		if ($this->getAttribute($file,$node,'direct'))
		{
			return "echo '$src';";
		}
		elseif ($this->getAttribute($file,$node,'file',null,true,true))
		{
			nxltrace("store source to [$file]");
			eval("\$file=$file;");
			nxltrace("store source to [$file]");
			$this->context->saveSrc($file,$src);	
		}
		else		
			$this->context->addResource($src,'output');
			
		if($this->getAttribute($trace,$node,'trace') && $trace=="yes")
			nxltrace('<ul><li>tag result: '.nl2br(htmlspecialchars($src,ENT_NOQUOTES)).'</ul><li>','frag_source');
			
		return '';
	}
	
	// check permissions
	function check_permissions($node,$content,$level,$pad)
	{
		$this->getAttribute($creds,$node,'credentials','',true,true);
		$this->getAttribute($perms,$node,'permissions','default',true,true);
		$this->getAttribute($msg,$node,'message','permission not granted',true,true);
		if ($content=='')
			$content="echo $msg; die();";
		
		return "\n{$pad}if (\$this->checkPerm($perms,$creds)==false) {
{$pad}	$content
{$pad}}
		
		;";
	}	

	// activate/deactivate trace modes
	function set_trace($node,$content,$level,$pad)
	{	
		// display all messages
		if ($this->getAttribute($trace,$node,'debug'))
			if($trace=='yes'||$trace=='true')
				$GLOBALS['nxl_trace_debug']=true;
			else
				$GLOBALS['nxl_trace_debug']=false;
				
		// display DOM node walker
		if ($this->getAttribute($trace,$node,'walker'))
			if($trace=='yes'||$trace=='true')
				$GLOBALS['nxl_trace_walker']=true;
			else
				$GLOBALS['nxl_trace_walker']=false;
	}
	
	function exec($node,$content,$level,$pad)
	{	
		$this->getAttribute($trace,$node,'trace');
	
		$this->getAttribute($mode,$node,'mode','runtime');
		
		($this->getAttribute($buffer,$node,'buffer','false')!=='false')||$this->getAttribute($buffer,$node,'cache','false');
		$iscache=$this->getBool($buffer);		
		($this->getAttribute($prop,$node,'prop',null)!==null)||$this->getAttribute($prop,$node,'property',null);
		
		// instruction or expression ?
		if ($this->context->isInBlock($node) || !$iscache)
			$sep=';';
		else
			$sep='';
			
		// properties for the execution
		if (trim($content)=='' && !$prop)
        	$props='$this->props';
		else
			$props="array($content)";
				
		if ($this->getAttribute($file,$node,'file',null,true,true))
		{
			if($trace=='yes')
				nxltrace("exec: $file",'compil');

			if ($mode=='compil')		
				include($file);
			else
				return "\n{$pad}include($file);";
		}
		
		if ($this->getAttribute($logic,$node,'logic',null,true,true))
		{
			if($trace=='yes')
				nxltrace("exec application: $logic",'compil');

			// $logic=str_replace('/','.',$logic);
			if ($iscache || $prop)
				$f='runCachedLogic';
			else {
				$f='runLogic';
				$sep=';';
			}

			if ($mode=='compil')		
				$this->$f($logic,$props,$prop);
			else
				return "\n{$pad}\$this->$f($logic,$props,'$prop');";
		}
		
		return $this->fatal('no file or logic attribute');
	}
	
	function _logic($node) {
		$this->loadConfig($node);
	}
	
	function usePackage($node,$content,$level,$pad)
	{
		$file=null;
		if ($this->getAttribute($package,$node,'package',null))
		{
			$file=str_replace('.','/',$package).'.inc';
		}
		
		if (!$this->getAttribute($file,$node,'file',$file))
			return $this->fatal("no package or file attribute");
		
		nxltrace("ADD LIB INCLUDE: $file",'code');
		$this->context->addInclude($file);
		return $content;
	}

	function use_class($node,$content,$level,$pad)
	{
		$file=null;
		if ($this->getAttribute($package,$node,'class',null))
		{
			$file=str_replace('.','/',$package).'.inc';
		}
		
		if (!$this->getAttribute($file,$node,'file',$file))
			return $this->fatal("no package or file attribute");

		$this->getAttribute($base,$node,'base','NX_LIB');
		if ($base=='SITE_LIB')
			$base='NX_SITE."/lib/"';
		else
			$base='NX_LIB';
		
		nxltrace("ADD LIB INCLUDE: $file",'code');
		$this->context->addInclude($file,$base);
		return $content;
	}

	function define_class($node,$content) {
		nxltrace("DEFINE CLASS: $content",'code');
		$this->context->addHeaderCode($content);
	}
	
	function call_method($node,$content,$level,$pad)
	{
		// instruction or expression ?
		if ($this->context->isInBlock($node))
			$sep=';';
		else
			$sep='';

		if (!$this->getAttribute($func,$node,'func'))
			return $this->fatal('missing func');
			
		if ($this->getAttribute($object,$node,'object')!=null)
		{
			return "\n$pad{$object}->{$func}$sep";
		}
		else
			return "\n$pad{$func}$sep";
	}
		
	function _application(&$node,&$level)
	{
		// decode package
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		
		
		$this->loadConfig($node);
		
		// parse and set default attributes
		$this->getAttribute($name,$node,'name','default');
		$node->setAttribute('name',$name);

		$this->getAttribute($path,$node,'path','');
		$node->setAttribute('path',$path);

		// app file path w/o extension
		$fullpath=str_replace('.','/',trim($path,'/').'/'.$name);
		$node->setDeftAtt('fpath',trim($fullpath,'/'));
					
		// reorder operations: first is the default one => put it at the end and add default attribute
		$operations=array();
		foreach($node->childNodes as $i=>$child)
		{
			if ($child->localName=='operation')
				$operations[]=$i;
		}
		
		if (($len=count($operations))>0)
		{
			$last=&$node->childNodes[$operations[$len-1]];
			$last->setAttribute('end','true');		
			
			$default=&$node->childNodes[$operations[0]];
			$default->setAttribute('default','true');

			// at least 2 ops => reorder
			if ($last->id != $default->id)
			{
				$node->removeChild($default);
				$node->appendChild($default);
			}
		}
		
		$this->app=&$node;
				
		return true;		
	}

	function application($node,$content,$level,$pad)
	{
		// get root directory for application (and pop package)
		$package=$this->context->getLastResource('package',true);
		if ($package=='root')
			$rootPath=NX_CONF.'logic/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/logic/';
		else
			$rootPath=NX_CONF."packages/{$package}/logic/";
		
		$fullpath=trim($node->getAttribute('fpath'),'/.');
		$class_name=str_replace('/','_',$fullpath);
		$dot_path=str_replace('/','.',$fullpath);
				
		$src='<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.\'module/NxApplication.inc\');
';

		$src.=
"class $class_name extends NxApplication
{
	function $class_name".'($props,&$parentDesc,$aspect)
	{
		$this->NxApplication('."'$dot_path'".',$props,$parentDesc,$aspect);
	}

	function run()
	{
		switch ($this->operation)
		{'.$content.'
		}		
		$this->saveState();
		return true;
	}
}
?'.'>';
	
		nxltrace("application: [$dot_path] at $rootPath$fullpath",'compil');	
		
		$this->context->saveSrc($rootPath.$fullpath.'.php',$src);		
		// $this->context->saveSrc(NX_GEN.'logic/'.$fullpath.'.php',$src);		
		return '';		
	}
	
	function _operation(&$node,&$level)
	{
		// reset the automatic node level used for indentation
		$level=0;
		
		$node->setDeftAtt('name','default');
		$node->setDeftAtt('path',$this->app->getAttribute('fpath'));

		// op file path w/o extension
		$fullpath=str_replace('.','/',trim($node->getAttribute('path'),'/').'/'.$node->getAttribute('name'));
		$node->setDeftAtt('fpath',$fullpath);
		
		// reorder steps: first is the default one => put it at the end and add default attribute
		$steps=array();
		foreach($node->childNodes as $i=>$child)
		{
			if ($child->localName=='step')
				$steps[]=$i;
		}
		
		if (($len=count($steps))>0)
		{
			$last=&$node->childNodes[$steps[$len-1]];
			$last->setAttribute('end','true');		
			
			$default=&$node->childNodes[$steps[0]];
			//$default->setAttribute('default','true');

			// at least 2 steps => reorder
			if ($last->id != $default->id)
			{
				// $node->removeChild($default);
				// $node->appendChild($default);
			}
		}

		// step index for managing default step
		$this->indexStep=0;
		$this->addResource($node,'operation');
		$this->op=&$node;
		return true;
	}
	
	function operation($node,$content,$level,$pad)
	{		
		$this->getLastResource('operation',true);
		
		if (!$this->getAttribute($name,$node,'name',null,true))
			return $this->fatal('missing "name" attribute');

		$src = "    // operation: $name
		case '$name':";
		
		if ($this->getBool($node->getAttribute('default','false')))
		{
			$src.="
	default:
		\$this->operation='$name';
";
		}
			
		$src .="
		switch(\$this->step)
		{
		$content
		}
        break;
";
		return $src;		
	}
	
	function _module(&$node,&$level)
	{
		$parent=&$this->context->getParentElement($node);
		if ($parent->localName =='application')
		{
			$node->setDeftAtt('path',$this->app->getAttribute('fpath'));
		}

		// decode package from attributes name and package.
		$package=$this->decodePackage($node,'inherit');	
		$this->context->addResource($package,'package');		

		$this->loadConfig($node);
	
		
		$this->getAttribute($path,$node,'path','/');
		$this->getAttribute($name,$node,'name','process');

		// op file path w/o extension
		$fullpath=str_replace('.','/',trim($path,'/').'/'.$name);
		$node->setDeftAtt('fpath',$fullpath);

		$this->indexStep=0;
		
		$this->addResource($node,'module');
		$this->module=&$node;
		return true;		
	}
	
	function module($node,$content,$level,$pad)
	{
		// get root directory for views (and pop package)
		$this->context->getLastResource('package',true);	
		
		$this->removeResource('module');
		nxltrace(" MODULE: $content",'listing');	
		return $content;
	}
		
	function _step(&$node,&$level)
	{

		$this->indexStep++;		
		$node->setDeftAtt('name','default');
		$node->setDeftAtt('path',$this->op->getAttribute('fpath'));

		// op file path w/o extension
		$fullpath=str_replace('.','/',trim($node->getAttribute('path'),'/').'/'.$node->getAttribute('name'));
		$node->setDeftAtt('fpath',$fullpath);
		
		$this->module=&$node;
		$this->addResource($node,'module');
		return true;		
	}
		
	// application step: display servlet code (operation switch)
	// NB. actual step code written in <code>
	function step($node,$content,$level,$pad)
	{			
		$this->removeResource('module');
		
		// generate application logic for the step
		if (!$this->getAttribute($name,$node,'name','default',true))
			return $this->fatal('missing "name" attribute');
			
		if ($this->indexStep==1)
		{
			$node->setAttribute('default','true');			
		}

		$this->getAttribute($fullpath,$node,'fpath');
		$dot_path=str_replace('/','.',$fullpath);
		$oname=$this->op->getAttribute('name');

		
		$src  = "
         // step $name    
		 case '$name':";
		
		if ($this->getBool($node->getAttribute('default','false')))
		{
			$src.='
		 default:
			$this->operation=\''.$oname.'\';
			$this->step=\''.$name.'\';
			$this->startOp();
';
		}
		
		// define step properties
		$props=array();
		$trans = $this->context->getResources('trans',null,true);
		if (isset($trans['back']))
		{
			$props['back.url']='$backUrl='.$trans['back'];
			$props['cancel.url']='$backUrl';
		}
		else
			$props['cancel.url']='$this->getProperty(\'back.url\')';
		
		if (isset($trans['next']))
		{
			$props['next.url']='$nextUrl='.$trans['next'];
			$props['nextStep']='$nextUrl';
		}
				
		if (count($props))
		{
			$propSrc=',
				array(';
			foreach ($props as $n=>$prop)
			{
				$propSrc.="
					'$n'=>$prop,";
			}
			$propSrc.=')';
		}
		else
			$propSrc='';
		
			
		// add step logic
		$src.="
			if (\$this->runLogic('$dot_path'".$propSrc."))
			{";
		
		if ($this->getBool($node->getAttribute('end','false')))
		{
			if (isset($trans['redirect']))
			{
				$url=$trans['redirect'];
				// $src.='
				// $this->redirectToOperation('.$url.');';
				$src.="
				header('Location:'.{$url});";
				
			}
						
			$src.='
				$this->finish();
';
		}
		else
			$src.="\n";
				
		$src .="			}
            break;\n";
		
		return $src;
	}
	
	function logic() {
		return '';
	}
	
	function transitions($node,$content,$level,$pad)
	{
		return '';
	}
	
	function back($node,$content,$level,$pad)
	{
		$url=$this->_getUrl_($node,$content,'back.url');
		$this->context->addResource($url,'trans','back');
		return '';
	}

	function next($node,$content,$level,$pad)
	{
		$url=$this->_getUrl_($node,$content,'next.url');
		$this->context->addResource($url,'trans','next');
		return '';
	}

	function redirect($node,$content,$level,$pad)
	{
		$parent=$this->context->getParentElement($node);

		// automatic redirection in transitions
		if ($parent->localName=='transitions')
		{
			$url=$this->_getUrl_($node,$content,'next.url',true);
			// if ($url[1]=='?') $url='preg_replace(\'#^/([^?]+)?.*$#\',\'/$1\',$_SERVER[\'REQUEST_URI\']).'.$url;
			//$url='preg_replace(array(\'|^(?!(\w+)://)|\',\'|^[\\/]|\'),array(dirname($_SERVER[\'REQUEST_URI\']).\'/\',"http://{$GLOBALS[\'NX_DOMAIN\']}/"),'.$url.')';
			
			$this->context->addResource($url,'trans','redirect');
			return '';
		}
	
		// direct redirection in code
		$url=$this->_getUrl_($node,$content,'next.url',true,false,true);
		// if ($url[1]=='?') $url='preg_replace(\'#^/([^?]+)?.*$#\',\'/$1\',$_SERVER[\'REQUEST_URI\']).'.$url;
		// $url='preg_replace(array(\'|^(?!(\w+)://)|\',\'|^[\\/]|\'),array(dirname($_SERVER[\'REQUEST_URI\']).\'/\',"http://{$GLOBALS[\'NX_DOMAIN\']}/"),'.$url.')';
	
		// execute
//		return "\n{$pad}header('Location:'.\$this->getProperty('site.url.noprefix','').{$url});";
		return "\n{$pad}header('Location:'.(\$u={$url}));\n{$pad}nxLog('redirect to '.\$u,'redirect');\n{$pad}return true;\n";
	}
	
	function _completeURL($node,$url)
	{
			
		if (preg_match('|^/|',$url))
		$this->getAttribute($mode,$node,'mode','absolute');
	}
	
	// url param for button
	function url_param($node,$content,$level,$pad)
	{
		// name
		if (($name=$node->getAttribute('name'))===null)
		{
			return ".$content";
		}
		$name=$this->parseVal($name,true);
		
		// value
		$this->getAttribute($v,$node,'value',$content,true,true);
		
		return ".'&'.$name.'='.$v";
	}
	
	function _code(&$node,&$level)
	{
		// reset the automatic node level used for indentation
		$level=1;

		$node->setDeftAtt('name','default');
		$fullpath=$this->module->getAttribute('fpath');
		$node->setDeftAtt('path',$fullpath);
		
		if (!$this->getAttribute($path,$node,'path'))
			return $this->fatal('no path attribute');
		$this->getAttribute($name,$node,'name','process');

		// op file path w/o extension
		$fullpath=str_replace('.','/',trim($path,'/').'/'.$name);
		$node->setDeftAtt('fpath',$fullpath);
		$this->context->declareBlock($node,true);
		return true;
	}
	
	function methods($node,&$content,$level,$pad)
	{
		$this->methods=$content;
	}
	
	function code($node,&$content,$level,$pad)
	{		
		// get root directory for step
		$package=$this->context->getLastResource('package',false);
		if ($package=='root')
			$rootPath=NX_CONF.'logic/';
		elseif (defined('GENERIS_PACKAGES') && GENERIS_PACKAGES)
			$rootPath=GENERIS_PACKAGES.$package.'/conf/logic/';
		else
			$rootPath=NX_CONF."packages/{$package}/logic/";

		$fullpath=trim($this->module->getAttribute('fpath'),'/');
		$class_name=str_replace('/','_',$fullpath);	
		$dot_path=str_replace('/','.',$fullpath);
		
		$src='<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

require_once(NX_LIB.\'module/NxPage.inc\');
';

		// add library headers
		$imports=array_unique($this->context->getResources('import',null,true));
		foreach ($imports as $import)
		{
			$src .= $import."\n";
		}
		
		$src.=
"
class $class_name extends NxPage
{
	function $class_name".'($props,&$parentDesc,$aspect)
	{
		$this->NxPage($props,$parentDesc,$aspect);
	}

	'.$this->methods.'

	function run()
	{
		$self=$this;
		';

	$src .= $content;
	
	$src .='
		return true;
	}
}
?'.'>';
	
	$this->methods='';
	$path=$rootPath.$fullpath.'.php';
	$this->context->saveSrc($path,$src);
	//$this->context->saveSrc(NX_GEN.'logic/'.$fullpath.'.php',$src);
	nxltrace("module: [$dot_path] at $path",'compil');
	nxltrace("<pre>$src</pre>",'listing');
	return '';
	}

/*
		<display view="page.error.view" method="html">
			<property name="msg" value="error:msg"/>
		</display>
*/	
	function display($node,$content,$level,$pad)
	{			
		/* view */
		$view=$node->setDeftAtt('view','page');
		
		// if remapping of view (connection through page properties), 
		// complete name w/ .view if not present
		if ($this->getBool($node->getAttribute('remap','no')))
		{
			if (!preg_match('/[.]view$/',$view))
				$view .= '.view';		
		}

		// get mime and encoding
		$mime=null;
		if ($this->getAttribute($type,$node,'type',null)!=null)
		{
			$type=strtoupper($type);
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
				case 'json':
					$mime='application/x-json';
					break;
				case 'word':
					$mime='application/msword';
					break;
				case 'html':
				default:
					$mime='text/html';
					break;
			}
			$type_prop="{$pad}	'view.type' => '$type',\n";
		}
		else
			$type_prop='';
				
		// get mime and encoding
		$this->getAttribute($mime,$node,'mime',$mime);
		if ($mime!=null)
			$src="global \$NX_MIME;\n\$NX_MIME='$mime';\n";
		else
			$src='';
				
		$view=$this->parseVal("'{$view}'");
		
		// if property is defined then view output is redirected to this property
		if (!$this->getAttribute($prop,$node,'property',null,true,true))
			$prop='null';
		
		return "{$src}
{$pad}\$this->runView({$view},array(
{$pad}	'fmethod' => 'toHTML',
{$type_prop}{$content}
{$pad}	),$prop);";

	}
	
	function display_form($node,$content,$level,$pad)
	{
		/* view */
		$view=$node->setDeftAtt('view','page');
		/*
		if (!preg_match('/[.]view$/',$view))
			$view .= '.view';
		*/
		$method_prop=$this->parseVal("{$view}.form.method",true);
		$view=$this->parseVal("'{$view}'");
		
		// if property is defined then view output is redirected to this property
		if (!$this->getAttribute($prop,$node,'property',null,true,true))
			$prop='null';
			
		if ($this->getAttribute($method,$node,'method',null,true,true))
			$content= "\n{$pad}{$method_prop}=>$method,'submit.form.method'=>$method,\n".$content;
		
		return "
{$pad}\$this->runView({$view},array(
{$pad}	'fmethod' => 'toForm',
{$content}
{$pad}	),$prop);";
	}
	
	function form($node,$content,$level,$pad)
	{
		return $this->display_form($node,$content,$level,$pad);
	}
	
	function property($node,$content,$level,$pad)
	{
		if (!$this->getAttribute($name,$node,'name',null,true,true))
			return $this->fatal('no name');

		if (!$this->getAttribute($dft,$node,'default',null,true,true))
			$dft=$content;	

		if (!$this->getAttribute($value,$node,'value',null,true,true))
			$value=$content;	
			// return $this->fatal('no value');
				
		if ($dft!='')
			return "\n{$pad}$name=>(\$_v=($value))?(\$_v):($dft),";
		else
			return "\n{$pad}$name=>$value,";
	}

	function _script_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}

	function script_($node,$content,$level,$pad)
	{
		$language=$node->setDeftAtt('language','php');
		
		switch($language)
		{
			case 'php':
				return "\n{$content}\n";
			default:
				return $this->fatal("code section error : language not supported [{$language}]");
		}
	}
	
	function _iferror(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}
	
	function iferror($node,$content,$level,$pad)
	{
		/* terminate */
		if ($node->getAttribute('terminate')=='yes')
			$terminate="{$pad}return false;";
		else
			$terminate='';
		
		return "
{$pad}if (\$this->error->isError()) {{$content}
	{$terminate}\n{$pad}}";		
	}
	
	/* simple instruction block */
	function _block(&$node,&$level)
	{
		$this->context->declareBlock($node,true);
	}
	
	function block($node,$content,$level,$pad)
	{		
		return $content;
	}

	function clear_error($node,$content,$level,$pad)
	{
		return "\n{$pad}\$this->error->clear();";
	}

	function _ifok(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}
	
	function ifok($node,$content,$level,$pad)
	{		
		return "
{$pad}if (!\$this->error->isError()) {{$content}
\n{$pad}}";		
	}
	
	function XXXunset_($node,$content,$level,$pad)
	{	
		/* name */
		if (($name=$node->getAttribute('name'))===null)
			return $this->fatal('error in nxl:get : no "name" attribute');
			
		$an=explode(":",$name);
		if (isset($an[1]))
		{
			$name=$an[1];
			$sel=$an[0];
		}
		else
		{
			$sel='property';
		}
							
		switch(strtolower($sel))
		{
			case 'request':
			case 'post':
			case 'cookie':
			case 'session':
			case 'get':
				$var='_'.strtoupper($sel)."[\"$name\"]";
				return "\n{$pad}if(isset(\${$var}))unset(\${$var});";				
			case 'prop':
			case 'property':
				return "\n{$pad}\$this->setProperty(\"{$name}\",null);";
			case 'var':
			case 'param':
			case 'parameter':
				return "\n{$pad}if(isset(\${$name}))unset(\${$name});";		
			default:
				return "\n{$pad}/*unset : unknown variable $sel:$name*/";
		}
		
		return "\n{$pad}$s;";
	}
	
	// parameter or local variable
	function param($node,&$content,$level,$pad)
	{				
		/* name */
		if (($name=$node->getAttribute('name'))===null)
			$this->fatal('error in session:get : no "name" attribute');
			
		$node->setDeftAtt('value','#get:'.$node->getAttribute('name').'#');
		
		/* value */
		$value = $this->parseVal($node->getAttribute('value'));
		$s="\n{$pad}\${$name}={$value};";
		
		/* storage */
		if (($store=$node->getAttribute('store'))!==null)
		{
			$var='_'.strtoupper($store);
			$s.="
{$pad}\${$var}['$name']=\${$name};";
		}

		return $s;		
	}
	
	// parameter or local variable
	function params($node,&$content,$level,$pad)
	{				
		/* name */
		if (!$this->getAttribute($names,$node,'names',null,true,true))
			$this->fatal('error: no "names" attribute');
			
		/* value */
		if (!$this->getAttribute($value,$node,'value',null,true,true))
			$this->fatal('error: no "value" attribute');
		
		$vars=trim(preg_replace('/([a-zA-Z_][a-zA-Z_0-9]*)([,])?/','\$$1$2',$names),"'");
		$s="\n{$pad}list($vars)=explode(',',$value);";

		return $s;		
	}	
	
	function parse($node,$content,$level,$pad)
	{			
		/* value */
		$this->getAttribute($value,$node,'value',$content);
		
		return "\n{$pad}$value";
	}
		
	// load properties from resource file
	// <load path="article/text" add="yes|no"/>
	function load($node,$content,$level,$pad)
	{
		// instruction or expression ?
		if ($this->context->isInBlock($node))
			$sep=';';
		else
			$sep='';
				
		// path
		if ($path=$node->getAttribute('path'))
		{
			$path=$this->parseVal("'{$path}'");
						
			// path
			if ($this->getBool($node->getAttribute('add','no')))
				$add='true';
			else
				$add='false';
			
			return "\n{$pad}\$this->loadResource($path,\$this->error,$add){$sep}";
		}
		elseif ($this->getAttribute($v,$node,'value',$content,true,false))
		{
			return "\n{$pad}(\$tmp=$v)!=null?\$this->addProperties(\$tmp):null{$sep}";
		}
		else
		{
			return $this->fatal('no "path" or "value" attribute');
		}
	}
	
	function expression($node,$content,$level,$pad)
	{
		// instruction or expression ?
		if ($this->context->isInBlock($node))
			// top level
			$sep=';';
		else
			$sep='';
			
		return "\n{$pad}".$this->parseVal($content).$sep;
	}

	// <foreach sequence="var:myprops" index="i" item="v">...</foreach>
	function _foreach_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}
	
	function foreach_($node,$content,$level,$pad)
	{
		// collection
		if (($collection=$node->getAttribute('sequence'))===null)
			return $this->fatal('error in nxl:foreach : no "sequence" attribute');
		$collection=$this->parseVal($collection);
		
		// index variable
		$index=$node->getAttribute('index','index');
		
		// item variable
		$item=$node->getAttribute('item','item');
		
		// check if sequence of type CSV
		if ($sep=$node->getAttribute('separator'))
		{	
			$prop=$this->createVarID('prop');
			
		return "\n{$pad}if(\${$prop}={$collection})
{$pad}foreach(explode('{$sep}',trim(\${$prop},'{$sep}')) as \$$index=>\$$item)
{$pad}{{$content}
{$pad}}";
		}
		else
		{
			return "\n{$pad}foreach({$collection} as \$$index=>\$$item)
{$pad}{{$content}
{$pad}}";
		}
	}
	
	// <if condition="#$mytest#">...</if>
	// <if test="#$mytest#">...</if>
	function _if_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}

	function if_($node,$content,$level,$pad)
	{
		// collection
		if (($condition=$node->getAttribute('condition'))===null &&
			($condition=$node->getAttribute('test'))===null)
			return $this->fatal('error in nxl:if : no "condition" or "test" attribute');
		$condition=$this->parseVal($condition);
		
		$condition=preg_replace(array('/(\sGT\s)|([&]gt[;])/i','/\sGTE\s/','/\sLT\s|&lt;/','/\sLTE\s/','/\sEQ\s/','/\sNEQ\s/'),
								array(' > ',' >=' ,' < ',' <= ',' === ',' != '),
								$condition);
				
		return "\n{$pad}if({$condition})
{$pad}{{$content}
{$pad}}";
	}

	// <else>...</else>
	function _else_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}

	function else_($node,$content,$level,$pad)
	{
		// todo: check if previous sibling is conditional...
		
		return "\n{$pad}else {{$content}
{$pad}}";
	}
	
	/*
		<xsl:choose>
		<xsl:when test="">
		</xsl:when>
		<xsl:otherwise>
		</xsl:otherwise>
	</xsl:choose>
	*/
	function _choose($node)
	{
		$this->context->declareBlock($node,true);
	}
	function choose($node,$content,$level,$pad)
	{
		return $content;
	}
	
	function _when($node)
	{
		$this->context->declareBlock($node,true);
	}
	function when($node,$content,$level,$pad)
	{
		// check if first "when" node
		$parent=$this->getParentElement($node);
		If ($parent->localName!='choose')
			return $this->fatal('when element needs to be in choose parent');
		
		$family=$parent->getElementsByTagName('when');
		$first=$family->item(0);
		if ($first->id == $node->id)
			$inst='if';
		else
			$inst='elseif';
		
		// collection
		if (($condition=$node->getAttribute('test'))===null)
			return $this->fatal('no "test" attribute');
		$condition=$this->parseVal($condition);
		
		$condition=preg_replace(array('/(\sGT\s)|([&]gt[;])/i','/\sGTE\s/','/\sLT\s|&lt;/','/\sLTE\s/','/\sEQ\s/','/\sNEQ\s/'),
								array(' > ',' >=' ,' < ',' <= ',' === ',' != '),
								$condition);
				
		return "\n{$pad}{$inst}({$condition})
{$pad}{{$content}
{$pad}}";	
	}

	function _otherwise($node)
	{
		$this->context->declareBlock($node,true);
	}
	function otherwise($node,$content,$level,$pad)
	{
		// check if first "when" node
		$parent=$this->getParentElement($node);
		If ($parent->localName!='choose')
			return $this->fatal('when element needs to be in choose parent');
		
		$family=$parent->getElementsByTagName('when');
		$first=$family->item(0);
		if ($first!=null)
			return "\n{$pad}else {{$content}
{$pad}}";
		else
			return "\n{$pad}$content";
	}

	// <while test="#$mytest#">...</while>
	function _while_(&$node,$level)
	{
		$this->context->declareBlock($node,true);
	}

	function while_($node,$content,$level,$pad)
	{
		// collection
		if (($condition=$node->getAttribute('condition'))===null &&
			($condition=$node->getAttribute('test'))===null)
			return $this->fatal('error in nxl:while : no "condition" or "test" attribute');
		$condition=$this->parseVal($condition);
		
		$condition=preg_replace(array('/(\sGT\s)|([&]gt[;])/i','/\sGTE\s/','/\sLT\s|&lt;/','/\sLTE\s/','/\sEQ\s/','/\sNEQ\s/'),
								array(' > ',' >=' ,' < ',' <= ',' === ',' != '),
								$condition);
				
		return "\n{$pad}while({$condition})
{$pad}{{$content}
{$pad}}";
	}

	function break_($node,$content,$level,$pad)
	{
		return "\n{$pad}break;";
	}
	
	function continue_($node,$content,$level,$pad)
	{
		return "\n{$pad}continue;";
	}
	
	function exit_($node,$content,$level,$pad)
	{
		$ret=strtolower($node->getAttribute('result','ok'));
		if ($ret=='ok')
			return "\n{$pad}return true;";
		else
			return "\n{$pad}return false;";		
	}
	
	function comment_($node,$content,$level,$pad)
	{	
		if (@$content[0]!='[')
			return '';
		$content = trim($content,'[]');
		
		if ($this->getAttribute($v,$node,'value'))
			$content=$v.trim($content,'#!|[]');
			
		return "\n\n{$pad}/*{$content} */";		
	}
	function _comment($node,$content,$level,$pad)
	{	
		if (@$content[0]!='[')
			return '';
		$content = trim($content,'[]');
		
		if ($this->getAttribute($v,$node,'value'))
			$content=$v.trim($content,'#!|[]');
			
		return "\n\n{$pad}/*{$content} */";		
	}
	
	function _text($node)
	{	
		$this->keepAllText($node,true);
	}
	
	function text($node,$content,$level,$pad)
	{
		return $content;
	}
	
	function testfunc($node,$content,$level,$pad)
	{		
		return 'ok';
	}

	function string($node,$content,$level,$pad)
	{
		if (!($this->getAttribute($v,$node,'value',null,true,false) ||
			$this->getAttribute($v,$node,'p0',$content,true)))
			return $this->fatal('no value attribute or content');
			
		if (preg_match('/\$\w/',$v))
			$str=$v;
		else
			$str="'$v'";	

		if (!$this->getAttribute($pack,$node,'package',null,true))
			$this->getAttribute($pack,$node,'p1','',true);
		
		return '$this->getString('.$str.',null,\''.$pack.'\')';
	}
	
	// == utilities ==
	function _getUrl_($node,$content,$prop=null,$withdomain=false,$encode=false,$decode=false)
	{		
		// add domain?
		if($this->getAttribute($isdomain,$node,'withDomain',$withdomain?'yes':'no') && $this->getBool($isdomain))
			$withdomain=true;
		else
			$withdomain=false;
		
		// parameters
		$this->getAttribute($params,$node,'params','');
		$params=trim($params);
		if ($params!='')
		{
			if ($this->getAttribute($parse,$node,'encode',$encode) && $this->getBool($parse))
			{
				$params="urlencode($params)"; 
			}
			if ($this->getAttribute($parse,$node,'decode',$decode) && $this->getBool($parse))
			{
				$params="preg_replace(array('%&amp;%','%[+]%'),array('&','%2B'),urldecode($params))"; 
			}

			$params=".$params";
		}
		$params.=trim($content);
		
		// url
		if ($this->getAttribute($url,$node,'url',null,true,true))
		{
			// url: absolute with domain (but we check anyway)
			$mode='absolute';
		}
		else
		{			
			// page
			if($page=$node->getAttribute('page'))
			{
				$page=$this->parseVal($page,true);
				
				// try to guess page mode
				if ($page[0]=='/')
					$mode='absolute';
				else
					$mode='relative';
					
				// if mode specified, then fine else we use our guess
				$this->getAttribute($mode,$node,'mode',$mode,true,false);
				
			}
			else
			{
				$page='$_SERVER["PHP_SELF"]';
				$mode='absolute';
			}
			
			$url=$page;		
		}
		
		// add domain name ?
		if ($withdomain)
		{
			// ltrim(dirname('/'),'\\/');
			if ($mode=='relative')
			{
				$url='rtrim(dirname($_SERVER[\'PHP_SELF\']),\'\\\\/\').\'/\'.'.$url;
				$mode='absolute';
			}
			
			// check if url has a domain prefix
			if (!preg_match('#^.?(https?|ftps?|file):[/][/]#',$url))	
				$url="\$this->getProperty('site.url.noprefix','').$url";
		}
		
		// operation and step
		if ( $this->getAttribute($op,$node,'operation') ||
			 $this->getAttribute($step,$node,'step'))
		{
			$op=$this->parseVal($node->getAttribute('operation','#prop:app.operation#'),true);	
			$step=$this->parseVal($node->getAttribute('step',''),true);
			$params=".'nxo='.{$op}.'&nxs='.{$step}{$params}";
		}

		// add ? if any param
		if ($params!='')
			$url="$url.'?'{$params}";
					
		return  $this->simplifyExpr($url);
	}
	
	// == utilities ==
	function _getUrl_old($node,$content,$prop=null,$withdomain=false)
	{		
		// parameters
		$this->getAttribute($params,$node,'params','');
		$params=trim($params);
		if ($params!='')
		{
			$params=".$params";
		}
		$params.=trim($content);
			

		// url
		if($this->getAttribute($url,$node,'url',null,true,true,false))
		{			
			$url.=$content;
			
			if (preg_match('#^.?(https?|ftps?|file):[/][/]#',$url))
				return $url;

			if ($url[1]=='/')
				$url=ltrim($url,'/');
			else if (preg_match('#[$]#',$url))
				return "(((\$u=$url)!=null) && !preg_match('#^(https?|ftps?|file):[/][/]#',\$u)? (NX_PROT.\"://{\$GLOBALS['NX_DOMAIN']}\".((\$u[0]=='/')?\$u:'/'.\$u)) : \$u)";
			elseif (preg_match('#^(https?|ftps?|file):[/][/]#',$url))
				return $url;
			else
				$url='dirname($_SERVER[\'PHP_SELF\']).\'/\'.'.$url;
				
			return 'NX_PROT."://{$GLOBALS[\'NX_DOMAIN\']}".'.$url;
		}
				
		// page
		$this->getAttribute($page,$node,'page',"''",true,true);

		/*
		$page=$node->getAttribute('page'))
		{
			$page=$this->parseVal($page,true);
		}
		else
//			$page='$_SERVER["REQUEST_URI"]';
			$page="''";
		*/
		
		if ($absolute  || (!$url && trim($page,"'")==''))
		{
			// add domain
			$url='NX_PROT."://{$GLOBALS[\'NX_DOMAIN\']}"';
			
			if ($page!="''")
			{
				// page defined: add url path
				$url.='.dirname($_SERVER[\'PHP_SELF\']).\'/\'.'.$page;
			}
			else
			{
				// no page (same script) add script name (w/ path)
				$url.='.$_SERVER[\'SCRIPT_NAME\']';
			}
		}
		else
		{
			$url=$page;
		}
						
		$this->getAttribute($params,$node,'params','');
		$params=trim($params);
		if ($params)
			$content.=$params;
			
		// operation and step
		$op=$this->parseVal($node->getAttribute('operation','#this:operation#'),true);
		$step=$this->parseVal($node->getAttribute('step',''),true);
						
		return  $this->simplifyExpr($url.".'?nxo='.{$op}.'&nxs='.{$step}{$content}");
	}
	
	// by default we pass through unknown elements
	function tag_default($node,$content,$level,$pad,$ns,$op)
	{
		nxltrace("no handler for instruction: {$ns}:{$op}",'warning');
		
		if ($node->nodeType!=ELEMENT_NODE)
			return '';
			
		if ($ns)
			$name="$ns:$op";
		else
			$name=$op;
			
		$atts='';
		foreach ($node->_getAttributes($node) as $an=>$av)
		{
			if ($an=='uri')
				continue;
				
			$av=$this->parseVal($av,false);			
			$atts .=" $an=\"$av\"";
		}


		// copy ns
		if ($node->nsdef)
			foreach ($node->nsdef as $ns)
			{
				if ($ns['prefix'])
					$atts .=" xmlns:{$ns['prefix']}=\"{$ns['uri']}\"";
				else
					$atts .=" xmlns=\"{$ns['uri']}\"";
			}
		
		if ($content!='')
			$res= "<{$name}{$atts}>$content</$name>";
		else
			$res= "<{$name}{$atts}/>";
			
		nxltrace("passing through:".htmlentities($res),'warning');
		return $res;
	}
	
	
}
?>
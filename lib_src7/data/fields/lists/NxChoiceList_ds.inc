<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/

// singleton
$GLOBALS['NxChoiceList_ds_docRecs']=array();

class NxChoiceList_ds 
{
	var $desc;
	var $props;
	
	// public interface
	
	function __construct(&$props,$name,$dir='lists',$ds='db:ds_category.list',$lang=null,$tpl_label='',$catField='cat.value')
	{
		global $NX_LANG;
		// if default lang, get it from properties
		if ($lang == null)
			$lang = $NX_LANG;
			
		$this->exts=array('html','inc');

		// manage package
		$_name = explode('@',$name);
		if(count($_name)>1)
		{
			$this->package=$_name[1];
			$this->name=$_name[0];
		}
		else
		{
			$this->package=$props->getProperty('nx.package','');
			$this->name=$name;
		}
			
		$this->props =& $props;
		$this->dir=ltrim($dir,'/\\');
		$this->ds=$ds;
		$this->lang=$lang;
		$this->tpl_label=$tpl_label;
		$this->catField = $catField;

		// get array
		$this->error = new NxErrorCB();
		
		$this->loadList(false);
	}
	
	function &getListAsArray()
	{
		return $this->desc;
	}
	
	function getListAsProperties()
	{
		return new NxProperties($this->desc,$this->props);
	}
	
	function getChoice($v)
	{
		if (isset($v) && isset($this->desc[$v]))
			return $this->desc[$v];
		elseif (isset($this->desc["?"]))
			return $this->desc["?"];
		else
			return '';
	}
	
	function addChoices($values)
	{	
		if (count($values)==0) return;
		
		// delete the whole content cache (because this ds could be used in other user's spaces)
		if ($GLOBALS['NX_CONTENT_COMP'] && is_dir($GLOBALS['NX_CONTENT_COMP']))
		{
			$fs=$this->props->getPlugIn('FileSys');
			$fs->rmDir($GLOBALS['NX_CONTENT_COMP']);
		}
		
		// get ds settings
		if ($this->ds!='')
		{
			$ads=explode(':',$this->ds);
			switch(count($ads))
			{
				case 2:
					list($media,$view)=$ads;
					break;
				case 1:
					list($view)=$ads;
					$media='db';
					break;
				default:
					$view='ds_category.list';
					$media='db';
					break;				
			}
		}
		else
		{
			$view='ds_category.list';
			$media='db';
		}

		// get db data source plugin		
		$ds =& $this->props->getPlugIn(strtolower($media),'NxDS_','ds');
		
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","DS_XDOCS");
			return null;
		}
		
		$ds->setProperty('cat.id',preg_replace('|/|','.',$this->props->getProperty('list',$this->name)));
		$width=$this->props->getProperty('ds_size',0,false);
		
		// get record desc
		$recdesc=$ds->getRecDesc($view, $this->error);
		
		// get field names for key and value
		$fkey=$this->catField;
		$flabel=$recdesc->getFieldNameByAlias('cat.label');
		
		// create array of records
		$arecords=array();
		foreach ($values as $k=>$v)
		{
			$arecords[]=array($fkey=>$k,$flabel=>$v);
		}
		
		require_once(NX_LIB.'NxData_ArrayRecords.inc');
		$records=new NxData_ArrayRecords($arecords,$recdesc,'array');
		
		// add records to DS (nb. assumes there will be no duplicates...)
		$ds->putRecords($records,$view,$this->error,'insert');
	}	

	
	// protected interface
	function loadList($refresh=true)
	{
		// load document
		// get property path
		$path = $this->dir.'/'.strtr($this->name,'.','/');
		$uid="$path@{$this->package}";
		
		// if already loaded, returns it
		if ($refresh || !isset($GLOBALS['NxChoiceList_ds_docRecs'][$uid]))
		{		
			nxLog("Build ds list : $uid",'DS_LIST');

			// get or build executable
			$exe =& $this->buildExe($path);
			if ($exe==null)
				return $exe;
			
			// execute it and get back an array of doc properties
			$GLOBALS['NxChoiceList_ds_docRecs'][$uid] =& $this->execute($exe);
		}
		
		$this->desc =& $GLOBALS['NxChoiceList_ds_docRecs'][$uid];

		// wrap it as record
		if ($this->desc == null)
		{
			$this->desc = array();
		}
	}

	function refreshList()
	{
		// get dir for the list		
		$fs=$this->props->getPlugIn('FileSys');
		
		// check if output directory exist
		$fs->rmDir($GLOBALS['NX_CONTENT_COMP'].$this->dir.'/'.strtr($this->name,'.','/'));
	}
	
	function &buildExe($path)
	{
		if (!($exePath = $this->getExePath($path)))
		{
			$this->error->addError("cant find content destination for category: $path");
			return null;
		}
		
		// if production mode (once) and output file exists => returns it
		/*
		if (NX_CHECK_CONTENT=='once' && is_file($exePath))
			// deployed once
			return $exePath;
		*/
				
		// if DS specified then try it first, else try static doc first
		$src=null;

		//load from Data source
		$src=$this->loadFromDS($path);

		if ($src)
		{
			// parse properties		
			$src=preg_replaceX(
				array(
					'/{(?:=|property:)([^}]+)}/',
					'/{(?:\$\$|global:)([^}]+)}/',
					'/{(?:const(?:ant):)([^}]+)}/',
					'/{(?:string:|_)"([^"]*)"(?:[:]"?([^"}]+)"?)?}/e',
				),
				array(
					"'.\$this->getProperty('$1').'",
					"'.\$GLOBALS['$1'].'",
					"'.$1.'",
					'$this->mapString("$1","$2")',
				),
				$src,$this);

			// save it as a php file
			$this->saveExe($src,$exePath);

		}
		else
		{
			// not found => error..
			$this->error->addError("cant find content source: $path");
			return null;
		}
					
		return $exePath;
	}	
		
	function getExePath($path)
	{
		global $NX_LANG;
		//global $NX_DEFT_LANG;
		if ($this->package)
			return $GLOBALS['NX_CONTENT_COMP']."{$path}_{$NX_LANG}@{$this->package}.inc";
		else
			return $GLOBALS['NX_CONTENT_COMP']."{$path}_{$NX_LANG}.inc";
	}

	function & execute($exe)
	{
		// execute PHP code (that initialises $list)
		if (isset($exe) && $exe != '')
		{
			include($exe);
			return $list;
		}
		return array();
	}

	function saveExe($src,$exePath)
	{
		$fs=$this->props->getPlugIn('FileSys');
		
		// check if output directory exist
		if (!is_dir($d=dirname($exePath)))
			if ($fs->createTree($d)==null)
			{
				$this->error->addError('cant create directory :'.$d);
				return false;
			}
			
		// transform it and write to file
		if ($fp = fopen($exePath,'wb'))
		{
			// parse and copy to file
			fputs($fp,$src);
			fclose($fp);
			
			nxLog("compiled content to $exePath","DS:LIST",NX_COMPILE_EVENT);
			return true;
		}
		else 
		{
			$this->error->addError('cant save content executable :'.$exePath);
			return false;
		}
	}
	
	function loadFromDS($path)
	{
		// get ds settings
		if ($this->ds!='')
		{
			$ads=explode(':',$this->ds);
			switch(count($ads))
			{
				case 2:
					list($media,$view)=$ads;
					break;
				case 1:
					list($view)=$ads;
					$media='db';
					break;
				default:
					$view='ds_category.list';
					$media='db';
					break;				
			}
		}
		else
		{
			$view='ds_category.list';
			$media='db';
		}

		// split media.instance
		$plug=explode('.',$media);
		if (count($plug)>1)
			$inst=$plug[1];
		else
			$inst=false;

		// get db data source plugin		
		$ds = $this->props->getPlugIn(strtolower($plug[0]),'NxDS_','ds',true,$inst);
		
		if (!$ds)
		{
			nxError("Can't find datasource: [$media]","DS_XDOCS");
			return null;
		}
		
		$ds->setProperty('cat.id',preg_replace('|/|','.',$this->props->getProperty('list',$this->name)));
		$width=$this->props->getProperty('ds_size',0,false);
	
		// get records
		$records = $ds->getRecords($view, $this->error,null,$ds);

		// ok we're in, we have the records, lets build the source now..
		$s='$list=array('; $i=0;
		$self=$this;
		if ($records != null && $records->count()>0) 
			for ($iter = $records->getRecordIterator(); $iter->isValid(); $iter->next())
			{
				$i++;
				$this->record =& $iter->getCurrent();
				
				// get value
				$this->f_v=$this->record->getField($this->catField);
				if (!$this->f_v)
					continue;
				$v=$this->f_v->toObject('format');
												
				// map template to label
				if ($this->tpl_label=='')
					// simple label from field cat.label
					$label=$this->mapField($this->record,'cat.label',$this->f_v);
				else
				{
					$label=preg_replace_callback_array(
						array(
							'/[{]field(.object)?([:?])([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?[}]/'=> 
								function($m) use($self) {
									$n=count($m);
									return $self->mapField($self->record,$m[3],$self->f_v,ltrim($m[1],'.'),$m[2],$n>4?$m[4]:'',$n>5?$m[5]:'');
								},
							'%\{(/?)(strong|i|span|div|br)( [a-z_0-9-]+)?(/?)\}%i'=>
								function($m) {
									if(ltrim($m[3]))
										return "<{$m[1]}{$m[2]} class=\"{$m[3]}\"{$m[4]}>";
									else
										return "<{$m[1]}{$m[2]}{$m[4]}>";
								},
							'%&nbsp;%i' => function(){return '';}
							), 
						$this->tpl_label
					);
				}
			/*				
			else
				{
					// complex label from properties, strings or fields
					// ex: "{string:user}:{field:first_name} {field:last_name}" 
					$label=preg_replaceX(
		
		array(
			'/{field.object([:?])([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?}/e',
			'/{field([:?])([^}|:=]+)(?:[:]([^}|=]+))?(?:[|=]([^}]+))?}/e',
			'%\{(/?)(strong|i|span|div|br) ([a-z_0-9-]+)(/?)\}%i',
			'%\{(/?)(strong|i|span|div|br)(/?)\}%i',
			'%&nbsp;%i',
			), 
		array(
			'$this->mapField($this->record,"$2",$this->f_v,"object","$1","$3","$4")',
			'$this->mapField($this->record,"$2",$this->f_v,"html","$1","$3","$4")',
			'<$1$2 class="$3"$4>',
			'<$1$2$3>',
			''
			),
		$this->tpl_label,$this);
				}
		*/
		
		$v=preg_replace("/'/","\\'",$v);
		
		if ($width>0)
			$label=substr($label, 0, $width);
												
				$s.= "'$v'=>'$label',\n";
			}
		$s.=');';

		nxLog("Built ds list $path : $i items({$this->catField},{$this->tpl_label})",'DS_LIST');
		
		return '<?php '.$s.'?>';
	}

	function mapString($str,$pack)
	{
		return $this->getString($str,null,$pack);
	}
	
	function mapProp($p,$isDeft,$dft,$static,$inPHP)
	{
		return $this->props->getProperty(trim($p),$dft);
	}
	
	// return label for field (or value if field not found)
	function mapField($record,$name,$fval=null,$method='html',$op=':',$pref='',$suf=''){

		if ($record)
		{
			$f=$record->getField($name);
			if (!$f)
				$f=$fval;			
		}
		else
			$f=$fval;
			
		if (!$f)
			return '';
			
		if(!$method) 
			$method='html';

		$method="to$method";
		$r= str_replace("'","\\'",$f->$method());

		if($op=='?' && $r)
		{
			$r = $pref.$r.$suf;
		}
			
		return $r;
	}

}

?>
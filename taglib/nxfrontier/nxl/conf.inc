<?PHP

/**
 * tag lib for creating resources. A reousrce is a file with a set of properties used to configure software.
 * 
 * 	   	<conf:resources path="ds" xmlns:conf="http://www.nxfrontier.com/nxl/conf">
			<conf:resource name="#{$name}#" mode="#{$mode}#">
				<conf:property name="#{$name}#.from" value="no_reply@nxfrontier.com" />
				<xsl:if test="$bcc!=''">
					<conf:property name="#{$name}#.bcc" value="#{$bcc}#" />
				</xsl:if>
				<conf:property name="#{$name}#.log" value="#{$is_log}#" type="boolean" />

				<conf:property name="#{$name}#.smtp.servers">
					<conf:array>
						<conf:properties>
							<conf:property name="host" value="#{$host}#" />
						</conf:properties>
					</conf:array>
				</conf:property>
			</conf:resource>
		</conf:resources>

 *
 */

class taglib_nxfrontier_nxl_conf extends NxTagLib
{		
	function _resources(&$node,&$level)
	{
		$node->setDeftAtt('path','/');
		$node->setDeftAtt('type','conf');
	}
	
	function _resource(&$node,&$level)
	{		
		// reset the automatic node level used for indentation
		$level=0;
	}
		
	/** Defines a "resource" file to be used when a bit of configuration is needed to be loaded.
	 * same type of config as NxSites, ie an array with keys to data.
	 * params:
	 *  @param path directory relative to NX_CONF or NX_GEN (depending on type). defaults to parent()/@path ("resources/@path")
	 * @param type conf or gen (where to store the file, default is conf)
	 * @param mode config mode (suffix relted to a server instance)
	 * @param lang language to be used. will be loaded with the language defined in Nx.
	 * @param name file name
	 * @param var name of variable used to instanciates this resource array (default to property)
	 * 
	 * example: 		
	   	<conf:resources path="ds" xmlns:conf="http://www.nxfrontier.com/nxl/conf">
			<conf:resource name="#{$name}#" mode="#{$mode}#">
				<conf:property name="#{$name}#.from" value="no_reply@nxfrontier.com" />
				<xsl:if test="$bcc!=''">
					<conf:property name="#{$name}#.bcc" value="#{$bcc}#" />
				</xsl:if>
				<conf:property name="#{$name}#.log" value="#{$is_log}#" type="boolean" />

				<conf:property name="#{$name}#.smtp.servers">
					<conf:array>
						<conf:properties>
							<conf:property name="host" value="#{$host}#" />
						</conf:properties>
					</conf:array>
				</conf:property>
			</conf:resource>
		</conf:resources>

	 *  
	 * */
	function resource(&$node,$content,$level,$pad)
	{
		// path
		if (!$this->getAttribute($path,$node,'path'))
		{
			$this->getAttribute($path,$this->context->getParentElement($node),'path','/');
		}
				
		// name
		if (!$this->getAttribute($name,$node,'name'))
		{
			return $this->fatal('name attribute required');
		}
		
		// mode (suffix used for resources or device mapping)
		if($this->getAttribute($mode,$node,'mode','') && $mode!='')
			$name.="_$mode";
		
		// lang
		if ($this->getAttribute($lang,$node,'lang') ||
			$this->getAttribute($lang,$this->context->getParentElement($node),'lang'))
			$lang="_$lang";
		else
			$lang='';
		
		
		// location type: conf, gen
		if (!$this->getAttribute($type,$node,'type'))
		{
			$this->getAttribute($type,$this->context->getParentElement($node),'type','conf');
		}

		if (strtolower($type)=='gen')
			$fpath = NX_GEN;
		else
			$fpath = NX_CONF;
			
		// file path
		$fpath .= trim($path,'/')."/{$name}$lang.inc";
		
		// variable name
		$this->getAttribute($var,$node,'var','properties');
				
		$src= '<?php
/* file automatically generated from NXL source. Do not edit it. */
';
		$src .= "\${$var}=array(\n$content);";
		$src .= '?>';
		
		$this->context->saveSrc($fpath,$this->cleanupSrc($src));

		nxltrace("RESOURCE: $fpath",'resource');	
		return '';
	}
	
	function output($node,$content,$level,$pad)
	{
		switch ($node->getAttribute('method','html'))
		{
			case 'xml2text':
				return '?>'.nl2br(htmlspecialchars($content)).'<?php';
			default:
				return '?>'.$content.'<?php';
		}		
	}	

	// <property name="part.new" value="#param:ptype#.edit" />
	function property($node,&$content,$level,$pad)
	{		
		if (!$this->getAttribute($n,$node,'name',null,true,true))
			return $this->fatal('name attribute required');
			
		if (!$this->getAttribute($v,$node,'value',null,true,true))
		{
			if ($content=='')
				return $this->fatal('value attribute required');
			else
				$v="\n$content";
		}
		else
		{
			if ($this->getAttribute($type,$node,'type',true,true))
			{
				switch(strtolower($type))
				{
					case 'bool':case 'boolean':
						$v=trim($v,"'");
					break;
				}
			}

			$v.=",\n";
		}
						
		return "{$pad}$n=>$v";
	}
	
	// <array name="part.new" value="#param:ptype#.edit" />
	function properties($node,&$content,$level,$pad)
	{		
		if ($this->getAttribute($n,$node,'name',null,true,true))
			return "{$pad}$n=>array(\n$content{$pad}),\n";
		else
			return "{$pad}array(\n$content{$pad}),\n";
	}
	
	// <array>" />
	function array_($node,&$content,$level,$pad)
	{		
		if ($this->getAttribute($n,$node,'name',null,true,true))
			return "{$pad}$n=>array(\n$content),\n";
		else
			return "{$pad}array(\n$content{$pad}),\n";
	}
	
	// <data name="part.new" value="#param:ptype#.edit" media="db"/>
	function data($node,&$content,$level,$pad)
	{		
		if (!$this->getAttribute($n,$node,'name',null))
			return $this->fatal('name attribute required');
			
		if (!$this->getAttribute($v,$node,'value',null))
			return $this->fatal('value attribute required');
	
		$src="{$pad}'{$n}.db'=>$v,\n";
		
		if (($v=$node->getAttribute('media'))!==null)
		{
			$v=$this->parseVal($v,true);
			$src.="{$pad}'{$n}.media'=>$v,\n";
		}
		
		return $src;
	}
	
	// <view name="page.view" value="#param:ptype#.edit" />
	function view($node,&$content,$level,$pad)
	{
		if (($n=$node->getAttribute('name'))===null)
			return $this->fatal('name attribute required element');
			
		if (!preg_match('/[.]view$/',$n))
			$n .= '.view';
		
		if (($v=$node->getAttribute('value'))===null)
			return $this->fatal('value attribute required');
		$v=$this->parseVal($v,true);		
			
		return "{$pad}'{$n}'=>$v,\n";
	}

	function cleanupSrc($src) {
		return preg_replace(
			array('/<\?php\s*\?>/','/[.]\'\'[.]/'),
			array('','.'),
			$src);
	}
}

?>
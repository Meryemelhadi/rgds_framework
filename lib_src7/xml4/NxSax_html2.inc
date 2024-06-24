<?php
/* 
	This SAX parser does not check closing tags and xml entities.
	It can treat html as plain text (CDATA).
	
	It has been created from the original class HtmlParser 
	from J. Solorzano (cf. copyrights below), with following modifications:
		- fixed comments
		- fixed tag names recognition (ns support)
		- support of empty tags( closed by />)
		- added cdata recognition
		- wrapped it as a SAX parser compatible with php SAX parser.
*/

/*
 * Copyright (c) 2003 Jose Solorzano.  All rights reserved.
 * Redistribution of source must retain this copyright notice.
 *
 * Jose Solorzano (http://jexpert.us) is a software consultant.
 *
 * Contributions by:
 * - Leo West (performance improvements)
 */
 
/*
define ("NODE_TYPE_START",0);
define ("NODE_TYPE_ELEMENT",1);
define ("NODE_TYPE_ENDELEMENT",2);
define ("NODE_TYPE_TEXT",3);
define ("NODE_TYPE_COMMENT",4);
define ("NODE_TYPE_DONE",5);
define ("NODE_TYPE_CDATA",6);
define ("NODE_TYPE_PI",7);
define ("NODE_TYPE_EMPTY_ELEMENT",8);
*/

define ('NODE_TYPE_START','start');
define ('NODE_TYPE_ELEMENT','elem');
define ('NODE_TYPE_ENDELEMENT','end_elem');
define ('NODE_TYPE_EMPTY_ELEMENT','empty_elem');
define ('NODE_TYPE_TEXT','#text');
define ('NODE_TYPE_COMMENT','#comment');
define ('NODE_TYPE_DONE','done');
define ('NODE_TYPE_CDATA','#cdata');
define ('NODE_TYPE_PI','#pi');

 
class NxSax_html
{
	function NxSax_html(&$cb) {
		$this->cb=&$cb;
	}
	
	function close() {
	}
	
	// parse xml or html source.
	// if ignoreHTML is true, transform all tags without namespace prefix into simple text.
	// if capitalise = true, makes all attributes names in upper case for compatibilty with PHP saxer.
	function parse($src,$htmlOpt='',$capAttr=true) 
	{	
		// prepare parser
		$src=$this->preprocess($src);
		$this->init($src);				
		
	    while ($this->parseNode()) {
	    	switch ($this->iNodeType)
	    	{
	    		case NODE_TYPE_TEXT:
	    		case NODE_TYPE_CDATA:
					$this->cb->cdata($this,$this->iNodeValue);
	    			break;
	    		case NODE_TYPE_COMMENT:
					$this->cb->_comment_open();
					$this->cb->cdata($this,$this->iNodeValue);
					$this->cb->_comment_close();
	    			break;
	    		case NODE_TYPE_ELEMENT:
	    		case NODE_TYPE_EMPTY_ELEMENT:
	    			$tagName=$this->iNodeName;
	    			$isNs=strpos($tagName,':')!==FALSE;

	    			if ($htmlOpt=='text' && !$isNs)
	    			{
	    				// process as text
				        $text="<{$tagName}";
		                $attrValues = $this->iNodeAttributes;
		                $attrNames = array_keys($attrValues);
		                $size = count($attrNames);
		                for ($i = 0; $i < $size; $i++) {
		                        $name = $attrNames[$i];
		                        $text.=" {$attrNames[$i]}=\"{$attrValues[$name]}\"";
		                }
				        $text.='>';
				        $this->cb->cdata($this,$text);
				        return true;
	    			}
    				// capitalise attributes
    				if ($capAttr)
    				{
    					$attrs=array();
    					foreach ($this->iNodeAttributes as $k=>$v)
    					{
    						$attrs[strtoupper($k)]=$v;
    					}
    				}
    				else
    					$attrs=$this->iNodeAttributes;
    				
    				// process as element
    				$this->cb->tag_open($this, $tagName, $attrs);
    				
    				// if no content or flatten html, close tag
    				if ($this->iNodeType==NODE_TYPE_EMPTY_ELEMENT ||
    					($htmlOpt=='flatten' && !$isNs))
	    				$this->cb->tag_close($this, $tagName);
	    			break;
	    		case NODE_TYPE_ENDELEMENT:
	    			$tagName=$this->iNodeName;
	    			$isNs=strpos($tagName,':')!==FALSE;
	    			if ($htmlOpt=='text' && !$isNs)
	    			{
	    				// process as text
				        $text.="</{$tagName}>";
				        $this->cb->cdata($this,$text);
				        return true;
	    			}

	    			// if flatten html, treat closing tag as an opening and empty tag
	    			if ($htmlOpt=='flatten' && !$isNs)
	    			{
	    				$attrs=array('tag'=>$tagName,'pmethod'=>'flatten');
	    				// $attrs=array('tag'=>$tagName);
	    				$tagName="close_$tagName";
	    				$this->cb->tag_open($this, $tagName, $attrs);
	    			}
	    			
    				// process as element
					$this->cb->tag_close($this, $tagName);
			        
			        break;
			    default:
			    	// echo "[other...]";
			    	break;
	        }
	    }		
		return true;
	}
	
	// parse and dump result as html or text (rewrite tags so that they can be displayed
	// in an html page). Return result as a string
	function dump($src,$display=true) 
	{	
	    if ($display)
	    {
		    $gt='&gt;';
		    $lt='&lt;';
		    $res= "<pre>";
	    }
	    else
	    {
		    $gt='>';
		    $lt='<';
		    $res= '';
	    }
		
		// prepare parser
		$src=$this->preprocess($src);
		$this->init($src);				
		
	    while ($this->parseNode()) {
	    	switch ($this->iNodeType)
	    	{
	    		case NODE_TYPE_TEXT:
	    			$res.= $this->iNodeValue;
	    			break;
	    		case NODE_TYPE_CDATA:
	    			$res.= "$lt![CDATA[{$this->iNodeValue}]]$gt";
	    			break;
	    		case NODE_TYPE_COMMENT:
	    			$res.= "$lt!--{$this->iNodeValue}--$gt";
	    			break;
	    		case NODE_TYPE_ELEMENT:
	    		case NODE_TYPE_EMPTY_ELEMENT:
			        $res.= "$lt" . $this->iNodeName;
	                $attrValues = $this->iNodeAttributes;
	                $attrNames = array_keys($attrValues);
	                $size = count($attrNames);
	                for ($i = 0; $i < $size; $i++) {
	                        $name = $attrNames[$i];
	                        $res.= " {$attrNames[$i]}=\"{$attrValues[$name]}\"";
	                }
	                
    				if ($this->iNodeType==NODE_TYPE_EMPTY_ELEMENT)
			        	$res.= " /$gt";
			        else
			        	$res.= "$gt";
	    			break;
	    		case NODE_TYPE_ENDELEMENT:
			        $res.= "$lt/" . $this->iNodeName . "$gt";
			        break;
			    default:
			    	// $res.= "[other...]";
			    	break;
	        }
	    }		
		return $res;
	}
		
	function getCurrentLine()
	{
		return 0;
	}
		
	// --------- utilities -------
	// transform comments into element and add entity references found in doc.
	function preprocess($src)
	{
		return preg_replace(
				array(
					'/<!--/s','/-->/'
					),
				array(
					'<_comment><![CDATA[',']]></_comment>'),
				$src);
	}
	
// class HtmlParser {

    /**
     * Field iNodeType.
     * May be one of the NODE_TYPE_* constants above.
     */
    var $iNodeType;

    /**
     * Field iNodeName.
     * For elements, it's the name of the element.
     */
    var $iNodeName = "";

    /**
     * Field iNodeValue.
     * For text nodes, it's the text.
     */
    var $iNodeValue = "";

    /**
     * Field iNodeAttributes.
     * A string-indexed array containing attribute values
     * of the current node. Indexes are always lowercase.
     */
    var $iNodeAttributes;

    // The following fields should be 
    // considered private:

    var $iHtmlText;
    var $iHtmlTextLength;
    var $iHtmlTextIndex = 0;
    var $iHtmlCurrentChar;
    var $BOE_ARRAY;
    var $B_ARRAY;
    var $BOS_ARRAY;
    
    /**
     * Constructor.
     * Constructs an HtmlParser instance with
     * the HTML text given.
     */
    function init($aHtmlText) {
        $this->iHtmlText = $aHtmlText;
        $this->iHtmlTextLength = strlen($aHtmlText);
        $this->iNodeAttributes = array();
        $this->setTextIndex (0);

        $this->BOE_ARRAY = array (" ", "\t", "\r", "\n", "=" );
        $this->B_ARRAY = array (" ", "\t", "\r", "\n" );
        $this->BOS_ARRAY = array (" ", "\t", "\r", "\n", "/" );
    }

    /**
     * Method parseNode.
     * Parses the next node. Returns false only if
     * the end of the HTML text has been reached.
     * Updates values of iNode* fields.
     */
    function parseNode() {
        $text = $this->skipToElement();
        if ($text != "") {
            $this->iNodeType = NODE_TYPE_TEXT;
            $this->iNodeName = "Text";
            $this->iNodeValue = $text;
            return true;
        }
        return $this->readTag();
    }

    function clearAttributes() {
        $this->iNodeAttributes = array();
    }

    function readTag() {
        if ($this->iCurrentChar != "<") 
        {
            $this->iNodeType = NODE_TYPE_DONE;
            return false;
        }
        $this->clearAttributes();
        $this->skipMaxInTag ("<", 1);
        if ($this->iCurrentChar == '/') {
            $this->moveNext();
            $name = $this->skipToBlanksInTag();
            $this->iNodeType = NODE_TYPE_ENDELEMENT;
            $this->iNodeName = $name;
            $this->iNodeValue = "";            
            $this->skipEndOfTag();
            return true;
        }
        $name = $this->skipToBlanksOrSlashInTag();
        if (!$this->isValidTagIdentifier ($name)) 
        {
                $comment = false;
                if (strpos($name, "!--") === 0) 
                {
                    $lenName=strlen($name);
                    if (strpos($name, "--", 3) === ($lenName - 2)) 
                    {
                        $this->iNodeType = NODE_TYPE_COMMENT;
                        $this->iNodeName = "#comment";
                        
                    	if ($lenName==5)
	                    	// empty comment...
	                        $this->iNodeValue = '';
	                    else
	                    {
	                    	// extract comment content
	                    	$this->iNodeValue=substr($name,3,$lenName-5);
	                    }
	                    	
				        $this->skipEndOfTag();
                        return true;                    	
                    }
                    else
                    {
                    	// normal comment                    	
                        $this->iNodeType = NODE_TYPE_COMMENT;
                        $this->iNodeName = "#comment";
                        $rest=$this->skipToStringInTag ("-->");
                        if ($rest!='')
							// get rid of end comment
                        	$rest=substr($rest,0,strlen($rest)-3);

                        $this->iNodeValue = substr($name,3).$rest;    
                        // Already skipped end of tag
                        return true;
                    }
                }
                // <![CDATA[...]]>
                elseif(strpos($name, "![CDATA[") === 0) 
                {
                    $lenName=strlen($name);
                    if (strpos($name, "]]", 8) === ($lenName - 2)) 
                    {
                        $this->iNodeType = NODE_TYPE_CDATA;
                        $this->iNodeName = "#cdata";
                        
                    	if ($lenName==10)
	                    	// empty comment...
	                        $this->iNodeValue = '';
	                    else
	                    {
	                    	// extract comment content
	                    	$this->iNodeValue=substr($name,8,$lenName-10);
	                    }
	                    	
				        $this->skipEndOfTag();
                        return true;                    	
                    }
                    else
                    {
                    	// normal cdata                    	
                        $this->iNodeType = NODE_TYPE_CDATA;
                        $this->iNodeName = "#cdata";
                        $rest=$this->skipToStringInTag ("]]>");
                        if ($rest!='')
							// get rid of end comment
                        	$rest=substr($rest,0,strlen($rest)-3);

                        $this->iNodeValue = substr($name,8).$rest;    
                        // Already skipped end of tag
                        return true;
                    }
                }
                // <?xml > processing instruction
                elseif(strpos($name, '?') === 0) 
                {
                	// processing intruction, ignore it for now..
                    $this->iNodeType = NODE_TYPE_PI;
                    $this->iNodeName = substr($name,1);
                    $this->iNodeValue='';
			        $this->skipEndOfTag();   
			        return true;             	
                }
                // other < characters in code (loose check => allow them)
                else {
                    $this->iNodeType = NODE_TYPE_TEXT;
                    $this->iNodeName = "#text";
                    $this->iNodeValue = "<" . $name;
                    return true;
                }
        }
        else {
                $this->iNodeType = NODE_TYPE_ELEMENT;
                $this->iNodeValue = "";
                $this->iNodeName = $name;
                while ($this->skipBlanksInTag())
                {
                    $attrName = $this->skipToBlanksOrEqualsInTag();
                    if ($attrName!='')
	                    if ($attrName=='/')
	                    	// empty element
	                    	$this->iNodeType = NODE_TYPE_EMPTY_ELEMENT;
	                    else
	                    {
	                        $this->skipBlanksInTag();
	                        if ($this->iCurrentChar == "=") 
	                        {
	                            $this->skipEqualsInTag();
	                            $this->skipBlanksInTag();
	                            $value = $this->readValueInTag();
	                            $this->iNodeAttributes[strtolower($attrName)] = $value;
	                        }
	                        else 
	                        {
	                            $this->iNodeAttributes[strtolower($attrName)] = "";
	                        }
	                    }
                }
        }
        $this->skipEndOfTag();
        return true;            
    }

    function isValidTagIdentifier ($name) {
        return preg_match('|^[A-Za-z0-9_:\-]+$|', $name);
    }
    
    function skipBlanksInTag() {
        return "" != ($this->skipInTag ($this->B_ARRAY));
    }

    function skipToBlanksOrEqualsInTag() {
        return $this->skipToInTag ($this->BOE_ARRAY);
    }

    function skipToBlanksInTag() {
        return $this->skipToInTag ($this->B_ARRAY);
    }

    function skipToBlanksOrSlashInTag() {
        return $this->skipToInTag ($this->BOS_ARRAY);
    }

    function skipEqualsInTag() {
        return $this->skipMaxInTag ("=", 1);
    }

    function readValueInTag() {
        $ch = $this->iCurrentChar;
        $value = "";
        if ($ch == "\"") {
            $this->skipMaxInTag ("\"", 1);
            $value = $this->skipToInTag ("\"");
            $this->skipMaxInTag ("\"", 1);
        }
        else if ($ch == "'") {
            $this->skipMaxInTag ("'", 1);
            $value = $this->skipToInTag ("'");
            $this->skipMaxInTag ("'", 1);
        }                
        else {
            $value = $this->skipToBlanksInTag();
        }
        return $value;
    }

    function setTextIndex ($index) {
        $this->iHtmlTextIndex = $index;
        if ($index >= $this->iHtmlTextLength) {
            $this->iCurrentChar = -1;
        }
        else {
            $this->iCurrentChar = $this->iHtmlText{$index};
        }
    }

    function moveNext() {
        if ($this->iHtmlTextIndex < $this->iHtmlTextLength) {
            $this->setTextIndex ($this->iHtmlTextIndex + 1);
            return true;
        }
        else {
            return false;
        }
    }

    function skipEndOfTag() {
        while (($ch = $this->iCurrentChar) !== -1) {
            if ($ch == ">") {
                $this->moveNext();
                return;
            }
            $this->moveNext();
        }
    }

    function skipInTag ($chars) {
        $sb = "";
        while (($ch = $this->iCurrentChar) !== -1) {
            if ($ch == ">") {
                return $sb;
            } else {
                $match = false;
                for ($idx = 0; $idx < count($chars); $idx++) {
                    if ($ch == $chars[$idx]) {
                        $match = true;
                        break;
                    }
                }
                if (!$match) {
                    return $sb;
                }
                $sb .= $ch;
                $this->moveNext();
            }
        }
        return $sb;
    }

    function skipMaxInTag ($chars, $maxChars) {
        $sb = "";
        $count = 0;
        while (($ch = $this->iCurrentChar) !== -1 && $count++ < $maxChars) {
            if ($ch == ">") {
                return $sb;
            } else {
                $match = false;
                for ($idx = 0; $idx < count($chars); $idx++) {
                    if ($ch == $chars[$idx]) {
                        $match = true;
                        break;
                    }
                }
                if (!$match) {
                    return $sb;
                }
                $sb .= $ch;
                $this->moveNext();
            }
        }
        return $sb;
    }

    function skipToInTag ($chars) {
        $sb = "";
        while (($ch = $this->iCurrentChar) !== -1) {
            $match = $ch == ">";
            if (!$match) {
                for ($idx = 0; $idx < count($chars); $idx++) {
                    if ($ch == $chars[$idx]) {
                        $match = true;
                        break;
                    }
                }
            }
            if ($match) {
                return $sb;
            }
            $sb .= $ch;
            $this->moveNext();
        }
        return $sb;
    }

    function skipToElement() {
        $sb = "";
        while (($ch = $this->iCurrentChar) !== -1) {
            if ($ch == "<") {
                return $sb;
            }
            $sb .= $ch;
            $this->moveNext();
        }
        return $sb;             
    }

    /**
     * Returns text between current position and $needle,
     * inclusive, or "" if not found. The current index is moved to a point
     * after the location of $needle, or not moved at all
     * if nothing is found.
     */
    function skipToStringInTag ($needle) {
        $pos = strpos ($this->iHtmlText, $needle, $this->iHtmlTextIndex);
        if ($pos === false) {
            return "";
        }
        $top = $pos + strlen($needle);
        $retvalue = substr ($this->iHtmlText, $this->iHtmlTextIndex, $top - $this->iHtmlTextIndex);
        $this->setTextIndex ($top);
        return $retvalue;
    }
}

?>
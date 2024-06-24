<?php

class Action
{
	var $attr;
	function __construct($attr)
	{
		$this->$attr=$attr;
	}

	function getAttribute($n,$deft=null)
	{
		if (isset($attr[$n]))
			return $attr[$n];
		return null;
	}
}

class NxActions
{
	function error($msg)
	{
		echo "$msg<br/>\n";
		return "/* Error: $msg\n */";
	}
}

class nxlib_img_gd extends NxActions
{
	// returns the image as an image stream
	function send($action,$content,$level)
	{
		return '<?php Header("Content-type: image/'.$this->type.'");'.
					$content.'?>';		
	}
	
	// save the PHP source
	function save($action,$content,$level)
	{
		$path=$action->getAttribute('path','/images');
		$this->file=$path."/{$this->name}.{$this->type}";
		
		$fp=fopen($this->file,"w");
		fwrite($fp, $content);
		fclose($fp);
		
		return $content;		
	}
	
	// generate GD commands (replace "echo" by actual GD commands...)
	//  - create the image
	//  - include drawing commands
	//  - close the image
	//  - save it as a file
	function image($action,$content,$level)
	{
		$name=$action->getAttribute('name','no_name');
		$this->type=$action->getAttribute('type','jpeg');		
		return "echo \"creating image: $name\";\n{$content} echo \"end image\n\"";
	}

	// generate GD commands:
	//  - load font
	//  - write text following required formatting options:
	//		+ color
	// 		+ font
	//		+ dimensions, etc.
	function text($action,$content,$level)
	{
		$value=$action->getAttribute('value');
		if ($value===null)
			return $this->error('"value" attribute required');

		return 'echo "creating text: ".value;';
	}
}

//===============  testing ===================
$lib = new nxlib_img_gd();

// create text
$content = $lib->text(
	new Action(array(value=>"Title",size=>"20",boxW=>"250",overlapX=>"compress",	font=>"Verdana,Arial"),'',3));

// create image
$content = $lib->image(
	new Action(array(name=>"title", path=>"/images", type=>"png", background=>"blue",	width=>"*", height=>"*"),$content,2));

// wrap it with headers
$content = $lib->send(new Action(array(),$content,1));

// store PHP file
$content = $lib->save(
	new Action(array(path=>"/images/buttons"),$content,0));

// execute commands and return image to calling browser
include($lib->file);

?>
<?php
// TTF button from PHP_button project on source forge.
// modifications:
// 	franck@nxfrontier.com: constructor parameters and show() with param
//

class	TTFButton
{
	// image Handles
	var	$hNew ;
	var	$hButton;
	var	$hIconL;
	var	$hTxt;
	var $hIconR;
	
	var $background_image = null;
	var $sIconL = "" ;
	var $sIconR = "" ;

	var $margin_top = 0 ; 
	var $margin_bottom = null ;
	var $margin_left = null ;
	var $margin_right = null;
	var $width = 0;
	var $padding_left = null ;
	var $padding_right = null;
	var $padding_top = null;
	var $padding_bottom = null;
		
	var	$text = 'Demo Text';
	var $font_family ='arial';
	var $font_size = 12 ;
	var $angle=0;
	var $font_style = 'normal' ;
	var $color = "#000000" ;
	var $transparent_color=null;
	var $background_color='#c8c8c8';
	
	/*
	
example with images genrated in skin dir:

<tpl:button-image name="submit" label="Save" class="mac_blue" cache="{const:NX_IMAGES}"/>
 	generates
<a href="{=submit.url}" title="Save"><img src="{@NX_IMAGES}/buttons/mac_blue/Save.gif" />
	and
NX_CONF/views/NX_SKIN/images/buttons/mac_blue/desc.inc
	contains:
	
	$properties=array(
		'background_image' => "spacer.gif",
		'sIconL' => "",
		'sIconR' => "",
	
		'margin_top' => 0,
		'margin_bottom' => 0,
		'margin_left' => 15 ,
		'margin_right' => 15,
			
		'text' => 'Demo Text',
		'font_family' => 'arial',
		'font_size' => 12,
		'color' => "#000000"
	);
	
	so:
	
	include(NX_CONF/views/NX_SKIN/images/buttons/mac_blue/desc.inc);
	$text=basename($url);
	$btn = new TTFButton( $text, NX_CONF.'/fonts', NX_IMAGES, $properties) ;
	$btn->show();
	
	store image in skin dir
	
	*/
	
	function TTFButton($txt="", $font_dir, $img_dir, $props=null)
	{	
		// add descriptor props
		if (is_array($props))
		{		
			foreach ($props as $n=>$v)
			{
				$n=preg_replace('/[\-.]/','_',strtolower($n));
				$this->$n=$v;
			}
		}
		
		if (strtolower($this->font_style)=='italic')
			$this->angle=30;

		// set text and image
		if( $txt ) $this->text = ( ord(trim($txt)) > 127 ) ? $this->gb2utf8($txt) : $txt;
		// if( $background_image ) $this->background_image = $background_image;
		
		// find image and font in conf
		if ($this->background_image)
			$this->background_image = $img_dir.$this->background_image;
		
		$this->ttf_font="{$font_dir}{$this->font_family}.ttf";
		if (!is_file($this->ttf_font)) {
			$this->ttf_font="{$font_dir}{$this->font_family}.TTF";

			if (!is_file($this->ttf_font))
			{
				nxError("Can't find font:{$this->ttf_font}","GD");
				return;
			}
		}
			
		nxLog("loading font: {$this->ttf_font}","GD");
			
		// Set the enviroment variable for GD
		// putenv('GDFONTPATH=' . realpath($font_dir));

	}


	function ripper()
	{		
		
		if ($this->background_image)
		{
			$this->hButton = $this->imageCreateFromImg( $this->background_image );
		
			// get transparent colour from button
			$colorTransparent = imageColorTransparent($this->hButton);
		}
		
		$this->txt2img() ;
		
		if (!$this->background_image)
			return;
		
		$nTxt = imageSX( $this->hTxt );
		
		$nLeft = $this->margin_left!==null ? $this->margin_left : 6 ;
		
		// fy: remove this margin and add it directly in text image..
		$nRight = $this->margin_right!==null ? $this->margin_right : 6 ;
		
		$nIconMargin = 4 ;	
		if( is_file( $this->sIconL ) ) {
			$this->hIconL = $this->imageCreateFromImg( $this->sIconL ) ;
			$nIconL =  imageSX( $this->hIconL ) + $nIconMargin;
		}
		else
			$nIconL = 0;
		
		if( is_file( $this->sIconR ) ) {
			$this->hIconR = $this->imageCreateFromImg( $this->sIconR ) ;
			$nIconR =  imageSX( $this->hIconR ) + $nIconMargin;
		}
		else
			$nIconR = 0;

		// have we got a fixed size for the button?
		$newX = $nLeft + $nIconL + $nTxt + $nIconR + $nRight;
		if ($this->width > $newX)
		{
			$wTextBox=$this->width-$newX+$nTxt;
			$wTextOffset=floor(($wTextBox-$nTxt)/2);
			$newX = $nLeft + $nIconL + $wTextBox + $nIconR + $nRight;
		}
		else
		{
			$wTextBox=$nTxt;
			$wTextOffset=0;
			$newX = $nLeft + $nIconL + $wTextBox + $nIconR + $nRight;
		}
		
		$newX = $nLeft + $nIconL + $wTextBox + $nIconR + $nRight;
		if ($this->background_image)
		{
			$newY = imageSY($this->hButton) ;
		}
		else {
			$newY = imageSY($this->hTxt);
		}
		
		$this->hNew = imageCreate( $newX, $newY );		

		if ($this->background_image)
		{
			// Cut Button Left
			imageCopy( $this->hNew, $this->hButton,	   0, 0,     0, 0,    $nLeft, $newY );  
			if ($this->transparent_color)
			{
				// try to pick transp colour from existing image
				$colorTransp=$this->getExactColor($this->hNew,$this->transparent_color);
				if ($colorTransp<0)
					$colorTransp=$this->getcolor($this->hNew,$this->transparent_color);
					
				imageColorTransparent( $this->hNew, $colorTransp);
			} 
			
			// Background of IconL + Text + IconR
			imageCopy( $this->hNew, $this->hButton,  $nLeft, 0,     $nLeft, 0,     $nIconL+$wTextBox+$nIconR, $newY ); 
	
			if( $this->hIconL ) :
				$dst_y = $this->dst_y( $this->hIconL );
				imageCopy( $this->hNew, $this->hIconL,
					$nLeft, $dst_y,
					0, 0,     $nIconL-$nIconMargin, imageSY($this->hIconL)   ); // Copy IconL
			endif;
		}
		// add the text
		$dst_y = $this->dst_y( $this->hTxt ); 
		imageCopy( $this->hNew, $this->hTxt,$nLeft+$nIconL+$wTextOffset, $dst_y, 0, 0, $nTxt, imageSY($this->hTxt)   );

		if( $this->hIconR ) :
			$dst_y = $this->dst_y( $this->hIconR ); 
			imageCopy( $this->hNew, $this->hIconR,	   $nLeft+$nIconL+$wTextBox+$nIconMargin, $dst_y,     0, 0,     $nIconR, imageSY($this->hIconR) ); // Copy IconR
		endif;

		if ($this->background_image)
		{
			imageCopy( $this->hNew, $this->hButton, $nLeft+$nIconL+$wTextBox+$nIconR, 
				0, imageSX($this->hButton)-$nRight, 0, $nRight, $newY ); // Cut Button Right
		}
	}
	
	function	dst_y( $hImg )
	{
		if (isset($this->hButton)) {
			$valid_height = imageSY( $this->hButton ) - $this->margin_top - $this->margin_bottom ;
			$dst_y = $this->margin_top+Floor( ( $valid_height - imageSY($hImg) ) / 2 );
		}
		else
			$dst_y = imageSY( $hImg );
			
		return $dst_y ;
	}
	
	
	function	imageCreateFromImg( $filename )
	{
		$split = explode( "\.", basename( trim( $filename ) ) );
		$ext = $split[ count($split) - 1 ] ;
		switch( strtolower( $ext ) )
		{
			case	"gif" :
				return imageCreateFromGif( $filename );
				break;
			case 	"png" :
				return imagecreatefrompng( $filename );
				break;
			case 	"jpg" :
			case 	"jpeg" :
				return imagecreatefromjpeg( $filename );
				break;
		}
	}
	
	// if $path not null store image in dir before sending
	function show($path=null)
	{
		$this->ripper();
		
		$imgH = isset($this->hNew)?$this->hNew :$this->hTxt ;
		
		// get image type
		if ($this->background_image)
		{
			$split = explode( "\.", basename( trim( $this->background_image ) ) );
			$ext = strtolower($split[ count($split) - 1 ]) ;
			switch($ext)
			{
				case 	"jpg" :
				case 	"jpeg" :
					$type='jpeg';
				default:
					$type=$ext;
					break;
			}
		}
		else
			$type='png';
		
		// create/send image
        $functionName   = 'Image' . $type;
        if (!function_exists($functionName))
        { 
			// dont send header if output to file
			if (!$path)
				Header( "Content-Type: image/png" );
	        
			ImagePNG($imgH, $path) ;
        }
        else
        {
			if (!$path)
				Header( "Content-Type: image/$type");
				
		   $functionName($imgH, $path,80) ;
        }
		
		$this->cleanup();
	}
	
	
	function	cleanup()
	{		
		if ($this->hNew)
			imageDestroy($this->hNew );
			
		imageDestroy( $this->hTxt);
		
		if ($this->hButton)
			imageDestroy( $this->hButton );
			
		if( $this->hIconL ) imageDestroy( $this->hIconL );
		if( $this->hIconR ) imageDestroy( $this->hIconR );
		//print "New Y = " . imageSY( $this->hNew ) . " TXT Y = " . imageSY($this->hTxt) . " /2 = " . Floor((imageSY($this->hNew)-imageSY($this->hTxt))/2);
	}


	function	txt2img()
	{
		$hImg_temp = imageCreate( 10, 10 );
		$txt_color = $this->txtcolor( $hImg_temp );
		
		$angle=$this->angle;
		
		$p4 = imageTTFText( $hImg_temp, $this->font_size, $this->angle, 0, 0, $txt_color, $this->ttf_font, $this->text );	
		// fy: check the box height (baseline etc.)
		$pbox=imagettfbbox ($this->font_size, $this->angle,$this->ttf_font, "RegisteraAbBCgjpPyY!?<>");

		$p4=imagettfbbox ($this->font_size, $this->angle,$this->ttf_font, $this->text);
		
		imageDestroy( $hImg_temp );
		
		$margin = 4 ;

		// fix fy
		$nRight = $this->padding_right!==null ? $this->padding_right : $margin ;
		$nLeft = $this->padding_left!==null ? $this->padding_left : $margin ;
		$x_size = $nLeft + ($p4[2]-$p4[0]) + $nRight; // nb. bug in size x.. have to add a margin..
		
		$padding_top=$this->padding_top!==null ? $this->padding_top : $margin;
		$padding_bottom=$this->padding_bottom!==null ? $this->padding_bottom : $margin;
		
		$y_size = $pbox[1]-$pbox[7] + $padding_top +$padding_bottom ;
		$this->hTxt = imageCreate( $x_size, $y_size );
		//$hTxt = imageCreate( 600, 300 );
		// $bg_color = imageColorAllocate( $this->hTxt, 200, 200, 200 );
		$bg_color = $this->getcolor($this->hTxt,$this->background_color);
		imageFill( $this->hTxt, 0, 0, $bg_color );
		imageColorTransparent( $this->hTxt, $bg_color );
//		$x = 0+floor($margin/2) ;
		$x = $nLeft ;
		$y = $y_size-$padding_bottom;
		$txt_color = $this->txtcolor( $this->hTxt );
		$p4_=imageTTFText( $this->hTxt, $this->font_size, 0, $x, $y, $txt_color, $this->ttf_font, $this->text);
		//$txt_shadow = imageColorAllocate( $hTxt, 100, 100, 100 );
		//imageTTFText( $hTxt, $this->font_size, 0, $x-1, $y-1, $txt_shadow, $this->ttf_font, $this->text );
		//return $hTxt ;
	}
	
	function	txtcolor( $hImg_temp )
	{
		$hexcolor = ( "#" == substr( $this->color, 0, 1 )  ) ? substr( $this->color, 1, strlen($this->color)-1 ) . "000000" : $this->color . "000000" ;
		return imageColorAllocate( $hImg_temp, hexdec(substr($hexcolor,0,2))  ,  hexdec(substr($hexcolor,2,2)),  hexdec(substr($hexcolor,4,2)) );
	}

	function	getcolor($hImg_temp,$color)
	{
		$hexcolor = ( "#" == substr( $color, 0, 1 )  ) ? substr( $color, 1, strlen($color)-1 ) . "000000" : $color . "000000" ;
		return imageColorAllocate( $hImg_temp, hexdec(substr($hexcolor,0,2))  ,  hexdec(substr($hexcolor,2,2)),  hexdec(substr($hexcolor,4,2)) );
	}
	
	function getExactColor($im,$color) {
		$hexcolor = ( "#" == substr( $color, 0, 1 )  ) ? substr( $color, 1, strlen($color)-1 ) . "000000" : $color . "000000" ;
		return imageColorExact($im,hexdec(substr($hexcolor,0,2))  ,  hexdec(substr($hexcolor,2,2)),  hexdec(substr($hexcolor,4,2)));	}

}

function	ImageOut( $im = "" )
{
	 if (function_exists("imagejpeg")) {
	    $im ? ImageJPEG($im, "", 100) : Header("Content-type: image/jpeg");
	  }
	  elseif (function_exists("imagepng")) {
	    $im ? ImagePNG($im) : Header("Content-type: image/png");
	  }
	  elseif (function_exists("imagegif")) {
	    $im ? ImageGIF($im) :  Header("Content-type: image/gif");
	  }
	  elseif (function_exists("imagewbmp")) {
	    $im ? ImageWBMP($im) : Header("Content-type: image/vnd.wap.wbmp");
	  }
	  else
	    die("No image support in this PHP server");
}
		
?>
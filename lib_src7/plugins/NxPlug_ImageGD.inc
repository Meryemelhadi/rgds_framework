<?php
/*
COPYRIGHT NxFrontier Ltd
www.nxfrontier.com
This code may not be copied, modified or redistributed without the written consent of NxFrontier Limited, UK company number 04462961. 
*/
//
//    Usage :
//    $img    = new Image_Transform_GD();
//    $angle  = -78;
//    $img->load('magick.png');
//
//    if($img->rotate($angle,array('autoresize'=>true,'color_mask'=>array(255,0,0)))){
//        $img->addText(array('text'=>"Rotation $angle",'x'=>0,'y'=>100,'font'=>'/usr/share/fonts/default/TrueType/cogb____.ttf'));
//        $img->display();
//    } else {
//        echo "Error";
//    }
//
//
// $Id: GD.php,v 1.10 2002/09/08 03:07:51 busterb Exp $
//
// Image Transformation interface using the GD library
//


class Image_Transform_Driver_GD
{
	/**
	 * Holds the image file for manipulation
	 */
    var $imageHandle = '';

	/**
	 * Holds the original image file
	 */
    var $old_image = '';

    /**
     * Check settings
     *
     * @return mixed true or  or a PEAR error object on error
     *
     * @see PEAR::isError()
     */
    function Image_Transform_Driver_GD(&$errorCB)
    {
    	$this->errorCB = & $errorCB;
    	$this->resized = false;
    	
        if (!function_exists("ImageTypes"))
        	$this->errorCB->addError("libgd not compiled into PHP");

        if (!ImageTypes())
        	$errorCB->addError("GD: No supported image types available");
        return;
    } // End function Image

    
    function getImageSize($imgPath)
    {
    	// array($x, $y, $type)
    	return getimagesize( $imgPath );
    }
    /**
     *
     * @access public
     * @return void
     */
    function _get_image_details($image)
    {
    	$data = @GetImageSize($image);
        #1 = GIF, 2 = JPG, 3 = PNG, 4 = SWF, 5 = PSD, 6 = BMP, 7 = TIFF(intel byte order), 8 = TIFF(motorola byte order,
        # 9 = JPC, 10 = JP2, 11 = JPX, 12 = JB2, 13 = SWC
        if (is_array($data)){
            switch($data[2]){
                case 1:
                    $this->type='gif';
                    break;
                case 2:
                    $this->type='jpeg';
                    break;
                case 3:
                    $this->type='png';
                    break;
                case 4:
                    $this->type='swf';
                    break;
                case 5:
                    $this->type='psd';
                case 6:
                    $this->type='bmp';
                case 7:
                case 8:
                    $this->type='tiff';
                default:
                	$this->type=null;
                	$this->errorCB->addError("We do not recognize this image format");
                    return false;
            }
            $this->img_x = $data[0];
            $this->img_y = $data[1];

            return true;
        } else {
            // return PEAR::raiseError("Cannot fetch image or images details.", true);
           	$this->errorCB->addError("Cannot fetch image or images details.");
            return false;
        }
    }

    /**
     * Check if image type is supported.
     * @return type same type if its supported, an alternate type (jpeg) if it can be read but not written,
     * or null if not supported at all.
     */
    function isSupportedImageType($type)
    {
        if (function_exists('ImageCreateFrom' . $type))
	    	return $type;
	    else if (function_exists('ImageCreateFromPNG'))
	    	return 'png';
	    else if (function_exists('ImageCreateFromJPEG'))
	    	return 'jpeg';
	    else
	    	return null;
    }
    
    /**
     * Load image
     *
     * @param string filename
     *
     */
    function load($image)
    {
        $this->image = $image;
        if (!$this->_get_image_details($image))
        {
        	nxError('cannot load image','NXPLUG_GD');
        	return false;
        }
        
        $functionName = 'ImageCreateFrom' . $this->type;
        if (function_exists($functionName))
        {
			if (!($this->imageHandle = $functionName($this->image)))
			{
				nxError('cannot load image '.$this->image,'NXPLUG_GD');
				return false;
			}
	        $this->img_x = imagesx($this->imageHandle);
	        $this->img_y = imagesy($this->imageHandle);
	        return true;
        }
        else {
        	nxError('Unsupported function:'.$functionName);
        	return false;
        }
    } // End load

   /**
    * Resize Action
    *
    * For GD 2.01+ the new copyresampled function is used
    * It uses a bicubic interpolation algorithm to get far
    * better result.
    *
    * @param $new_x int  new width
    * @param $new_y int  new height
    *
    * @return true on success or pear error
    * @see PEAR::isError()
    */
    function _resize($new_x, $new_y) {
        if ($this->resized === true) {
        	$this->errorCB->addError('You have already resized the image without saving it.  Your previous resizing will be overwritten');
        	return false;
        }
       
       if ($new_x > $this->img_x && $new_y > $this->img_y)
       {
			$new_x=$this->img_x;
			$new_y= $this->img_y;
	   }
       
//        if($this->type!='gif' && function_exists('ImageCreateTrueColor')){
        if(NX_USE_GD2 && function_exists('ImageCreateTrueColor')){
            $new_img =ImageCreateTrueColor($new_x,$new_y);
            /*
            $white = ImageColorAllocate($myImage, 255, 255, 255);
            ImageFill($myImage, 0, 0, $white);
            */
            if($this->type=='png')
			{
				imagealphablending($new_img,FALSE);
				imagesavealpha($new_img,TRUE);
			}

            ImagePaletteCopy ($new_img,$this->imageHandle);
            $white = ImageColorAllocate($this->imageHandle, 255, 255, 255);

			// modif FY : 7/10/2010	
            if($this->type!='gif')
           		;//ImageFill($this->imageHandle, 0, 0, $white);
        } else {
            $new_img =ImageCreate($new_x,$new_y);
            ImagePaletteCopy ($new_img,$this->imageHandle);
        }

        // manage transparent GIFs (NB. looks like transparent color becomes black...)
        if ($this->type=='gif' && ($transp=ImageColorTransparent($this->imageHandle))>=0)
        {
        	$black = ImageColorAllocate ($this->imageHandle, 0, 0, 0);

        	// remove transparent
			ImageColorTransparent($this->imageHandle, $black);        
        }
        
//        if($this->type!='gif' && function_exists('ImageCopyResampled')){
        if(NX_USE_GD2 && function_exists('ImageCopyResampled')){
            ImageCopyResampled($new_img, $this->imageHandle, 0, 0, 0, 0, $new_x, $new_y, $this->img_x, $this->img_y);
        } else {
        	$this->ImageCopyResampleBicubic($new_img, $this->imageHandle, 0, 0, 0, 0, $new_x, $new_y, $this->img_x, $this->img_y);
            // ImageCopyResized($new_img, $this->imageHandle, 0, 0, 0, 0, $new_x, $new_y, $this->img_x, $this->img_y);
        }
       
        $this->old_image = $this->imageHandle;
        $this->imageHandle = $new_img;
        $this->resized = true;

        $this->new_x = $new_x;
        $this->new_y = $new_y;
        return true;
    }


	/*
	port to PHP by John Jensen July 10 2001 (updated 4/21/02) -- original code (in C, for the PHP GD Module) by jernberg@fairytale.se
	*/
	function ImageCopyResampleBicubic (&$dst_img, &$src_img, $dst_x, $dst_y, $src_x, $src_y, $dst_w, $dst_h, $src_w, $src_h)
	{
		$palsize = ImageColorsTotal ($src_img);
		for ($i = 0; $i < $palsize; $i++) 
		{  // get palette.
			$colors = ImageColorsForIndex ($src_img, $i);
			ImageColorAllocate ($dst_img, $colors['red'], $colors['green'], $colors['blue']);
		}
		
		$scaleX = ($src_w - 1) / $dst_w;
		$scaleY = ($src_h - 1) / $dst_h;
		$scaleX2 = (int) ($scaleX / 2);
		$scaleY2 = (int) ($scaleY / 2);
		for ($j = $src_y; $j < $dst_h; $j++) 
		{
			$sY = (int) ($j * $scaleY);
			$y13 = $sY + $scaleY2;
			
			for ($i = $src_x; $i < $dst_w; $i++) 
			{
				$sX = (int) ($i * $scaleX);
				$x34 = $sX + $scaleX2;
				$color1 = ImageColorsForIndex ($src_img, ImageColorAt ($src_img, $sX, $y13));
				$color2 = ImageColorsForIndex ($src_img, ImageColorAt ($src_img, $sX, $sY));
				$color3 = ImageColorsForIndex ($src_img, ImageColorAt ($src_img, $x34, $y13));
				$color4 = ImageColorsForIndex ($src_img, ImageColorAt ($src_img, $x34, $sY));
				$red = ($color1['red'] + $color2['red'] + $color3['red'] + $color4['red']) / 4;
				$green = ($color1['green'] + $color2['green'] + $color3['green'] + $color4['green']) / 4;
				$blue = ($color1['blue'] + $color2['blue'] + $color3['blue'] + $color4['blue']) / 4;
				ImageSetPixel ($dst_img, $i + $dst_x - $src_x, $j + $dst_y - $src_y, ImageColorClosest ($dst_img, $red, $green, $blue)); 
			}
		}
	}
	

    /**
     * Save the image file
     *
     * @param $filename string  the name of the file to write to
     * @param $quality  int     output DPI, default is 75
     * @param $types    string  define the output format, default
     *                          is the current used format
     *
     * @return none
     */
    function save($filename, $type = '', $quality = 75)
    {
        $type           = $type==''? $this->type : $type;
        $functionName   = 'Image' . $type;

        if (!function_exists($functionName) || $type=='jpeg')
        { 
	        ImageJPEG($this->imageHandle, $filename,$quality) ;
        }
		else
		   $functionName($this->imageHandle, $filename) ;
		   
        $this->imageHandle = $this->old_image;
        $this->resized = false;
    } // End save

	function getBoxedSizes($dst_w, $dst_h )
	{
		//src_w ,src_h-- start width and height
		//dst_w ,dst_h-- end width and height
		//return array  w=>new width h=>new height mlt => multiplier
		//the function tries to shrink or enalrge src_w,h in such a way to best fit them into dst_w,h
		//keeping x to y ratio unchanged
		//dst_w or/and dst_h can be "*" in this means that we dont care about that dimension
		//for example if dst_w="*" then we will try to resize by height not caring about width 
		//(but resizing width in such a way to keep the xy ratio)
		//if both = "*" we dont resize at all.
		#### Calculate multipliers
		$src_w = $this->img_x;
		$src_h = $this->img_y;
		
		$mlt_w = $dst_w / $src_w;
		$mlt_h = $dst_h / $src_h;
	
		$mlt = $mlt_w < $mlt_h ? $mlt_w:$mlt_h;
		if($dst_w == "*") $mlt = $mlt_h;
		if($dst_h == "*") $mlt = $mlt_w;
		if($dst_w == "*" && $dst_h == "*") $mlt=1;
	
		#### Calculate new dimensions
		$img_new_w =  round($src_w * $mlt);
		$img_new_h =  round($src_h * $mlt);
		return array("x" => $img_new_w, "y" => $img_new_h, "mlt_x"=>$mlt, "mlt_y"=>$mlt,  "mlt"=>$mlt);
	}

	/**
     * Resize the Image in the X and/or Y direction
     * If either is 0 it will be scaled proportionally
     *
     * @access public
     *
     * @param mixed $new_x (0, number, percentage 10% or 0.1)
     * @param mixed $new_y (0, number, percentage 10% or 0.1)
     *
     * @return mixed none or PEAR_error
     */
    function resize($new_x = 0, $new_y = 0, $stretch=false)
    {
    	// get box size (fit/resize proportionnaly the image to fit in the box)
        $res = $this->getBoxedSizes($new_x, $new_y);
        
        // Now do the library specific resizing.
        return $this->_resize($res["x"], $res["y"], $stretch);
    } // End resize

    /**
     * Display image without saving and lose changes
     *
     * @param string type (JPG,PNG...);
     * @param int quality 75
     *
     * @return none
     */
    function display($type = '', $quality = 75)
    {
        if ($type != '') {
            $this->type = $type;
        }
        $functionName = 'Image' . $this->type;
        header('Content-type: image/' . strtolower($this->type));
        $functionName($this->imageHandle, '', $quality);
        $this->imageHandle = $this->old_image;
        $this->resized = false;
        ImageDestroy($this->old_image);
        $this->free();
    }

    /**
     * Destroy image handle
     *
     * @return none
     */
    function free()
    {
        if ($this->imageHandle){
            ImageDestroy($this->imageHandle);
        }
    }

} // End class ImageGD

class NxPlug_ImageGD
{
	function __construct(&$props)
	{
		$this->props = & $props;
	}

	function getSizes($src_w, $src_h, $dst_w,$dst_h )
	{
		//src_w ,src_h-- start width and height
		//dst_w ,dst_h-- end width and height
		//return array  w=>new width h=>new height mlt => multiplier
		//the function tries to shrink or enalrge src_w,h in such a way to best fit them into dst_w,h
		//keeping x to y ratio unchanged
		//dst_w or/and dst_h can be "*" in this means that we dont care about that dimension
		//for example if dst_w="*" then we will try to resize by height not caring about width 
		//(but resizing width in such a way to keep the xy ratio)
		//if both = "*" we dont resize at all.
		#### Calculate multipliers
		$mlt_w = $dst_w / $src_w;
		$mlt_h = $dst_h / $src_h;
	
		$mlt = $mlt_w < $mlt_h ? $mlt_w:$mlt_h;
		if($dst_w == "0") $mlt = $mlt_h;
		if($dst_h == "0") $mlt = $mlt_w;
		if($dst_w == "0" && $dst_h == "0") $mlt=1;
	
		#### Calculate new dimensions
		$img_new_w =  round($src_w * $mlt);
		$img_new_h =  round($src_h * $mlt);
		return array("w" => $img_new_w, "h" => $img_new_h, "mlt_w"=>$mlt, "mlt_h"=>$mlt,  "mlt"=>$mlt);
	}

	function getImageSize($imgPath,&$errorCB)
	{
		$im = new Image_Transform_Driver_GD($errorCB);
		if ($errorCB->isError())
			return null;
			
		$sz = $im->getImageSize($imgPath);
		return array("w"=>$sz[0], "h"=>$sz[1]);
	}

	function resizeImage($imgPath,$newImgPath,$new_x, $new_y,&$errorCB)
	{
		$im = new Image_Transform_Driver_GD($errorCB);
		if ($errorCB->isError())
			return null;

		nxLog("load image $imgPath",'GD');				
		if (!$im->load($imgPath))
			return null;
		
		// next will resize so that the largest length is 300px - height or width
		nxLog("resize to $new_x x $new_y",'GD');				
		$im->resize($new_x, $new_y,false);

		//$im->display();
		nxLog("save to $newImgPath",'GD');				
		$im->save($newImgPath,$im->type,80);
		$dim_wh = array("w"=>$im->new_x, "h"=>$im->new_y);
		
		// Now free the memory - should be called free?
		$im->free();
		
		nxLog("image resized!",'GD');				
		return $dim_wh;
	}
}

?>

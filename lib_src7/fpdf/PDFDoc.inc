<?php

// NX extension of FPDF and FPFI, allow to use a template + descriptor
/*
		$pdf = new PDFDoc($orientation='P', $unit='mm', $format='A4');
		$self = $this;
		$pdf->loadDoc($path,$configDoc,
			function($content) use ($self,$mapper,$sessionInfos,$infos) {
				return $self->processInviteBody($mapper,$sessionInfos,$infos,$content,$i=0,$n=10);
			});

		$pdf->Output("I",$filename.".pdf");
*/

require(NX_LIB.'fpdf/fpdf.php');
require(NX_LIB.'fpdi/fpdi.php');

//function hex2dec
//returns an associative array (keys: R,G,B) from
//a hex html code (e.g. #3FE5AA)
function hex2dec($couleur = "#000000"){
    $R = substr($couleur, 1, 2);
    $rouge = hexdec($R);
    $V = substr($couleur, 3, 2);
    $vert = hexdec($V);
    $B = substr($couleur, 5, 2);
    $bleu = hexdec($B);
    $tbl_couleur = array();
    $tbl_couleur['R']=$rouge;
    $tbl_couleur['V']=$vert;
    $tbl_couleur['B']=$bleu;
    return $tbl_couleur;
}

class UnitsConv {
	static $dpi=72;

	static function setDPI($dpi) {
		self::$dpi=$dpi;
	}

	//conversion pixel -> millimeter at 72 dpi (with screenshots, adjust image DPI before mesuring in pixels..)
	static function px2mm($px){
		return $px*25.4/self::$dpi;
		//return $px*0.266497462;
	}

	static function mm2px($px){
		return $px*self::$dpi/25.4;
		// return $px/0.266497462;
	}

	//points to mm
	static function pt2mm($pt){
		return 0.352778*$pt;
	}

	static function mm2pt($pt){
		return $pt/0.352778;
	}

	static function px2pt($px){
		return self::mm2pt(self::px2mm($px));
	}
}

function txtentities($html){
    $trans = get_html_translation_table(HTML_ENTITIES);
    $trans = array_flip($trans);
    $res= strtr($html, $trans);
	$res=str_replace('&#38;#039;',"'",$res);
	return $res;
}

// Write order kept after page break in multi column layout
class Capture_Cell {
	function __construct($data) { $this->data = $data; }
	function play($pdf) {
		list($x,$w,$h,$txt,$border,$ln,$align,$fill,$link)=$this->data;
		$pdf->setX($x);
		$pdf->Cell($w,$h,$txt,$border,$ln,$align,$fill,$link);
	}
};
class Capture_Fill {
	function __construct($col) { $this->FillColor = $col; }
	function play($pdf) {
		$pdf->_out($this->FillColor);
	}
};

class Capture_Col {
	function __construct($data) { $this->data = $data; }
	function play($pdf) {
		list($pdf->TextColor,$pdf->ColorFlag)=$this->data;
	}
};

class Capture_action {
	function __construct($action,$data) { $this->action = $action; $this->data = $data; }
	function play($pdf) {
		$pdf->_replayAction($this->action,$this->data);
	}
};


class PDF_HTML extends FPDI
{
	//variables of html parser
	protected $B;
	protected $I;
	protected $U;
	protected $HREF;
	protected $fontList;
	protected $issetfont;
	protected $issetcolor;
	var $prevWrite='';
	var $debug = false;
	var $blockLeftX;

	var $defaultStyle = array('font'=>'arial','size'=>'8','style'=>'','color'=>'#000000','align'=>'justify','background-color'=>'',
		'border'=>'0','border-color'=>'#000000','border-size'=>'0.2mm');

	var $tags = array(
		'h1'=>array('size'=>'20','style'=>'B','color'=>'#550000'),
		'h2'=>array('size'=>'18','style'=>'B','color'=>'#000000'),
		'h3'=>array('size'=>'16','style'=>'B','color'=>'#000000'),
		'h4'=>array('size'=>'12','style'=>'BI','color'=>'#000000'),
		'h5'=>array('size'=>'9','style'=>'I','color'=>'#000000'),
		'p'=> array('size'=>'5','style'=>'','color'=>'#000000'),
		'html'=> array('font'=>'arial','size'=>'5','style'=>'','color'=>'#000000','align'=>'justify'),
		'pagebreakwhen'=> array('maxY'=>'28cm'),
	);

	var $stylePath=array();
	var $rules = array();

	var $mapAlign = array(
		'left'=>'L',
		'right'=>'R',
		'center'=>'C',
		'justify'=>'J'		
	);

	function __construct($orientation='P', $unit='mm', $format='A4')
	{
		//Call parent constructor
		parent::__construct($orientation,$unit,$format);

		//Initialization
		$this->B=0;
		$this->I=0;
		$this->U=0;
		$this->HREF='';
		$this->fontlist=array('arial', 'times', 'courier', 'helvetica', 'symbol');
		$this->issetfont=false;
		$this->issetcolor=false;
		$this->blockLeftX = $this->lMargin;

		$this->pushStyle($this->defaultStyle);
	}

	function initTags($tags=null) {
		if($tags)
			$tags = array_merge($this->tags,$tags);
		else
			$tags = $this->tags;

		$this->tags = $this->initTagsStyles($tags);
	}

	function initCSSRules($rules=null) {
		if($rules)
			$this->rules =  array_merge($this->rules,$rules);
	}

	function initTagsStyles($tags) {

		foreach($tags as $t=>$desc)
		{
			$desc['tag']=$t;
			$tags2[strtolower($t)]=$desc;
		}

		$allowedTags = explode(',','b,u,i,a,img,p,br,strong,em,font,tr,blockquote,ul,ol,li,pagebreakwhen');
		$allowedTags = array_unique(array_merge(array_keys($tags2),$allowedTags));	
		$this->allowedTags = '<'.implode('><',$allowedTags).'>';

		return $tags2;
	}

	function getLeft() {
		if($this->blockLeftX)
			$leftX = $this->blockLeftX;
		else
			$leftX = $this->lMargin;

		return $leftX;
	}
	
	function setLeft($x) {
		$this->blockLeftX = $x;
	}
	function resetLeft() {
		$this->blockLeftX = $this->lMargin;
	}

	function isStartOfLine() {
		if($this->x == $this->getLeft())
			return true;
		else
			return false;
	}

	function offsetXY($x,$y) {
		if($x)
			$this->x += $x;

		if($y)
			$this->y += $y;
	}

	// MULTI COLUMN MODE
	//
	// manage end of page (capture cells etc. and replay after page)
	// used with 2 columns, when first goes after end of page.
	
	var $column=-1;
	function setColumn($col) {
		$this->column=$col;
	}
	
	// called when Y overflow
	function AcceptPageBreak()
	{
		if($this->column>=0)
		{
			if(!$this->capture)
				; // nxError("CAPTURE");

			$this->capture = true;
			return false;
		}
		else
			return true;
	}

	function onAfterPage() {
		if(!$this->capture)
			return;

		$this->capture = false;
		$this->column = -1;

		if(count($this->cellsNextPage)==0)
			return;

		// have to display cells
		$this->addPage();

		// play context : fill, text
		foreach($this->captureContext as $action)
		{
			$action->play($this);
		}

		// play actions
		foreach($this->cellsNextPage as $action)
		{
			$action->play($this);
		}

		$this->captureContext = array();
		$this->cellsNextPage = array();
	}

	function Cell($w, $h=0, $txt='', $border=0, $ln=0, $align='', $fill=false, $link='')
	{
		// Output a cell
		if($this->y+$h>$this->PageBreakTrigger && !$this->InHeader && !$this->InFooter)
		{
			if(!$this->AcceptPageBreak())
			{
				$this->cellsNextPage[] = new Capture_Cell(array($this->getX(),$w,$h,$txt,$border,$ln,$align,$fill,$link));
				return;
			}
		}

		call_user_func_array("parent::Cell",func_get_args());
	}

	function SetFillColor() 
	{
		$this->_execAction("parent::SetFillColor",func_get_args());
	}

	function SetTextColor() {
		$this->_execAction("parent::SetTextColor",func_get_args());
	}

	function SetFont($family, $style='', $size=0)
	{
		//nxError("SetFont($family,$style,$size)");
		$this->_execAction("parent::SetFont",func_get_args());
	}

	function SetStyle($style='')
	{
		// nxError("SetStyle($style)");
		$this->_execAction('self::SetFont',array('',$style),'self::SetStyle');
	}

	function Ln($h=null)
	{
		call_user_func_array($o="parent::Ln",$a=func_get_args());
		if($this->capture)
			$this->cellsNextPage[] = new Capture_Action($o,$a);
	}

	function _execAction($o,$a,$id=null) {
		call_user_func_array($o,$a);
		if($this->column>=0)
		{
			$action = new Capture_Action($o,$a);
			if(!$id)
				$id=$o;
			if($this->capture)
			{
				$this->cellsNextPage[] = $action;
				//nxError('capture op :'.$o);
			}
			else
			{
				$this->captureContext[$id]=$action;
				//nxError('capture context :'.$o);
			}
		}
	}
	function _replayAction($o,$a) {
		call_user_func_array($o,$a);
	}
	function keepCell($w,$h,$txt,$border,$ln,$align,$fill,$link) {
		$this->cellsNextPage[] = new Capture_Cell(array($this->getX(),$w,$h,$txt,$border,$ln,$align,$fill,$link));
	}
	
	// / manage end of page


	function Write($h, $txt, $link='',$lineHeight=null,$border=false,$bgCol=null,$w=null)
	{
		if(!$lineHeight)
			$lineHeight = $h;

		if($this->debug)
			$border=1;

		// left base for newline
		$leftX = $this->getLeft();

		// bg color
		if($bgCol===true) {
			$bgFill=true;
		}
		elseif($bgCol===false) {
			$bgFill=false;
		}
		elseif($bgCol) {
			$bgFill=true;
			$this->SetFillColorHex($bgCol);
		}

		// Output text in flowing mode
		if(!isset($this->CurrentFont))
			$this->Error('No font has been set');
		$cw = &$this->CurrentFont['cw'];

		if($w===null)
		$w = $this->w-$this->rMargin-$this->x;
		$wmax = ($w-2*$this->cMargin)*1000/$this->FontSize;
		$s = str_replace("\r",'',$txt);
		$nb = strlen($s);
		$sep = -1;
		$i = 0;
		$j = 0;
		$l = 0;
		$nl = 1;
		while($i<$nb)
		{
			// Get next character
			$c = $s[$i];
			if($c=="\n")
			{
				// Explicit line break
				$this->Cell($w,$h,substr($s,$j,$i-$j),$border,2,'',$bgFill,$link);
				$i++;
				$sep = -1;
				$j = $i;
				$l = 0;
				if($nl==1)
				{
					$this->x = $leftX;
					$w = $this->w-$this->rMargin-$this->x;
					$wmax = ($w-2*$this->cMargin)*1000/$this->FontSize;
				}
				$nl++;
				continue;
			}
			if($c==' ')
				$sep = $i;
			$l += $cw[$c];
			if($l>$wmax)
			{
				// Automatic line break
				if($sep==-1)
				{
					if($this->x>$leftX)
					{
						// Move to next line
						$this->x = $leftX;
						$this->y += $h;
						nxLog("auto br +$h",'FPDF');

						$w = $this->w-$this->rMargin-$this->x;
						$wmax = ($w-2*$this->cMargin)*1000/$this->FontSize;
						$i++;
						$nl++;
						continue;
					}
					if($i==$j)
						$i++;
					$this->Cell($w,$h,substr($s,$j,$i-$j),$border,2,'',$bgFill,$link);
				}
				else
				{
					$this->Cell($w,$h,substr($s,$j,$sep-$j),$border,2,'',$bgFill,$link);
					$i = $sep+1;
				}
				$sep = -1;
				$j = $i;
				$l = 0;
				if($nl==1)
				{
					$this->x = $leftX;
					$w = $this->w-$this->rMargin-$this->x;
					$wmax = ($w-2*$this->cMargin)*1000/$this->FontSize;
				}
				$nl++;
			}
			else
				$i++;
		}
		// Last chunk
		if($i!=$j)
			$this->Cell($l/1000*$this->FontSize,$h,substr($s,$j),$border,0,'',$bgFill,$link);
	}

	function writeText($s) {

		$s2 = stripslashes(txtentities($s));
		if(!trim($s2))
			return;

		$tag = $this->curStyle['tag'];

		if($this->debug ||(isset($this->curStyle['debug'])&&$this->curStyle['debug']))
			$debug=true;
		else
			$debug=false;

		if($this->debug ||(isset($this->curStyle['break'])&&$this->curStyle['break']))
			DebugBreak();

		if(isset($this->curStyle['beforeWrite']))
			$s2 = $this->curStyle['beforeWrite']($s2,$this->curStyle,$this);

		if(isset($this->curStyle['size']))
			$size = $this->curStyle['size'];
		else
			$size = 16;

		if(isset($this->curStyle['background-color']))
			$bgCol = $this->curStyle['background-color'];
		else
			$bgCol = null;

		if($bgCol) {
			$this->SetFillColorHex($bgCol);
			$bgfill = true;
		}
		else
			$bgfill = false;

		if(isset($this->curStyle['margin-bottom']))
			$mgBottom = $this->curStyle['margin-bottom'];
		else
			$mgBottom = 0;

		if(isset($this->curStyle['margin-top']))
			$mgTop = $this->curStyle['margin-top'];
		else
			$mgTop = 0;

		if(isset($this->curStyle['height']))
			$h = $this->curStyle['height'];
		else
			$h = UnitsConv::pt2mm($size);

		if(isset($this->curStyle['width']))
			$width = $this->curStyle['width'];
		else
			$width = null;

		if(isset($this->curStyle['border']) && ($border=$this->curStyle['border']))
		{
			$border = 1;
		}
		else
			$border = 0;

		if($this->textPrefix)
			$s2 = $this->textPrefix.$s2;

		if(isset($this->curStyle['left']))
			$this->setX($this->getLeft() + $this->any2mm($this->curStyle['left']));

		if($mgTop)
			$this->offsetXY(0, $this->any2mm($mgTop));

		$align = $this->curStyle['align'] ? strtoupper($this->curStyle['align'][0]) : '';

		if(/*$border ||*/ $width || ($align && $align!='J' && $align!='L'))
		{
			nxLog($log="[$tag]Write MultiCell text align:{$align},height:{$h}mm, size:{$size}pt,width:{$width}, text:{$s2},bg:{$bgCol}",'FPDF');

			if($debug)
				$s2 = $log;

			if(isset($this->curStyle['left']))
				$this->setX($this->getLeft() + $this->any2mm($this->curStyle['left']));
			else
				$this->setX($this->getLeft());

			$this->MultiCell($width,$h,$s2,$border, $align,$bgfill);
			$this->prevWrite='block';
			nxLog($log="PREV WRITE IS BLOCK",'FPDF');
		}
		else {
			nxLog($log="[$tag] Write height:{$h}mm, size:{$size}pt, width:{$width}, text:{$s2}",'FPDF');
			if($debug)
				$s2 = $log;

			$this->Write($h,$s2,$link='',null,$border,$bgCol,$width);
			$this->prevWrite='inline-block';
			nxLog($log="PREV WRITE IS INLINE BLOCK",'FPDF');
		}

		if($mgBottom)
			$this->offsetXY(0, $this->any2mm($mgBottom));
	}

	function writeHTML($html,$curStyle)
	{
		// set new style
		$this->pushStyle($curStyle);
		$this->prevWrite='';

		//HTML parser
		$html=strip_tags($html,$this->allowedTags); //supprime tous les tags sauf ceux reconnus
		$html=str_replace(array('\\n',"\n",'&#039;'),array(' ',' ',"'"),$html); //remplace retour à la ligne par un espace
		$a=preg_split('/<(.*)>/U',$html,-1,PREG_SPLIT_DELIM_CAPTURE); //éclate la chaîne avec les balises
		foreach($a as $i=>$e)
		{
			if($i%2==0)
			{
				//Text
				if($this->HREF)
					$this->PutLink($this->HREF,$e);
				else
					$this->writeText($e);
			}
			else
			{
				//Tag
				if($e[0]=='/')
					$this->CloseTag(strtoupper(substr($e,1)));
				else
				{
					//Extract attributes
					$a2=explode(' ',$e);
					$tag=strtoupper(array_shift($a2));
					$attr=array();
					foreach($a2 as $v)
					{
						if(preg_match('/([^=]*)=["\']?([^"\']*)/',$v,$a3))
							$attr[strtoupper($a3[1])]=$a3[2];
					}
					$this->OpenTag($tag,$attr);
				}
			}
		}

		// restore previous style
		$this->pullStyle();
	}

	function br() {

		if($this->prevWrite!='block')
		{
			nxLog("br : LN(5)",'FPDF');
			$this->Ln(5);
		}
		else
		{
			nxLog("br : PREV IS BLOCK => no LN()",'FPDF');
			$this->prevWrite='';
		}

		if($this->inBlock())
			; // $this->prevWrite='block';
		else
			$this->prevWrite='';
	}

	function nl() {
		if($this->prevWrite!='block')
		{
			nxLog("nl(5)",'FPDF');
			$this->Ln(5);
		}
		else
		{
			nxLog("nl : PREV IS BLOCK => no LN()",'FPDF');
			$this->prevWrite='';
		}

		if($this->inBlock())
			$this->prevWrite='block';
		else
			$this->prevWrite='';
	}

	function enterUL() {
		if(!$this->isStartOfLine())
			$this->nl();
	}
	function enterOL() {
		if(!$this->isStartOfLine())
			$this->nl();
	}
	function closeUL() {
	}
	function closeOL() {
	}
	function li() {
		$this->textPrefix = '- ';
	}
	function closeLi() {
		$this->br();
		$this->textPrefix = '';
	}

	function pageBreakWhen() {
		if(isset($this->curStyle['maxY']))
		{
			$maxY = $this->any2mm($this->curStyle['maxY']);
			if($this->y > $maxY)
				$this->AddPage();
		}
		else
			$this->AddPage();
	}

	function OpenTag($tag, $attr)
	{
		nxLog("OPEN <{$tag}>",'FPDF');

		//Opening tag
		switch($tag){
			case 'STRONG':
				$this->SetTextDecoration('B',true);
				break;
			case 'EM':
				$this->SetTextDecoration('I',true);
				break;
			case 'B':
			case 'I':
			case 'U':
				$this->SetTextDecoration($tag,true);
				break;
			case 'A':
				$this->HREF=$attr['HREF'];
				break;
			case 'UL':
				$this->enterUL();
				break;
			case 'OL':
				$this->enterOL();
				break;
			case 'LI':
				$this->li();
				break;		
			case 'IMG':
				if(isset($attr['SRC']) && (isset($attr['WIDTH']) || isset($attr['HEIGHT']))) {
					if(!isset($attr['WIDTH']))
						$attr['WIDTH'] = 0;
					if(!isset($attr['HEIGHT']))
						$attr['HEIGHT'] = 0;

					if($this->dirPath)
						$attr['SRC'] = preg_replace("%^./%",$this->dirPath.'/',$attr['SRC']);

					$this->Image($attr['SRC'], $this->GetX(), $this->GetY(), $this->any2mm($attr['WIDTH']), $this->any2mm($attr['HEIGHT']));
				}
				break;
			case 'TR':
			case 'BLOCKQUOTE':
				$this->br();
				break;
			case 'BR':
			case 'BR/':
				$this->br();
				break;
			case 'P':
				$this->nl();
				$this->pushTagStyle($tag);
				break;
			case 'FONT':
				if (isset($attr['COLOR']) && $attr['COLOR']!='') {
					$this->SetTextColorHex($attr['COLOR']);
					$this->issetcolor=true;
				}
				if (isset($attr['FACE']) && in_array(strtolower($attr['FACE']), $this->fontlist)) {
					$this->SetFont(strtolower($attr['FACE']));
					$this->issetfont=true;
				}
				break;
			case 'PAGEBREAKWHEN':
				$this->pushTagStyle($tag);
				$this->pageBreakWhen();
				break;
			case 'H1':
			case 'H2':
			case 'H3':
			case 'H4':
			case 'H5':
			default: // support any tag defined in tag list from config
				$this->pushTagStyle($tag);
				break;
		}
	}

	function CloseTag($tag)
	{
		nxLog("CLOSE </{$tag}>",'FPDF');

		//Closing tag
		if($tag=='STRONG')
			$tag='B';
		elseif($tag=='EM')
			$tag='I';

		if($tag=='B' || $tag=='I' || $tag=='U')
			$this->SetTextDecoration($tag,false);
		elseif($tag=='LI')
			$this->closeLi();
		elseif($tag=='OL')
			$this->closeOL();
		elseif($tag=='UL')
			$this->closeUL();
		elseif($tag=='A')
			$this->HREF='';
		elseif($tag=='PAGEBREAKWHEN')
			$this->pullTagStyle($tag);
		elseif($tag=='FONT'){
			if ($this->issetcolor==true) {
				$this->SetTextColor(0,0,0);
			}
			if ($this->issetfont) {
				$this->SetFont('arial');
				$this->issetfont=false;
			}
		}
		else
		{
			$this->pullTagStyle($tag);
		}
	}

	function SetTextDecoration($tag, $enable)
	{
		//Modify style and select corresponding font
		$this->$tag+=($enable ? 1 : -1);
		$style='';
		foreach(array('B','I','U') as $s)
		{
			if($this->$s>0)
				$style.=$s;
		}
		$this->SetStyle($style);
	}

	function pushTagStyle($tag)
	{
		$tag = strtolower($tag);
		$selectStyle = $this->getRulesStyle($tag);

		if(isset($this->tags[$tag]))
		{
			$this->pushStyle($this->tags[$tag],$selectStyle);
		}

		$this->stylePath[]=$tag;
	}
	function pullTagStyle($tag)
	{
		$tag = strtolower($tag);
		if(isset($this->tags[$tag]))
			$this->pullStyle();

		array_pop($this->stylePath);
	}

	// CSS selectors (ex. div>p)
	function stylePath($tag) {
		$curPath = implode('>',$this->stylePath);
		if($curPath)
				$curPath.='>'.$tag;

		return $curPath;
	}

	function getRulesStyle($tag) {
		$tag = strtolower($tag);
		$curPath = $this->stylePath($tag);

		$res = array();
		$style = array();
		foreach($this->rules as $sel=>$rule) {
			if($this->matchCSSRule($sel,$curPath))
			{
				$res[] = $rule;
				$style = array_merge($style,$rule);
			}
		}

		if(count($res))
			return $style;
		else
			return null;
	}

	function matchCSSRule($selector,$curPath) {
		return $this->endsWith($curPath, $selector);
	}

	function PutLink($URL, $txt)
	{
		//Put a hyperlink
		$this->SetTextColor(0,0,255);
		$this->SetTextDecoration('U',true);
		$this->Write(5,$txt,$URL);
		$this->SetTextDecoration('U',false);
		$this->SetTextColor(0,0,0);
	}

	// style stack management (allow to have <b> in <p>, of <i> in <h1> or <b>)
	var $styleStack = array();
	var $curStyle;

	function _setStyle($style) {
		// get new UBI value
		if(isset($style['style']))
			$curUbi = $this->getCurUBI($style['style']);
		else
			$curUbi = '';
		$style['style'] = $curUbi;

		if(!isset($style['align']))
			$style['align'] = '';

		if(!isset($style['font']))
			$style['font'] = $this->curStyle['font'];

		if(!isset($style['color']))
			$style['color'] = $this->curStyle['color'];

		if(!isset($style['border']))
			$style['border'] = $this->curStyle['border'];

		if(!isset($style['border-color']))
			$style['border-color'] = $this->curStyle['border-color'];
		if(!isset($style['border-size']))
			$style['border-size'] = $this->curStyle['border-size'];

		if(isset($style['line-height']))
			$style['height'] = $style['line-height'];

		if(!isset($style['height']))
			$style['height'] = $this->any2mm($this->curStyle['size'],'pt');
		else
			$style['height'] = $this->any2mm($style['height']);

		if(isset($style['width']))
			$style['width'] = $this->any2mm($style['width']);

		if(isset($style['font-size']))
			$style['size'] = $style['font-size'];

		if(!isset($style['size']))
			$style['size'] = $this->any2pt($this->curStyle['size']);
		else
			$style['size'] = $this->any2pt($style['size']);

		if(!isset($style['align']))
			$style['align'] = $this->curStyle['align'];

		if(!isset($style['background-color']))
			$style['background-color'] = $this->curStyle['background-color'];

		$this->SetFont($style['font'],$curUbi,$style['size']);
		$this->SetTextColorHex($style['color']);

		if($style['border-color'])
		{
			$this->SetDrawColorHex($style['border-color']);
		}
		if($style['border-size'])
		{
			$this->SetLineWidth($this->any2mm($style['border-size']));
		}

		nxLog("Apply font style font:{$style['font']},decoration:{$style['style']},size:{$style['size']},color:{$style['color']}",'FPDF');

		$this->curStyle = $style;
		return $style;
	}

	function getCurStyle() {
		return $this->curStyle;
	}

	function inBlock() {
		return $this->curStyle['align']!='' && $this->curStyle['width']!='' && strtoupper($this->curStyle['align'][0])!='L';
	}

	function getCurUBI($ubi) {
		
		$this->U=$this->B=$this->I=false;

		$l = strlen($ubi);
		for($i=0;$i<$l;$i++)
		{
			$u = $ubi[$i];
			$this->$u = 1;
		}
		$cur = ($this->U?'U':'').($this->B?'B':'').($this->I?'I':'');
		return $cur;
	}

	function pushStyle($style,$rulesStyles=null) {

		// add css selector style if any
		if($rulesStyles)
			$style = array_merge($style,$rulesStyles);

		$newStyle = $this->_setStyle($style);
		$this->styleStack[]=$newStyle;
	}

	function pullStyle() {
		if(count($this->styleStack))
		{
			$curStyle = array_pop($this->styleStack);
			$prevStyle=$this->styleStack[count($this->styleStack)-1];
			$this->_setStyle($prevStyle);
		}
	}

	function SetTextColorHex($hex) {
		$coul=hex2dec($hex);
		$this->SetTextColor($coul['R'],$coul['V'],$coul['B']);
	}

	function SetDrawColorHex($hex) {
		$coul=hex2dec($hex);
		$this->SetDrawColor($coul['R'],$coul['V'],$coul['B']);
	}

	function SetFillColorHex($hex) {
		$coul=hex2dec($hex);
		$this->SetFillColor($coul['R'],$coul['V'],$coul['B']);
	}

	function _setY($y)
	{
		$y2 = $this->any2mm($y);

		if($y2!=$y)
			nxLog("SetY({$y}) -> SetY({$y2})",'FPDF');
		else
			nxLog("SetY({$y})",'FPDF');

		$this->SetY($y2,true);
	}

	function _setXY($x,$y,$isBlock=true)
	{
		$x2 = $this->any2mm($x);
		$y2 = $this->any2mm($y);

		if($y2!=$y)
			nxLog("SetXY({$x},{$y}) -> SetXY({$x2},{$y2})",'FPDF');
		else
			nxLog("SetXY({$x},{$y})",'FPDF');

		$this->SetXY($x2,$y2,true);
	}

	function em2mm($pt){
		return $this->curStyle['size']*$em;
	}

	function any2mm($x,$unit='mm') {

		if(is_numeric($x))
		{
			if($unit)
				$x=''.$x.$unit;
			else
				return $x;
		}

		if(strpos($x,'mm')>-1)
		{
			$x2 = str_replace('mm','',$x);
			return $x2;
		}

		if(strpos($x,'cm')>-1)
		{
			$x2 = str_replace('cm','',$x);
			return $x2*10;
		}

		if(strpos($x,'px')>-1)
		{
			$x2 = str_replace('px','',$x);
			return UnitsConv::px2mm($x2);
		}

		if(strpos($x,'pt')>-1)
		{
			$x2 = str_replace('pt','',$x);
			return UnitsConv::pt2mm($x2);
		}

		if(strpos($x,'em')>-1)
		{
			$x2 = str_replace('em','',$x);
			return $this->em2mm($x2);
		}

		return $x;
	}

	function any2pt($x,$unit='pt') {

		if(is_numeric($x))
		{
			if($unit)
				$x=''.$x.$unit;
			else
				return $x;
		}

		if(strpos($x,'pt')>-1)
		{
			$x2 = str_replace('pt','',$x);
			return $x2;
		}

		if(strpos($x,'mm')>-1)
		{
			$x2 = str_replace('mm','',$x);
			return UnitsConv::mm2pt($x2);
		}

		if(strpos($x,'cm')>-1)
		{
			$x2 = str_replace('cm','',$x);
			return UnitsConv::mm2pt($x2)*10;
		}

		if(strpos($x,'px')>-1)
		{
			$x2 = str_replace('px','',$x);
			return UnitsConv::px2pt($x2);
		}

		if(strpos($x,'em')>-1)
		{
			$x2 = str_replace('em','',$x);
			return UnitsConv::mm2pt($this->em2mm($x2));
		}

		return $x;
	}

	function endsWith($haystack, $needle)
	{
		$length = strlen($needle);
		if ($length == 0) {
			return true;
		}

		return (substr($haystack, -$length) === $needle);
	}


}//end of class

class PDFDocTpl 
{
	var $pageCount=1;
	var $pageNo=0;
	var $path=null;
	var $pdf;
	
	function __construct($pdf,$path) {
		$this->pdf = $pdf;
		$this->path = $path;

		$this->init();
	}

	function init() 
	{
		if($this->path)
			$tpl->pageCount = $this->pdf->setSourceFile($this->path);
		else
			$tpl->pageCount = 0;

		$this->pageNo = 1;
	}

	function pageIdx() {
		return $this->pageNo - 1;
	}

	function addPage() {
		if($this->path)
		{
			$templateId = $this->pdf->importPage($this->pageNo);
			$this->pdf->AddPage();

			// use the imported page and adjust the page size
			$this->pdf->useTemplate($templateId);
		}
		else
		{
			$this->pdf->AddPage();
		}

		$this->pageNo++;
	}
}

class PDFDoc extends PDF_HTML
{
	// default tags styles
	var $tags = array(
		'h1'=>array('size'=>'20','style'=>'B','color'=>'#550000'),
		'h2'=>array('size'=>'18','style'=>'B','color'=>'#000000'),
		'h3'=>array('size'=>'16','style'=>'B','color'=>'#000000'),
		'h4'=>array('size'=>'12','style'=>'BI','color'=>'#000000'),
		'h5'=>array('size'=>'9','style'=>'I','color'=>'#000000'),
		'p'=> array('size'=>'8','style'=>'','color'=>'#000000'),
		'div'=> array('size'=>'8','style'=>'','color'=>'#000000'),
		'html'=> array('size'=>'8','style'=>'','color'=>'#000000','align'=>'justify'),
		'pagebreakwhen'=> array('maxY'=>'28cm'),
	);

	function __construct($configDoc=null,$orientation='P', $unit='mm', $format='A4')
	{
		if($configDoc && $p=$configDoc['page'])
		{
			if($p['orientation'])
				$orientation=$p['orientation'];

			if($p['unit'])
				$unit=$p['unit'];

			if($p['format'])
				$format=$p['format'];


			if($p['dpi'])
				UnitsConv::setDPI($p['dpi']);
		}

		//Call parent constructor
		parent::__construct($orientation,$unit,$format);

		$this->cellsNextPage=array();
	}

	function createDoc($configDoc=null,$fnReplace=null) {
		if($configDoc['model'])
			$path = $configDoc['model'];

		return $this->loadDoc($path,$configDoc,$fnReplace);
	}

	function loadDoc($path,$configDoc=null,$fnReplace=null) {
		if($configDoc['debug'])
			$this->debug=true;
		else
			$this->debug=false;

		if($path)
			$this->dirPath = dirname($path);

		if($configDoc['page'])
		{
			$margins=null;

			if($configDoc['page']['marginsXY'])
		{
				$margins = $configDoc['page']['marginsXY'];
			}
			elseif($configDoc['page']['margins'])
			{
				$margins = $configDoc['page']['margins'];
			}

			if($margins) 
			{
				$mL = $this->any2mm($margins[0]);
				$this->SetLeftMargin($mL);


				if(count($margins)==1)
					$mT = $mL;
				else
					$mT = $this->any2mm($margins[1]);

				$this->SetTopMargin($mT);


				if(count($margins)<3)
					$mR = $mL;
				else
					$mR = $this->any2mm($margins[2]);
				$this->SetRightMargin($mR);

				if(count($margins)<4)
					$mB = $mT;
				else
					$mB = $this->any2mm($margins[3]);

				$this->SetAutoPageBreak(true,$mB);
			}
		}

		if($configDoc['tags'])
		{
			$this->initTags($configDoc['tags']);
		}
		else
			$this->initTags();

		if($configDoc['selectors'])
		{
			$this->initCSSRules($configDoc['selectors']);
		}

		if($configDoc['defaultStyle'])
		{
			$this->defaultStyle = array_merge($this->tags['p'],$configDoc['defaultStyle']);
		}
		else
			$this->defaultStyle = $this->tags['html'];

		// init default style
		$this->pushStyle($this->defaultStyle);

		if(isset($configDoc['pages']))
			$partsByPage = $configDoc['pages'];
		else
			$partsByPage = null;

		// set the source file
		$tpl = $this->tplDoc = new PDFDocTpl($this,$path);

		$pageCountDesc = count($partsByPage);
		if($pageCountDesc<$tpl->pageCount)
			$tpl->pageCount = $pageCountDesc;

		// iterate through all pages
		for ($i = 0; $i < count($partsByPage); $i++) 
		{
			$tplIdx = $tpl->pageIdx();

			if($tplIdx <= $i)
				// import page
				$tpl->addPage();

			// write in page
			$this->writeParts($partsByPage[$i],$fnReplace);
		}

		$this->onAfterPage();

		return;
		// iterate through all pages
		for ($tpl->pageNo = 1; $tpl->pageNo <= $tpl->pageCount; $tpl->pageNo++) 
		{
			$idx = $tpl->pageIdx();

			// import page
			$tpl->addPage();

			// $templateId = $this->importPage($tpl->pageNo);
			// $this->AddPage();
			// use the imported page and adjust the page size
			// $this->useTemplate($templateId);

			// write in page
			if($partsByPage && count($partsByPage) >  $idx)
				$this->writeParts($partsByPage[$idx],$fnReplace);
		}
	 }

	// add a new page and load nexwt template page if any
	function nextDocPage() {
		$this->tplDoc->addPage();
	}

	function pageBreakWhen() {
		if(isset($this->curStyle['maxY']))
		{
			$maxY = $this->any2mm($this->curStyle['maxY']);
			if($this->y > $maxY) 
			{
				$this->nextDocPage();

				if(isset($this->curStyle['moveToY']))
				{
					$toY = $this->any2mm($this->curStyle['moveToY']);
					$this->setY($toY);
				}
			}
		}
		else
		{
			$this->nextDocPage();

			if(isset($this->curStyle['moveToY']))
			{
				$toY = $this->any2mm($this->curStyle['moveToY']);
				$this->setY($toY);
			}
		}
	}

	function writeParts($parts,$fnReplace) 
	{
		// $this->SetFont(strtolower('arial'));
		
		foreach($parts as $part) 
		{
			$curStyle = $this->getCurStyle();

			if(isset($part['active']) && $part['active']===false)
				continue;

			if(isset($part['font']))
				$curStyle['font']=$part['font'];

			if(isset($part['xy']))
			{
				nxLog("SetXY({$part['xy'][0]},{$part['xy'][1]})",'FPDF');
				$this->_setXY($part['xy'][0],$part['xy'][1]);
				$this->setLeft($this->any2mm($part['xy'][0]));
			}
			elseif(isset($part['y']))
			{
				nxLog("SetY({$part['y']})",'FPDF');
				$this->_setY($part['y']);
				$this->resetLeft();
			}
			else
			{
				$this->resetLeft();
			}

			if(isset($part['column']))
				// manage Y overflow
				$this->setColumn($part['column']);
			else
				$this->setColumn(-1);

			if(isset($part['align']))
				$curStyle['align']=$part['align'];

			if(isset($part['size']))
				$curStyle['size']=$part['size'];

			if(isset($part['debug']))
				$curStyle['debug']=$part['debug'];

			if(isset($part['break']))
				DebugBreak();

			if(isset($part['text']))
				$this->Write($curStyle['size'], $part['text']);

			if(isset($part['html']))
			{
				$content = $part['html'];
				if($fnReplace)
					$content = $fnReplace($content);

				$content = iconv('UTF-8', 'windows-1252', $content);

				if(isset($part['beforeWrite']))
					$s2 = $part['beforeWrite']($content,$this->curStyle,$this,$fnReplace);

				$this->writeHTML($content,$curStyle);

				if(isset($part['afterWrite']))
					$s2 = $part['afterWrite']($content,$this->curStyle,$this,$fnReplace);
			}
		 }
	}
}
?>
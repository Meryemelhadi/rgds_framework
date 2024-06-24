<?php

  define('FPDF_FONTPATH',NX_EXT_LIB.'fpdf/font/');
  require(NX_EXT_LIB.'fpdf/fpdf.php');
  
  class pdf_base extends FPDF
  {

  	// ################################# Initialisation

  	var $wLine; // Largeur maximale de la ligne
  	var $hLine; // Hauteur de la ligne
  	var $Text; // Texte à afficher
  	var $border;
  	var $align; // Justification du texte
  	var $fill;
  	var $Padding;
  	var $lPadding;
  	var $tPadding;
  	var $bPadding;
  	var $rPadding;
  	var $TagStyle; // Style associé à chaque balise
  	var $Indent;
  	var $Space; // Espace minimum entre les mots
  	var $PileStyle;
  	var $Line2Print; // Ligne à afficher
  	var $NextLineBegin; // Tampon entre lignes
  	var $TagName; // Tableau
  	var $Delta; // Largeur maximale moins largeur
  	var $StringLength;
  	var $LineLength;
  	var $wTextLine; //Largeur moins les "padding"
  	var $nbSpace; // Nombre d'espaces dans la ligne
  	var $Xini; // Position initiale
  	var $href; // URL courante
  	var $TagHref; // URL associée à une cellule

  	// ################################# Fonctions Publiques

  	function WriteTag($w,$h,$txt,$border=0,$align="J",$fill=0,$padding=0)
  	{
  		$this->wLine=$w;
  		$this->hLine=$h;
  		$this->Text=trim($txt);
  		$this->Text=ereg_replace("\n|\r|\t","",$this->Text);
  		$this->border=$border;
  		$this->align=$align;
  		$this->fill=$fill;
  		$this->Padding=$padding;

  		$this->Xini=$this->GetX();
  		$this->href="";
  		$this->PileStyle=array();
  		$this->TagHref=array();
  		$this->LastLine=false;

  		$this->SetSpace();
  		$this->Padding();
  		$this->LineLength();
  		$this->BorderTop();

  		while($this->Text!="")
  		{
  			$this->MakeLine();
  			$this->PrintLine();
  		}

  		$this->BorderBottom();
  	}


  	function SetStyle($tag,$family,$style,$size,$color,$indent=-1)
  	{
  		 $tag=trim($tag);
  		 $this->TagStyle[$tag]['family']=trim($family);
  		 $this->TagStyle[$tag]['style']=trim($style);
  		 $this->TagStyle[$tag]['size']=trim($size);
  		 $this->TagStyle[$tag]['color']=trim($color);
  		 $this->TagStyle[$tag]['indent']=$indent;
  	}


  	// ############################ Fonctions privées

  	function SetSpace() // Espace minimal entre les mots
  	{
  		$tag=$this->Parser($this->Text);
  		$this->FindStyle($tag[2],0);
  		$this->DoStyle(0);
  		$this->Space=$this->GetStringWidth(" ");
  	}


  	function Padding()
  	{
  		if(ereg("^.+,",$this->Padding)) {
  			$tab=explode(",",$this->Padding);
  			$this->lPadding=$tab[0];
  			$this->tPadding=$tab[1];
  			if(isset($tab[2]))
  				$this->bPadding=$tab[2];
  			else
  				$this->bPadding=$this->tPadding;
  			if(isset($tab[3]))
  				$this->rPadding=$tab[3];
  			else
  				$this->rPadding=$this->lPadding;
  		}
  		else
  		{
  			$this->lPadding=$this->Padding;
  			$this->tPadding=$this->Padding;
  			$this->bPadding=$this->Padding;
  			$this->rPadding=$this->Padding;
  		}
  		if($this->tPadding<$this->LineWidth)
  			$this->tPadding=$this->LineWidth;
  	}


  	function LineLength()
  	{
  		if($this->wLine==0)
  			$this->wLine=$this->fw - $this->Xini - $this->rMargin;

  		$this->wTextLine = $this->wLine - $this->lPadding - $this->rPadding;
  	}


  	function BorderTop()
  	{
  		$border=0;
  		if($this->border==1)
  			$border="TLR";
  		$this->Cell($this->wLine,$this->tPadding,"",$border,0,'C',$this->fill);
  		$y=$this->GetY()+$this->tPadding;
  		$this->SetXY($this->Xini,$y);
  	}


  	function BorderBottom()
  	{
  		$border=0;
  		if($this->border==1)
  			$border="BLR";
  		$this->Cell($this->wLine,$this->bPadding,"",$border,0,'C',$this->fill);
  	}


  	function DoStyle($tag) // Applique un style
  	{
  		$tag=trim($tag);
  		$this->SetFont($this->TagStyle[$tag]['family'],
  			$this->TagStyle[$tag]['style'],
  			$this->TagStyle[$tag]['size']);

  		$tab=explode(",",$this->TagStyle[$tag]['color']);
  		if(count($tab)==1)
  			$this->SetTextColor($tab[0]);
  		else
  			$this->SetTextColor($tab[0],$tab[1],$tab[2]);
  	}


  	function FindStyle($tag,$ind) // Héritage des éléments parents
  	{
  		if(empty($tag))
  			return ;

  		// Famille
  		if($this->TagStyle[$tag]['family']!="")
  			$family=$this->TagStyle[$tag]['family'];
  		else
  		{
  			reset($this->PileStyle);
  			while(list($k,$val)=each($this->PileStyle))
  			{
  				$val=trim($val);
  				if($this->TagStyle[$val]['family']!="") {
  					$family=$this->TagStyle[$val]['family'];
  					break;
  				}
  			}
  		}

  		// Style
  		$style1=strtoupper($this->TagStyle[$tag]['style']);
  		if($style1=="N")
  			$style="";
  		else
  		{
  			reset($this->PileStyle);
  			while(list($k,$val)=each($this->PileStyle))
  			{
  				$val=trim($val);
  				$style1=strtoupper($this->TagStyle[$val]['style']);
  				if($style1=="N")
  					break;
  				else
  				{
  					if(ereg("B",$style1))
  						$style['b']="B";
  					if(ereg("I",$style1))
  						$style['i']="I";
  					if(ereg("U",$style1))
  						$style['u']="U";
  				}
  			}
  			$style=$style['b'].$style['i'].$style['u'];
  		}

  		// Taille
  		if($this->TagStyle[$tag]['size']!=0)
  			$size=$this->TagStyle[$tag]['size'];
  		else
  		{
  			reset($this->PileStyle);
  			while(list($k,$val)=each($this->PileStyle))
  			{
  				$val=trim($val);
  				if($this->TagStyle[$val]['size']!=0) {
  					$size=$this->TagStyle[$val]['size'];
  					break;
  				}
  			}
  		}

  		// Couleur
  		if($this->TagStyle[$tag]['color']!="")
  			$color=$this->TagStyle[$tag]['color'];
  		else
  		{
  			reset($this->PileStyle);
  			while(list($k,$val)=each($this->PileStyle))
  			{
  				$val=trim($val);
  				if($this->TagStyle[$val]['color']!="") {
  					$color=$this->TagStyle[$val]['color'];
  					break;
  				}
  			}
  		}

  		// Résultat
  		$this->TagStyle[$ind]['family']=$family;
  		$this->TagStyle[$ind]['style']=$style;
  		$this->TagStyle[$ind]['size']=$size;
  		$this->TagStyle[$ind]['color']=$color;
  		$this->TagStyle[$ind]['indent']=$this->TagStyle[$tag]['indent'];
  	}


  	function Parser($text)
  	{
  		$tab=array();
  		// Balise fermante
  		if(ereg("^(</([^>]+)>).*",$text,$regs)) {
  			$tab[1]="c";
  			$tab[2]=trim($regs[2]);
  		}
  		// Balise ouvrante
  		else if(ereg("^(<([^>]+)>).*",$text,$regs)) {
  			$regs[2]=ereg_replace("^a","a ",$regs[2]); // Rustine : l'espace disparaît
  			$tab[1]="o";
  			$tab[2]=trim($regs[2]);

  			// Présence d'attributs
  			if(ereg("(.+) (.+)='(.+)' *",$regs[2])) {
  				$tab1=split(" +",$regs[2]);
  				$tab[2]=trim($tab1[0]);
  				while(list($i,$couple)=each($tab1))
  				{
  					if($i>0) {
  						$tab2=explode("=",$couple);
  						$tab2[0]=trim($tab2[0]);
  						$tab2[1]=trim($tab2[1]);
  						$end=strlen($tab2[1])-2;
  						$tab[$tab2[0]]=substr($tab2[1],1,$end);
  					}
  				}
  			}
  		}
  	 	// Espace
  	 	else if(ereg("^( ).*",$text,$regs)) {
  			$tab[1]="s";
  			$tab[2]=$regs[1];
  		}
  		// Texte
  		else if(ereg("^([^< ]+).*",$text,$regs)) {
  			$tab[1]="t";
  			$tab[2]=trim($regs[1]);
  		}
  		// Elagage
  		$begin=strlen($regs[1]);
   		$end=strlen($text);
   		$text=substr($text, $begin, $end);
  		$tab[0]=$text;

  		return $tab; // 0 : $text; 1 : type de balise (tag); 2 : élément
  	}


  	function MakeLine() // Fabrique une ligne
  	{
  		$this->Text.=" ";
  		$this->LineLength=array();
  		$this->TagHref=array();
  		$Length=0;
  		$this->nbSpace=0;

  		$i=$this->BeginLine();
  		$this->TagName=array();

  		if($i==0) {
  			$Length=$this->StringLength[0];
  			$this->TagName[0]=1;
  			$this->TagHref[0]=$this->href;
  		}

  		while($Length<$this->wTextLine)
  		{
  			$tab=$this->Parser($this->Text);
  			$this->Text=$tab[0];
  			if($this->Text=="") {
  				$this->LastLine=true;
  				break;
  			}

  			if($tab[1]=="o") {
  				array_unshift($this->PileStyle,$tab[2]);
  				$this->FindStyle($this->PileStyle[0],$i+1);

  				$this->DoStyle($i+1);
  				$this->TagName[$i+1]=1;
  				if($this->TagStyle[$tab[2]]['indent']!=-1) {
  					$Length+=$this->TagStyle[$tab[2]]['indent'];
  					$this->Indent=$this->TagStyle[$tab[2]]['indent'];
  				}
  				if($tab[2]=="a")
  					$this->href=$tab['href'];
  			}

  			if($tab[1]=="c") {
  				array_shift($this->PileStyle);
  				$this->FindStyle($this->PileStyle[0],$i+1);
  				$this->DoStyle($i+1);
  				$this->TagName[$i+1]=1;
  				if($this->TagStyle[$tab[2]]['indent']!=-1) {
  					$this->LastLine=true;
  					$this->Text=trim($this->Text);
  					break;
  				}
  				if($tab[2]=="a")
  					$this->href="";
  			}

  			// space
  			if($tab[1]=="s") {
  				$i++;
  				$Length+=$this->Space;
  				$this->Line2Print[$i]="";
  				if($this->href!="")
  					$this->TagHref[$i]=$this->href;
  			}

  			
  			if($tab[1]=="t") {
  				$i++;
  				$this->StringLength[$i]=$this->GetStringWidth($tab[2]);
  				$Length+=$this->StringLength[$i];
  				$this->LineLength[$i]=$Length;
  				$this->Line2Print[$i]=$tab[2];
  				if($this->href!="")
  					$this->TagHref[$i]=$this->href;
  			 }

  		}

  		trim($this->Text);
  		if($Length>$this->wTextLine || $this->LastLine==true)
  			$this->EndLine();
  	}


  	function BeginLine()
  	{
  		$this->Line2Print=array();
  		$this->StringLength=array();

  		$this->FindStyle($this->PileStyle[0],0);
  		$this->DoStyle(0);

  		if(count($this->NextLineBegin)>0) {
  			$this->Line2Print[0]=$this->NextLineBegin['text'];
  			$this->StringLength[0]=$this->NextLineBegin['length'];
  			$this->NextLineBegin=array();
  			$i=0;
  		}
  		else {
  			ereg("^(( *(<([^>]+)>)* *)*)(.*)",$this->Text,$regs);
  			$regs[1]=ereg_replace(" ", "", $regs[1]);
  			$this->Text=$regs[1].$regs[5];
  			$i=-1;
  		}

  		return $i;
  	}


  	function EndLine()
  	{
  		if(end($this->Line2Print)!="" && $this->LastLine==false) {
  			$this->NextLineBegin['text']=array_pop($this->Line2Print);
  			$this->NextLineBegin['length']=end($this->StringLength);
  			array_pop($this->LineLength);
  		}

  		while(end($this->Line2Print)==="")
  			array_pop($this->Line2Print);

  		$this->Delta=$this->wTextLine-end($this->LineLength);

  		$this->nbSpace=0;
  		for($i=0; $i<count($this->Line2Print); $i++) {
  			if($this->Line2Print[$i]=="")
  				$this->nbSpace++;
  		}
  	}


  	function PrintLine()
  	{
  		$border=0;
  		if($this->border==1)
  			$border="LR";
  		$this->Cell($this->wLine,$this->hLine,"",$border,0,'C',$this->fill);
  		$y=$this->GetY();
  		/*ADDED*/$this->Xini = $this->lMargin;
  		$this->SetXY($this->Xini+$this->lPadding, $y);

  		if($this->Indent!=-1) {
  			if($this->Indent!=0)
  				$this->Cell($this->Indent,$this->hLine,"",0,0,'C',0);
  			$this->Indent=-1;
  		}

  		$space=$this->LineAlign();
  		$this->DoStyle(0);
  		for($i=0; $i<count($this->Line2Print); $i++)
  		{
  			if($this->TagName[$i]==1)
  				$this->DoStyle($i);
  			if($this->Line2Print[$i]=="")
  				$this->Cell($space,$this->hLine,"         ",0,0,'C',0,$this->TagHref[$i]);
  			else
  				$this->Cell($this->StringLength[$i],$this->hLine,$this->Line2Print[$i],0,0,'C',0,$this->TagHref[$i]);
  		}

  		$this->LineBreak();
  		if($this->LastLine && $this->Text!="")
  			$this->EndParagraph();
  		$this->LastLine=false;
  	}


  	function LineAlign()
  	{
  		$space=$this->Space;
  		if($this->align=="J") {
  			if($this->nbSpace!=0)
  				$space=$this->Space + ($this->Delta/$this->nbSpace);
  			if($this->LastLine)
  				$space=$this->Space;
  		}

  		if($this->align=="R")
  			$this->Cell($this->Delta,$this->hLine,"",0,0,'C',0);

  		if($this->align=="C")
  			$this->Cell($this->Delta/2,$this->hLine,"",0,0,'C',0);

  		return $space;
  	}


  	function LineBreak() // Retour à la ligne
  	{
  		$x=$this->lMargin; /*MODIFIED*///$this->Xini;
  		$y=$this->GetY()+$this->hLine;
  		$this->SetXY($x,$y);
  	}


  	function EndParagraph() // Interligne entre paragraphes
  	{
  		$border=0;
  		if($this->border==1)
  			$border="LR";
  		$this->Cell($this->wLine,$this->hLine/2,"",$border,0,'C',$this->fill);
  		$x=$this->Xini;
  		$y=$this->GetY(); /*MODIFIED*///+$this->hLine/2; DISABLE space between paragraph
  		$this->SetXY($x,$y);
  	}



    //***************************************************************
    //***************************************************************

    function NbLines($w,$txt)
    {
        //Computes the number of lines a MultiCell of width w will take
        $cw=&$this->CurrentFont['cw'];
        if($w==0)
            $w=$this->w-$this->rMargin-$this->x;
        $wmax=($w-2*$this->cMargin)*1000/$this->FontSize;
        $s=str_replace("\r",'',$txt);
        $nb=strlen($s);
        if($nb>0 and $s[$nb-1]=="\n")
            $nb--;
        $sep=-1;
        $i=0;
        $j=0;
        $l=0;
        $nl=1;
        while($i<$nb)
        {
            $c=$s[$i];
            if($c=="\n")
            {
                $i++;
                $sep=-1;
                $j=$i;
                $l=0;
                $nl++;
                continue;
            }
            if($c==' ')
                $sep=$i;
            $l+=$cw[$c];
            if($l>$wmax)
            {
                if($sep==-1)
                {
                    if($i==$j)
                        $i++;
                }
                else
                    $i=$sep+1;
                $sep=-1;
                $j=$i;
                $l=0;
                $nl++;
            }
            else
                $i++;
        }
        return $nl;
    }
    
    //***************************************************************
    //***************************************************************

    //En-tête
    function Header()
    {
      /**/
    }

    //Pied de page
    function Footer()
    {
    }
    
    //***************************************************************
    //***************************************************************
    
  	function SetCol($col)
  	{
      //Positionnement sur une colonne
      $this->col=$col;
      $x=20+$col*89; //82larg+6esp //88
      $this->SetLeftMargin($x);
      $this->SetX($x);
  	}

  	function colMarginAdjust()
  	{
  		$this->colCurrentHeight = 297-30-$this->colStartY;
  		$this->colCurrentHeight -= $this->colCurrentHeight%$this->colCellHeight; // corrigé en multiple de taille cellule
      if ( $this->colCurrentHeight*$this->colNb > $this->colRemainingHeight )
      {
        // Assez de place pour caser le texte dans deux colonnes sur cette page
        $new_bMargin = 30 + $this->colCurrentHeight - floor($this->colRemainingHeight / $this->colNb); // 20 MMMMMMMM
        $new_bMargin -= $new_bMargin%$this->colCellHeight;
        $new_bMargin -= 1; // securité due au décalage observé avec WriteTag
        $this->SetAutoPageBreak(true, $new_bMargin);
      }
      else
      {
        // Il faudra déborder sur la page suivante
        $this->SetAutoPageBreak(true, 30);
      }
   	}

  	function AcceptPageBreak()
  	{
      //Méthode autorisant ou non le saut de page automatique
      if ($this->colActivation)
      {
        // en mode colonne
        if($this->col<$this->colNb-1)
        {
          //Passage à la colonne suivante
          $this->SetCol($this->col+1);
          //Ordonnée en haut
          $this->SetY($this->colStartY);
          //On reste sur la page
          return false;
        }
        else
        {
          //Retour en première colonne
          $this->SetCol(0);
          $this->colStartY = $this->tMargin;
          // Déplacement marge éventuelle
          $this->colRemainingHeight -= $this->colCurrentHeight*$this->colNb;
          $this->colMarginAdjust();
          //Saut de page
          return true;
        }
      }
      else
      {
        // Mode normal
        return true;
      }
  	}

  	function AddColText($text)
  	{
      if ($text!="")
      {
        $this->colActivation = true;
        $this->colStartY = ceil($this->getY());
        $this->colNb = 2;
        $this->colCellHeight = 4;
        // styles
        $this->SetStyle("p","times","N",9,"0,0,0",0);
        $this->SetStyle("i","times","I",0,"0,0,0");
        //Police
        $this->SetFont('times','',9);
        // Calculs Hauteurs sans balises (suppression balises <i> et </i>)
        $eval_text = str_replace("<i>", "", $text);
        $eval_text = str_replace("</i>", "", $eval_text);
        $eval_text = str_replace("<b>", "", $eval_text);
        $eval_text = str_replace("</b>", "", $eval_text);
        $this->colTotalHeight = $this->NbLines(82, $eval_text)*$this->colCellHeight;
        $this->colRemainingHeight = $this->colTotalHeight;
        // Déplacement marge éventuel
        $this->colMarginAdjust();
        // start col
        $this->SetCol(0);
        //Sortie du texte avec balises
        $text = str_replace("\n", "</p><p>", "<p>".$text."</p>");
        $this->WriteTag(80, $this->colCellHeight, $text,0,"J",0,0); //==largeur 80+2 // Décalé de 1 mm, compter +1 sur X avec cette fonction
        //Retour en première colonne
        $this->SetCol(0);
        // disable colonnage
        $this->colActivation = false;
        $this->colLastMargin = $this->bMargin;
        $this->SetAutoPageBreak(true, 30);
      }
  	}
  	
  	function AddFlowText($text)
  	{
      //$this->SetX(20);
      $this->SetStyle("p","times","N",10,"0,0,0",0);
      $this->SetStyle("i","times","I", 0,"0,0,0");
      $this->SetStyle("b","times","B", 0,"0,0,0");
      $text = str_replace("\n", "</p><p>", "<p>".$text."</p>");
      $this->WriteTag(170, 4, $text, 0, "N", 0, 0); //==largeur 80+2 // Décalé de 1 mm, compter +1 sur X avec cette fonction
    }

    // +FY
  	function AddHTML($text)
  	{
      //$this->SetX(20);
      $this->SetStyle("p","times","N",10,"0,0,0",0);
      $this->SetStyle("i","times","I", 0,"0,0,0");
      $this->SetStyle("b","times","B", 0,"0,0,0");
      // $text = str_replace("\n", "</p><p>", "<p>".$text."</p>");
      $this->WriteTag(170, 4, $text, 0, "N", 0, 0); //==largeur 80+2 // Décalé de 1 mm, compter +1 sur X avec cette fonction
    }
    
    function Error($msg)
	{
		//Fatal error
		nxError($msg,'FPDF');
		die('<B>FPDF error: </B>'.$msg);
	}
  	
  	//***************************************************************
  	//***************************************************************
  	//***************************************************************
  }
  
  ?>
<?php

class NxSoundex {

	function soundex_fr_ar( $sIn ) /// SOUNDEX 2 V FRANCAISE + Arabe
	{
		// Si il n'y a pas de mot, on sort immédiatement

		if ( $sIn === '' ) return ' ';
		/// On supprime tout ce qui n'est pas une lettre

		/// On supprime les accents
		$sIn = strtr($sIn, 'àáâäæçèéêëíìîïòóôöùúûü', 'aaaaaceeeeeiiiioooouuuu');

		 // suppression des caractères spéciaux
		$str = preg_replace(array('`&rsquo;`','`\W`'), ' ', $str);

		/// On met tout en majuscule
		$sIn = strtoupper( $sIn );
		$sIn = preg_replace( '`[^A-Z]`', '', $sIn );

		// Si la chaine ne fait qu'un seul caractere, on sort avec.
		if ( strlen( $sIn ) === 1 ) return $sIn . ' ';



		//*************   MODIF AZIZ **************************
		//
		//*****************************************************
		//
		//  Changement  de "np, nb et nm" en "mp, mb et mm".

		$convIn = array('`NB`', '`NP`', '`NM`' );
		$convOut = array('MB', 'MP', 'MM' );
		$sIn = preg_replace( $convIn, $convOut, $sIn );


		/// suppression des H sauf CH ou SH
		$sIn = preg_replace( '`((![CS])|(!^))H`', '', $sIn );

		$convIn = array('GM'/*+FY*/, 'GE','GI'/*+FY*/,'GUI', 'GUE', 'GA', 'GO', 'GU', 'CA', 'CO', 'CU', 'CCION', 'Q', 'CC', 'CK', 'IZM', 'TION', 'X','IZM', 'CA','CKA','CI','CE','CY','SH');
		$convOut = array( 'GEM','JE','JE','KI', 'KE', 'KA', 'KO', 'K', 'KA', 'KO', 'KU', 'KSION', 'K', 'K', 'K' , 'ISM', 'SSION', 'KS','ISM','KA','KA','SSI','SSE','SSY','CH');
		$sIn = str_replace( $convIn, $convOut, $sIn );

		// +FY: on remplace les Y entre consonnes par I
		// remplacer des par de qd suivi par consonne ex. descartes
		// remplacer des, mes, les par de, me, le
		$sIn=preg_replace(
			array
				('/([^EIOU])Y([^EIOU])/',
				'/([DL])ES([CMPDFGJLBK])/',
				'/^([DSLM])ES$/',
				'/(E?[AU])[TD]$/',
				'/(E?[AU])LT$/'
			),
			array(
				'$1I$2',
				'$1E$2',
				'$1E',
				'$1',
				'$1'
				),
			$sIn);

	/// on remplace les voyelles sauf le Y et sauf la première par A
		$sIn = preg_replace( '`(?<!^)[EIOU]`', 'A', $sIn );
		$sIn = preg_replace( '`(^)[EIAOYU]`', 'A', $sIn );
		/// on remplace les prefixes puis on conserve la premiere lettre
		/// et on fait les remplacements complementaires

		/*$convIn = array( '`^KN`', '`^(PH|PF)`', '`^MAC`', '`^SCH`', '`^ASA`', '`(?<!^)KN`', '`(?<!^)(PH|PF)`', '`(?<!^)MAC`',
		'`(?<!^)SCH`','`(?<!^)ASA`' );
		$convOut = array( 'NN', 'FF', 'MCC', 'SSS', 'AZA', 'NN', 'FF', 'MCC', 'SSS', 'AZA' );
		$sIn = preg_replace( $convIn, $convOut, $sIn );*/

		$convIn = array('`^(PH)`', '`^MAC`', '`^SCH`', '`^ASA`', '`(?<!^)CHN`', '`(?<!^)(PH|PF)`', '`(?<!^)MAC`',
		'`(?<!^)SCH`','`(?<!^)ASA`' );
		$convOut = array('F', 'MCC', 'SSS', 'AZA', 'KN', 'FF', 'MCC', 'SSS', 'AZA' );
		$sIn = preg_replace( $convIn, $convOut, $sIn );

		/// on supprime les lettres repetitives
		$sIn = preg_replace( '`(.)\1`', '$1', $sIn );

		/// suppression des Y sauf precedes d'un A
		$sIn = preg_replace( '`(?<!A)Y`', '', $sIn );
		/// on supprime les terminaisons A, T, D, S
		//$sIn = preg_replace( '`[ATS]$`', '', $sIn );

		/// suppression de tous les A sauf en tete
		$sIn = preg_replace( '`(?!^)A`', '', $sIn );

		$convIn = array( 'TH');
		$convOut = array( 'T');
	 	$sIn = str_replace( $convIn, $convOut, $sIn );
		/// on ne retient que 4 caracteres ou on complete avec des blancs
		while(strlen($sIn)<5)
		{
			$sIn=$sIn.'01234';
		}
		return trim($sIn);
	}
}

?>
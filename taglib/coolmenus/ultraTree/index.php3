<?
// UltraTree v.1.1 by Mike Baikov (mikebmv@hotmail.com)
// this program fetches the categories from your database
// and builds a nice threaded tree ***using only one select***
// yeah, that's right, it saves lots of time
// CONDITIONS OF USE
// you may use it as you like under the following conditions:
// 1) this script is provided AS IS WITHOUT ANY WARRANTY
// 2) you'll keep the notice <!--built by UltraTree v.1.1 http://www.tourbase.ru/zink/-->
// in the HTML output
// 3) you shall rack YOUR brain to solve all problems related
// with UltraTree and related software
// 4) don't say this script is not well documented
//
// UltraTree homepage: http://www.tourbase.ru/zink/

// time benchamrk (optional) start time count
  $timeparts = explode(" ",microtime());
// time benchamrk end

// dblayer.php3 is my DB-abstraction layer
include "dblayer.php3";
// include a nice header
include "header.php3";

// this function select the data from a DB and passes it to makebranch
// this line: $partable[$catid][$parcat]=$name;
// is used for buildparent
// you may delete it if you don't use buildparent
// this will save memory and time
function maketree($rootcatid,$sql,$maxlevel){
// $sql is the sql statement which fetches the data
// you MUST keep this order:
// 1) the category ID, 2) the parent category ID, 3) the name of the category
         $result=dbquery($sql);
                 while(list($catid,$parcat,$name)=dbfetch($result)){
                 $table[$parcat][$catid]=$name;
                 $partable[$catid][$parcat]=$name;
                 };
         $result=buildparent($rootcatid,$table,$partable)."<br>";
         $result.=makebranch($rootcatid,$table,0,$maxlevel);
         // please keep this notice
         $result.="<!--built by UltraTree v.1.1 http://www.tourbase.ru/zink/-->\n";
         RETURN $result;
}

// this function builds the branches,
// sorting them in alphabetical order
function makebranch($parcat,$table,$level,$maxlevel){
         $list=$table[$parcat];
         asort($list); // here we do the sorting
                while(list($key,$val)=each($list)){
                      // do the indent
                      if ($level=="0"){
                      $output="<img src=se.gif width=12 height=12>";
                      }else{
                      $width=($level+1)*24;
                      $output="<img src=e.gif width=$width height=12>";
                      };
                // the resulting HTML - feel free to change it
                // $level is optional
                $result.= "$output <a href=index.php3?catid=$key>$val</a> ($level)<br>\n";
                      if ((isset($table[$key])) AND (($maxlevel>$level+1) OR ($maxlevel=="0"))){
                      $result.= makebranch($key,$table,$level+1,$maxlevel);
                      };
                };
         RETURN $result;
}

// this function makes the list of the parent categories
// this function is optional
function buildparent($catid,$table,$partable){
         if ($catid!=0){
         $list=$partable[$catid];
         $result=each($list);
         $output="<a href=index.php3?catid=$result[0]>$result[1]</a> / ";
         $output=buildparent($result[0],$table,$partable).$output;
         };
         RETURN $output;
}

// set the default category
if (!isset($catid)){
$catid=0;
};
// build and print the tree NOTE: $maxlevel is the maximum level,
// set to 0 to show all levels
$maxlevel=0;
print maketree($catid,"SELECT catid,parcat,name FROM tree order by parcat",$maxlevel);

// time benchamrk (optional): end time count, get benchmark result
  $starttime = $timeparts[1].substr($timeparts[0],1);
  $timeparts = explode(" ",microtime());
  $endtime = $timeparts[1].substr($timeparts[0],1);
  $totaltime=bcsub($endtime,$starttime,6);
print "<font size=\"1\">UltraTree built in $totaltime sec</font>";
// time benchmark end
?>
<?
// this script will create a table called TREETEST
// and populate it with 500 lines of random data
// ...just to benchmark

// CONDITIONS OF USE
// you may use it as you like under the following conditions:
// 1) this script is provided AS IS WITHOUT ANY WARRANTY
// 2) you'll keep the notice <!--built by UltraTree v.1.1 http://www.tourbase.ru/zink/-->
// in the HTML output
// 3) you shall rack YOUR brain to solve all problems related
// with UltraTree and related software
// 4) don't say this script is not well documented

include "dblayer.php3";
include "header.php3";
$sql="CREATE TABLE treetest (   catid mediumint(5) NOT NULL auto_increment, parcat mediumint(5) NOT NULL,   name varchar(50) NOT NULL,   UNIQUE catid (catid))";
dbquery($sql);

srand((double)microtime()*1000000);
for ($i=1;$i<=500;$i++){
if ($i>20){
$parcat=rand(1,100);
}else{
$parcat=0;
};
$sql="INSERT INTO treetest (catid,parcat,name) VALUES ('$i','$parcat','CATEG $i-$parcat')";
dbquery($sql);
print "$i - $parcat<br>";
};
?>
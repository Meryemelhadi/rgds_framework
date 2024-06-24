<?php $properties["query"]="INSERT INTO _ds_category(_soid,_treeid,_fullpath,_path,_name,_full_name,_itemid,_selprompt,_index,_perms,_lang) VALUES('".$props->getProperty("site.oid","",true)."',".
"'".$props->getProperty("cat.id","",true)."',".
$record->getFieldValue("fullpath","DB","NULL").",".
$record->getFieldValue("path","DB","%{property:path}").",".
$record->getFieldValue("name","DB","NULL").",".
$record->getFieldValue("full_name","DB","NULL").",".
$record->getFieldValue("itemid","DB","NULL").",".
$record->getFieldValue("selprompt","DB","NULL").",".
$record->getFieldValue("index","DB","%{property:index}").",".
$record->getFieldValue("perms","DB","NULL").",".
$record->getFieldValue("lang","DB","en").")"; ?>
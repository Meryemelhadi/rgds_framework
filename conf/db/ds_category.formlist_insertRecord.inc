<?php $properties["query"]="INSERT INTO _ds_category(_oid,_soid,_treeid,_fullpath,_path,_name,_itemid,_full_name,_selprompt,_index,_perms,_lang) VALUES(".$record->getFieldValue("oid","DB","NULL").",".
"'".$props->getProperty("site.oid","",true)."',".
"'".$props->getProperty("cat.id","nav_pages",true)."',".
$record->getFieldValue("fullpath","DB","NULL").",".
$record->getFieldValue("path","DB","NULL").",".
$record->getFieldValue("name","DB","NULL").",".
$record->getFieldValue("itemid","DB","NULL").",".
$record->getFieldValue("full_name","DB","NULL").",".
$record->getFieldValue("selprompt","DB","NULL").",".
$record->getFieldValue("index","DB","NULL").",".
$record->getFieldValue("perms","DB","NULL").",".
$record->getFieldValue("lang","DB","en").")"; ?>
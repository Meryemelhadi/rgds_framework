@ECHO get files modified last %1 days and tgz it in "modif_%2.tgz" ...
@echo off
findU.exe %2 -type f -mtime -%1 -print > "modif_%2.txt"
tar.exe -c --files-from "modif_%2.txt" -f - | gzip > "modif_%2.tgz"
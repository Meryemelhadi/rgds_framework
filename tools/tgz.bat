@echo off
rem tar.exe -cvf "%1" -f - | gzip > "%1.tgz"
tar -z -cf "%1.tgz" "%1"

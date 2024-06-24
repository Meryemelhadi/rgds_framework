@echo off
tar.exe -c --files-from "%1" -f - | gzip > "%1.tgz"
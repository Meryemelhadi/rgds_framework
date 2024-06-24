@echo off
findU.exe %2 -type f -mtime -%1 -print > "modif_%2.txt"
#!/bin/sh
#
# run php script that backup the db and other files
# 
# install:
# - edit file dbsender.php and change parameters for the site db, ftp and email
# - copy file in /var/www/nx/bak/ on webserver (local for this site, ie. with chroot set to the site)
# - copy this sh file in /etc/cron.daily directory of site to be backed up
# - check that backup works daily (email and ftp)

php /var/www/nx/bak/dbsender.php > /dev/null

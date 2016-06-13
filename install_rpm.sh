#!/bin/sh
source repo_scripts/repo_funcs.sh
if [ $#  != 1 ] ;  
	then echo usage $0 path_to_rpm; exit 1;
	
fi
echo starting rpm install

copy_rpms_to_eol_repo $1


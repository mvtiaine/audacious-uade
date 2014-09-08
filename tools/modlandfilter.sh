#!/bin/sh

grep -E -f modland_amiga_dirs.txt allmods_md5.txt | sort > allmods_md5_amiga.txt
# cut -c 34-9999 allmods_md5_amiga.txt | sort > allmods_amiga_sorted.txt
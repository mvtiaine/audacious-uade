#!/bin/sh

while read -r line; do
  DIR=$(echo $line | cut -c 7-9999)
  ln -s "../modules/$DIR"
done < $(dirname -- "$0")/modland_amiga_dirs.txt
ln -s "../../incoming/vault/Cinemaware"

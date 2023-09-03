#!/bin/sh

while read -r line; do
  DIR=$(echo $line | cut -c 7-9999)
  ln -s "../modules/$DIR"
done < $(dirname -- "$0")/modland_amiga_dirs.txt
# these have author info available in "standard" format
ln -s "../../incoming/vault/Cinemaware"
ln -s "../../incoming/workshop/channel players"
ln -s "../../incoming/workshop/chiptracker"
ln -s "../../incoming/workshop/NP2"
ln -s "../../incoming/workshop/P41A"
ln -s "../../incoming/workshop/P4X"
ln -s "../../incoming/workshop/P60"
ln -s "../../incoming/workshop/P6X"
ln -s "../../incoming/workshop/PHA"
ln -s "../../incoming/workshop/prun"

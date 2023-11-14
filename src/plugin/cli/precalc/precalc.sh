#!/bin/sh

export PRECALC=$(dirname "$0")/precalc
export INCLUDEPATH=$1

run_uade() {
  HOME=/tmp/songdb/$1
  mkdir -p $HOME
  #MD5=$(md5 -q "$2")
  #MD5=($(md5sum "$2"))
  #if ! grep -q $MD5 "$SONGDB_TMP"; then
    "$PRECALC" "$2" $INCLUDEPATH 2>>/tmp/songdb/stderr >> "$HOME/songdb_tmp.tsv"
  #fi
}

mkdir -p /tmp/songdb

export -f run_uade
find -L  . -type f | sed "s/^\.\///g" | parallel --timeout 900 run_uade {%} {}

cat /tmp/songdb/*/songdb_tmp.tsv | sort | uniq > /tmp/songdb/songdb.tsv

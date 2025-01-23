#!/bin/sh

export PRECALC=$(dirname "$0")/precalc
export INCLUDEPATH=$1

run_uade() {
  HOME=/tmp/songdb/$1
  mkdir -p $HOME
  "$PRECALC" "$2" $INCLUDEPATH 2>>/tmp/songdb/stderr >> "$HOME/songdb_tmp.tsv"
}

mkdir -p /tmp/songdb

export -f run_uade
find -L  . -type f | sed "s/^\.\///g" | parallel --nice 20 --timeout 1800 run_uade {%} {}

cat /tmp/songdb/*/songdb_tmp.tsv | sort | uniq > /tmp/songdb/songdb.tsv

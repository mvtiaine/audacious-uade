#!/bin/sh

export PRECALC=$(dirname "$0")/precalc
export INCLUDEPATH=$1

run_uade() {
  HOME=/tmp/songdb/$1
  mkdir -p $HOME
  echo "Processing $2" >> $HOME/stderr
  "$PRECALC" "$2" $INCLUDEPATH >> $HOME/songdb_tmp.tsv 2>> $HOME/stderr
  RES=$?
  if [ "$RES" -gt "1" ]; then echo "Failed to process $2 - exit code $RES " | tee -a $HOME/stderr; fi
}

mkdir -p /tmp/songdb

export -f run_uade
find -L  . -type f | sed "s/^\.\///g" | parallel --nice 20 --timeout 1800 run_uade {%} {}

cat /tmp/songdb/*/songdb_tmp.tsv | sort | uniq > /tmp/songdb/songdb.tsv
cat /tmp/songdb/*/stderr > /tmp/songdb/stderr

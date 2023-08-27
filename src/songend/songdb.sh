#!/bin/sh

export UADE_SONGEND=$(dirname "$0")/uade_songend
export SONGDB_TMP=/tmp/uade_songend/songdb_tmp.tsv
export INCLUDEFNAME=$1

run_uade() {
  HOME=/tmp/uade_songend/$1
  mkdir -p $HOME
  MD5=$(md5 -q "$2")
  if ! grep -q $MD5 "$SONGDB_TMP"; then
    "$UADE_SONGEND" "$2" $INCLUDEFNAME 2>>/tmp/uade_songend/stderr >> "$SONGDB_TMP"
  fi
}

mkdir -p /tmp/uade_songend
touch "$SONGDB_TMP"

export -f run_uade
find -L  . -type f | sed "s/^\.\///g" | parallel run_uade {%} {}

sort "$SONGDB_TMP" | uniq > /tmp/uade_songend/songdb.tsv

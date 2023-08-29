#!/bin/sh

export UADE_SONGEND=$(dirname "$0")/uade_songend
export INCLUDEFNAME=$1

run_uade() {
  HOME=/tmp/uade_songend/$1
  mkdir -p $HOME
  #MD5=$(md5 -q "$2")
  #MD5=($(md5sum "$2"))
  #if ! grep -q $MD5 "$SONGDB_TMP"; then
    "$UADE_SONGEND" "$2" $INCLUDEFNAME 2>>/tmp/uade_songend/stderr >> "$HOME/songdb_tmp.tsv"
  #fi
}

mkdir -p /tmp/uade_songend

export -f run_uade
find -L  . -type f | sed "s/^\.\///g" | parallel run_uade {%} {}

cat /tmp/uade_songend/*/songdb_tmp.tsv | sort | uniq > /tmp/uade_songend/songdb.tsv

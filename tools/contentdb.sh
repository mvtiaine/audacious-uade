#!/bin/sh

# TODO lldb > crash/abort(kill) trace + filename
run_uade() {
  HOME=/tmp/uade_contentdb/$1
  mkdir -p $HOME
  MD5=$(md5 -q "$2")
  if ! grep -q $MD5 "$HOME/.uade/contentdb"; then
    uade123 -1 --frequency=3000 --filter=none --resampler=none --panning=0 -f /dev/null "$2" > /dev/null 2>/dev/null
  fi
}

export -f run_uade
find -L  . -type f | parallel run_uade {%} {}

cat /tmp/uade_contentdb/*/.uade/contentdb | sort | uniq > /tmp/uade_contentdb/uade_contentdb

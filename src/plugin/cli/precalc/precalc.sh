#!/bin/bash

set -o pipefail


export PROBE=1
export PLAY=$(dirname "$0")/../player/player
export PRECALC=$(dirname "$0")/precalc
export INCLUDEPATH=$1
export TAC=$(which tac || echo tail -r)

run_uade() {
  local HOME="/tmp/songdb/$1"
  local WORK="$HOME"
  mkdir -p $WORK
  echo "Processing $2" >> "$WORK/stderr"
  "$PRECALC" "$2" $INCLUDEPATH >> "$WORK/songdb.tsv" 2>> "$WORK/stderr"
  local RES=$?
  if [ "$RES" -gt "1" ]; then
    echo "Failed to process $2 - exit code $RES " | tee -a "$WORK/stderr"
  elif [ "$RES" -eq "0" ]; then
    local MD5=$(md5sum -b "$2" | head -c 32)
    local SUBSONGS="$(${PRECALC} "$2" subsongs 2> /dev/null)"

    local AUDIO_CHROMAPRINT
    local AUDIO_MD5
    local AUDIO_BYTES

    local SUBSONG

    for SUBSONG in $SUBSONGS; do
      local SONGLENGTH_MILLIS=$($TAC "$WORK/songdb.tsv" | grep -a -m 1 "$MD5	$SUBSONG	" | cut -f 3)
      if [ $SONGLENGTH_MILLIS -le 0 ]; then
        echo -e $MD5'\t'$SUBSONG'\t'0 >> "$WORK/audio.tsv"
        continue
      fi
      {
        local SONGLENGTH=$(( SONGLENGTH_MILLIS / 1000 + 1))
        local MAX_BYTES=$((4 * 11025 * 1200))
        local BYTES=$(( SONGLENGTH > 1200 ? MAX_BYTES : 4 * 11025 * SONGLENGTH ))
        # XXX chromaprint -channels parameter does not work on gentoo (ffmpeg issue?), use sox to force mono
        ${PLAY} 11025 "$2" $SUBSONG 2> /dev/null \
        | head -c $BYTES \
        | sox -t raw -b 16 -e signed -c 2 -r 11025 - -t raw -b 16 -e signed -c 1 -r 11025 -D - remix 1-2 \
        | tee \
          >(wc -c | xargs >&2) \
          >(echo $(md5sum | head -c 32) >&2) \
          | fpcalc -length 9999 -rate 11025 -channels 1 -format s16le -plain - 2>/dev/null
      } 2>&1 | {
        read AUDIO_MD5
        read AUDIO_BYTES
        read AUDIO_CHROMAPRINT
        echo -e $MD5'\t'$SUBSONG'\t'$AUDIO_BYTES'\t'$AUDIO_MD5'\t'$AUDIO_CHROMAPRINT >> "$WORK/audio.tsv"
      }
    done
  fi
}

mkdir -p /tmp/songdb

export -f run_uade
find -L  . -type f | sed "s/^\.\///g" | parallel --nice 20 --timeout 3600 run_uade {%} {}

cat /tmp/songdb/*/songdb.tsv | sort | uniq > /tmp/songdb/songdb.tsv
cat /tmp/songdb/*/audio.tsv | sort | uniq > /tmp/songdb/audio.tsv
cat /tmp/songdb/*/stderr > /tmp/songdb/stderr

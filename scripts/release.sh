#!/bin/sh

set -e

VERSION_OLD=$(cat VERSION)
cp VERSION-RELEASE VERSION
autoreconf -i && \
  ./configure && \
  make clean && \
  CFLAGS="-Werror" \
    make -j distcheck
echo ${VERSION_OLD} > VERSION

#!/bin/sh

set -e

VERSION_OLD=$(cat VERSION)
cp VERSION-RELEASE VERSION
autoreconf -i && \
  ./configure && \
  make clean && \
  CFLAGS="-Wall -Wpedantic -Wextra -Werror" \
  CXXFLAGS="${CFLAGS}" \
    make -j distcheck
echo ${VERSION_OLD} > VERSION

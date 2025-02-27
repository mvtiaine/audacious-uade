#!/bin/sh

set -e

VERSION_OLD=$(cat VERSION)
cp VERSION-RELEASE VERSION
autoreconf -i && \
  ./configure --enable-players=all --enable-plugin-audacious=yes --enable-plugin-deadbeef=yes && \
  make clean && \
  CFLAGS="-Wall -Wpedantic -Wextra -Werror" \
  CXXFLAGS="${CFLAGS}" \
    make -j distcheck
echo ${VERSION_OLD} > VERSION

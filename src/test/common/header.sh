#!/bin/sh

DIRNAME=$(dirname "$0")
: ${top_srcdir:="$DIRNAME/../.."}
: ${top_builddir:="$DIRNAME/../.."}

echo top_builddir=$top_builddir
echo top_srcdir=$top_srcdir

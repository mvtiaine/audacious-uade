#!/bin/sh

set -e

# Emscripten support does not actually work yet

#autoreconf -i

emconfigure ./configure
emmake make clean
WRAPPER="$(which node || echo node)" \
  emmake make -j check

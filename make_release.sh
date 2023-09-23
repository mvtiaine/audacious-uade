#!/bin/sh

autoreconf -i && ./configure && make clean && make -j distcheck

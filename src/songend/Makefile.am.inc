# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    songend/libsongend.la

songend_libsongend_la_SOURCES = \
    songend/detector.h \
    songend/detector.cc \
    songend/precalc.h \
    songend/precalc.cc

songend_libsongend_la_LIBADD = \
    player/libplayer.la

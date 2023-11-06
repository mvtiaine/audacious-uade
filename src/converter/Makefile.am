AUTOMAKE_OPTIONS = subdir-objects

noinst_LIBRARIES = libconverter.a
noinst_PROGRAMS = uade_convert

AM_CPPFLAGS = -I$(top_srcdir)/src

libconverter_a_SOURCES = \
    big_endian.h \
    converter.h \
    converter.cc \
    med/common_med.h \
    med/common_med.cc \
    med/converter_med4.cc \
    ../3rdparty/macos_endian.h \
    ../3rdparty/SimpleBinStream.h \
    ../3rdparty/proplayer.h

uade_convert_LDADD = libconverter.a
uade_convert_SOURCES = converter_main.cc

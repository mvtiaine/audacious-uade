AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libconverter.a
noinst_PROGRAMS = uade_convert

libconverter_a_SOURCES = \
    big_endian.h \
    converter.h \
    converter.cc \
    common_med.h \
    common_med.cc \
    converter_med4.cc \
    ../3rdparty/macos_endian.h \
    ../3rdparty/SimpleBinStream.h \
    ../3rdparty/proplayer.h

uade_convert_LDADD = libconverter.a
uade_convert_SOURCES = converter_main.cc

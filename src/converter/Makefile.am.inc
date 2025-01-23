# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
    converter/libconverter.la

converter_libconverter_la_SOURCES = \
    converter/converter.h \
    converter/converter.cc \
    converter/io.h \
    converter/med/common_med.h \
    converter/med/common_med.cc \
    converter/med/converter_med4.cc \
    3rdparty/SimpleBinStream.h \
    3rdparty/proplayer.h

converter/med/% : CXXFLAGS += -Wno-zero-length-array -Wno-pedantic

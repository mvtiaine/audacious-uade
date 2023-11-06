AUTOMAKE_OPTIONS = subdir-objects
noinst_LIBRARIES = libsongend.a
noinst_PROGRAMS = songend

AM_CPPFLAGS = -I$(top_srcdir)/src

libsongend_a_SOURCES = \
    songend.h \
    songend.cc \
    ../common.h

songend_SOURCES = \
    songend_main.cc

songend_DEPENDENCIES = libsongend.a
songend_LDADD = $(songend_DEPENDENCIES)


# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	songdb/libsongdb.la

songdb_libsongdb_la_SOURCES = \
    songdb/internal.h \
    songdb/songdb.h \
    songdb/songdb.cc \
    songdb/blacklist.cc \
    songdb/sources/generic.cc \
    songdb/sources/modland.cc

md5_idx_size=`cat $(top_srcdir)/conf/songdb/md5idx.tsv | wc -l | xargs`
modland_size=`cat $(top_srcdir)/conf/songdb/modland.tsv | wc -l | xargs`
amp_size=`cat $(top_srcdir)/conf/songdb/amp.tsv | wc -l | xargs`
unexotica_size=`cat $(top_srcdir)/conf/songdb/unexotica.tsv | wc -l | xargs`
demozoo_size=`cat $(top_srcdir)/conf/songdb/demozoo.tsv | wc -l | xargs`

songdb_libsongdb_la_CPPFLAGS = $(AM_CPPFLAGS) \
                               -DMD5_IDX_SIZE=$(md5_idx_size) \
                               -DMODLAND_SIZE=$(modland_size) \
                               -DAMP_SIZE=$(amp_size) \
                               -DUNEXOTICA_SIZE=$(unexotica_size) \
                               -DDEMOZOO_SIZE=$(demozoo_size)

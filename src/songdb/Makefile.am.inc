# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	songdb/libsongdb.la

songdb_libsongdb_la_SOURCES = \
    songdb/internal.h \
    songdb/songdb.h \
    songdb/songdb.cc \
    songdb/blacklist.cc

md5_idx_size=`cat $(top_srcdir)/conf/songdb/md5idx.tsv | wc -l | xargs`
songlengths_size=`cat $(top_srcdir)/conf/songdb/songlengths.tsv | wc -l | xargs`
modinfos_size=`cat $(top_srcdir)/conf/songdb/modinfos.tsv | wc -l | xargs`
combined_size=`cat $(top_srcdir)/conf/songdb/combined.tsv | wc -l | xargs`
# these are not included by default
# XXX use size 1 as zero length arrays are not allowed in c++
modland_size=1
amp_size=1
unexotica_size=1
demozoo_size=1

songdb_libsongdb_la_CPPFLAGS = $(AM_CPPFLAGS) \
                               -DMD5_IDX_SIZE=$(md5_idx_size) \
                               -DSONGLENGTHS_SIZE=$(songlengths_size) \
                               -DMODINFOS_SIZE=$(modinfos_size) \
                               -DCOMBINED_SIZE=$(combined_size) \
                               -DMODLAND_SIZE=$(modland_size) \
                               -DAMP_SIZE=$(amp_size) \
                               -DUNEXOTICA_SIZE=$(unexotica_size) \
                               -DDEMOZOO_SIZE=$(demozoo_size)

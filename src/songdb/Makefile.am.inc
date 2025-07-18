# NOT a standalone Makefile.am

noinst_LTLIBRARIES += \
	songdb/libsongdb.la

songdb_libsongdb_la_SOURCES = \
    songdb/internal.h \
    songdb/songdb.h \
    songdb/songdb.cc \
    songdb/blacklist.cc

hash_idx_size=`cat $(top_srcdir)/conf/songdb/xxh32idx.tsv | wc -l | xargs`
songlengths_size=`cat $(top_srcdir)/conf/songdb/songlengths.tsv | wc -l | xargs`
modinfos_size=`cat $(top_srcdir)/conf/songdb/modinfos.tsv | wc -l | xargs`
combined_size=`cat $(top_srcdir)/conf/songdb/combined.tsv | wc -l | xargs`

songdb_libsongdb_la_CPPFLAGS = $(AM_CPPFLAGS) \
                               -DHASH_IDX_SIZE=$(hash_idx_size) \
                               -DSONGLENGTHS_SIZE=$(songlengths_size) \
                               -DMODINFOS_SIZE=$(modinfos_size) \
                               -DCOMBINED_SIZE=$(combined_size)

ACLOCAL_AMFLAGS = -I m4

DIST_SUBDIRS = src
SUBDIRS = $(subdirs_extra) $(DIST_SUBDIRS)
EXTRA_DIST = \
	NOTICE \
	conf \
	debian \
	scripts \
	testdata \
	tools

AM_DISTCHECK_DVI_TARGET =
DISTCHECK_CONFIGURE_FLAGS = --with-audacious-plugindir="$$dc_install_base/lib/audacious"

install installdirs: SUBDIRS = src
uninstall uninstalldirs: SUBDIRS = src

dist-hook:
	mkdir -p "$(distdir)/uade"
	cd uade && git ls-files | tar Tc - | tar x -C "../$(distdir)/uade"

distclean-local:
	if test "$(srcdir)" != "."; then \
	    rm -rf uade; \
	fi
	rm -rf $(top_builddir)/build-tmp

clean-local:
	rm -rf $(top_builddir)/build-tmp

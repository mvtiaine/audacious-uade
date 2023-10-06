ACLOCAL_AMFLAGS = -I m4

SUBDIRS = $(subdirs_extra) src src/converter src/songend
DIST_SUBDIRS = src src/converter src/songend
EXTRA_DIST = default.nix conf tools NOTICE

AM_DISTCHECK_DVI_TARGET =
DISTCHECK_CONFIGURE_FLAGS = --with-audacious-plugindir="$$dc_install_base/lib/audacious"

install installdirs: SUBDIRS = src src/converter src/songend
uninstall uninstalldirs: SUBDIRS = src src/converter src/songend

dist-hook:
	mkdir -p "$(distdir)/uade"
	cd uade && git ls-files | tar Tc - | tar x -C "../$(distdir)/uade"

distclean-local:
	if test "$(srcdir)" != "."; then \
	    rm -rf uade; \
	fi

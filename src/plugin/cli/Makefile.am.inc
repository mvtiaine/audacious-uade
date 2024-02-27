# NOT a standalone Makefile.am

noinst_PROGRAMS += \
	plugin/cli/converter/converter \
	plugin/cli/player/player \
	plugin/cli/precalc/precalc \
	plugin/cli/songdb/songdb \
	plugin/cli/songend/songend

EXTRA_DIST += \
    plugin/cli/precalc/precalc.sh

plugin_cli_common_SOURCES = \
	plugin/cli/common/logger.cc

# converter
plugin_cli_converter_converter_SOURCES = \
    $(plugin_cli_common_SOURCES) \
    plugin/cli/converter/converter_main.cc

plugin_cli_converter_converter_LDADD = \
    converter/libconverter.la

# player
plugin_cli_player_player_SOURCES = \
    $(plugin_cli_common_SOURCES) \
    plugin/cli/player/player_main.cc

plugin_cli_player_player_LDADD = \
    player/libplayer.la \
    $(UADE_LIBS)

# precalc
plugin_cli_precalc_precalc_SOURCES = \
    $(plugin_cli_common_SOURCES) \
    plugin/cli/precalc/precalc_main.cc

plugin_cli_precalc_precalc_LDADD = \
    songend/libsongend.la \
    songdb/libsongdb.la \
    common/libmd5.la \
    $(UADE_LIBS)

# songdb
plugin_cli_songdb_songdb_SOURCES = \
    $(plugin_cli_common_SOURCES) \
    plugin/cli/songdb/songdb_main.cc

plugin_cli_songdb_songdb_LDADD = \
    songdb/libsongdb.la \
    common/libmd5.la

# songend
plugin_cli_songend_songend_SOURCES = \
    $(plugin_cli_common_SOURCES) \
    plugin/cli/songend/songend_main.cc

plugin_cli_songend_songend_LDADD = \
	songend/libsongend.la

plugin_cli_converter_converter_SOURCES += plugin/cli/common/compat.cc
plugin_cli_player_player_SOURCES += plugin/cli/common/compat.cc
plugin_cli_precalc_precalc_SOURCES += plugin/cli/common/compat.cc
plugin_cli_songdb_songdb_SOURCES += plugin/cli/common/compat.cc
plugin_cli_songend_songend_SOURCES += plugin/cli/common/compat.cc

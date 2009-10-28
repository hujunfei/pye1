METASOURCES = AUTO
bin_PROGRAMS = ibus-pye

ibus_pye_SOURCES = Config.cpp Engine.cpp PinyinEngine.cpp main.cpp

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@ -DPKGDATADIR=\"$(pkgdatadir)\"

# the library search path.
ibus_pye_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
ibus_pye_LDADD = $(INTLLIBS) engine/libengine.a utils/libutils.a

noinst_HEADERS = Config.h Engine.h PinyinEngine.h

SUBDIRS = include utils engine tools

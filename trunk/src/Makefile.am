METASOURCES = AUTO
bin_PROGRAMS = pye

pye_SOURCES = pye.cpp

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
pye_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
pye_LDADD = $(INTLLIBS) engine/libengine.a utils/libutils.a

SUBDIRS = include utils engine tools

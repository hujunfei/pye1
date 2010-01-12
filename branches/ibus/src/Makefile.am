METASOURCES = AUTO
libexec_PROGRAMS = ibus-pye ibus-setup-pye

ibus_pye_SOURCES = EngineConfig.cpp EngineObject.cpp HalfFullConverter.cpp PinyinEngine.cpp \
	ibus-pye.cpp
ibus_setup_pye_SOURCES = ibus-setup-pye.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
ibus_pye_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
ibus_pye_LDADD = $(INTLLIBS) engine/libengine.a
ibus_setup_pye_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
ibus_setup_pye_LDADD = $(INTLLIBS)

noinst_HEADERS = EngineConfig.h EngineObject.h HalfFullConverter.h PinyinEngine.h

SUBDIRS = engine tools

componentdir = $(datadir)/ibus/component
component_DATA = pye.xml
CLEANFILES = pye.xml
EXTRA_DIST = pye.xml.in
pye.xml: pye.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

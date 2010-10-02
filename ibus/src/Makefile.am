METASOURCES = AUTO
libexec_PROGRAMS = ibus-engine-pye ibus-setup-pye

ibus_engine_pye_SOURCES = engine.cc half_full_converter.cc ibus_engine_pye.cc \
                          parameter.cc pye_engine.cc
ibus_setup_pye_SOURCES = ibus_setup_pye.cc parameter_settings.cc

AM_CXXFLAGS = -Wall

# set the include path found by configure
ibus_engine_pye_CPPFLAGS = $(IBUS_CFLAGS) $(PYE_CFLAGS)
ibus_setup_pye_CPPFLAGS = $(IBUS_CFLAGS) $(GTK_CFLAGS)

# the library search path.
ibus_engine_pye_LDFLAGS = $(IBUS_LIBS) $(PYE_LIBS)
ibus_setup_pye_LDFLAGS = $(IBUS_LIBS) $(GTK_LIBS)

noinst_HEADERS = engine.h half_full_converter.h parameter.h parameter_settings.h \
                 pye_engine.h

componentdir = $(datadir)/ibus/component
component_DATA = pye.xml

EXTRA_DIST = pye.xml.in

CLEANFILES = pye.xml

pye.xml: pye.xml.in
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

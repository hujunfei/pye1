METASOURCES = AUTO
bin_PROGRAMS = pyeCreateMB pyeCreateUMB pyeParseUMB

pyeCreateMB_SOURCES = CreateMB.cpp pyeCreateMB.cpp
pyeCreateUMB_SOURCES = CreateUMB.cpp pyeCreateUMB.cpp
pyeParseUMB_SOURCES = ParseUMB.cpp pyeParseUMB.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
pyeCreateMB_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
pyeCreateMB_LDADD = $(INTLLIBS) ../engine/libengine.a ../utils/libutils.a
pyeCreateUMB_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
pyeCreateUMB_LDADD = $(INTLLIBS) ../engine/libengine.a ../utils/libutils.a
pyeParseUMB_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
pyeParseUMB_LDADD = $(INTLLIBS) ../engine/libengine.a ../utils/libutils.a

noinst_HEADERS = CreateMB.h CreateUMB.h ParseUMB.h

METASOURCES = AUTO
bin_PROGRAMS = pyeCreateMB

pyeCreateMB_SOURCES = CreateMB.cpp pyeCreateMB.cpp

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
pyeCreateMB_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
pyeCreateMB_LDADD = $(INTLLIBS) ../engine/libengine.a ../utils/libutils.a

noinst_HEADERS = CreateMB.h

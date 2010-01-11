METASOURCES = AUTO
bin_PROGRAMS = test

test_SOURCES = test.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
test_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@
test_LDADD = $(INTLLIBS) ../engine/libengine.a

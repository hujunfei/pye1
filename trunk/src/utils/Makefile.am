METASOURCES = AUTO
lib_LIBRARIES = libutils.a

libutils_a_SOURCES = support.cpp wrapper.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES = $(all_includes) @PACKAGE_CFLAGS@

noinst_HEADERS = output.h support.h wrapper.h

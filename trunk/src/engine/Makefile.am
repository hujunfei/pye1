METASOURCES = AUTO
lib_LIBRARIES = libengine.a

libengine_a_SOURCES = InquireSysPhrase.cpp InquireUserPhrase.cpp \
	ParseString.cpp PinyinEngine.cpp

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

noinst_HEADERS = InquireSysPhrase.h InquireUserPhrase.h ParseString.h \
	PinyinEngine.h mess.h

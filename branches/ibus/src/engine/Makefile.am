METASOURCES = AUTO
lib_LIBRARIES = libengine.a

libengine_a_SOURCES = InquireSysPhrase.cpp InquireUserPhrase.cpp \
	ParseString.cpp PhraseEngine.cpp PinyinEditor.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

noinst_HEADERS = InquireSysPhrase.h InquireUserPhrase.h ParseString.h \
	PhraseEngine.h PinyinEditor.h mess.h

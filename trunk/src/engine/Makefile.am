METASOURCES = AUTO
lib_LIBRARIES = libengine.a

libengine_a_SOURCES = DynamicPhrase.cpp InquireSysPhrase.cpp InquireUserPhrase.cpp \
	ParseString.cpp PhraseEngine.cpp PinyinEditor.cpp support.cpp wrapper.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
INCLUDES= $(all_includes) @PACKAGE_CFLAGS@

noinst_HEADERS = DynamicPhrase.h InquireSysPhrase.h InquireUserPhrase.h ParseString.h \
	PhraseEngine.h PinyinEditor.h mess.h deplib.h sys.h support.h wrapper.h

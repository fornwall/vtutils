AM_CFLAGS = -Wall -Wextra -pedantic -std=c11 -Os
AM_CXXFLAGS = -Wall -Wextra -pedantic -Wno-gnu-binary-literal -std=c++11 -Os

bin_PROGRAMS = vtshowkeys vtsize vtquery vttitle

vtquery_SOURCES = src/vtquery.cpp
vtshowkeys_SOURCES = src/vtutils.h src/vtshowkeys.cpp
vtsize_SOURCES = src/vtsize.c
vttitle_SOURCES = src/vttitle.cpp

dist_man1_MANS = man/vtshowkeys.1 man/vtsize.1 man/vttitle.1 man/vtquery.1

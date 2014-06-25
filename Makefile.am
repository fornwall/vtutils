CFLAGS = -Wall -Wextra -pedantic -Wno-gnu-binary-literal -std=c99 -Os
CXXFLAGS = -Wall -Wextra -pedantic -Wno-gnu-binary-literal -std=c++11 -Os

bin_PROGRAMS = toutf8 vtshowkeys vtsize vtcap vtquery

toutf8_SOURCES = src/toutf8.c
vtcap_SOURCES = src/vtcap.cpp
vtquery_SOURCES = src/vtquery.cpp
vtshowkeys_SOURCES = src/vtutils.h src/vtshowkeys.cpp src/musl/wcwidth.cpp
vtsize_SOURCES = src/vtsize.c

dist_man1_MANS = man/toutf8.1 man/vtshowkeys.1 man/vtsize.1


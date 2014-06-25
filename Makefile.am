CFLAGS = -Wall -Wextra -pedantic -Wno-gnu-binary-literal -std=c99 -Os
CXXFLAGS = -Wall -Wextra -pedantic -Wno-gnu-binary-literal -std=c++11 -Os

bin_PROGRAMS = toutf8 vtshowkeys vtsize vtcap vtquery unicodedata

toutf8_SOURCES = src/toutf8.c
vtcap_SOURCES = src/vtcap.cpp
vtquery_SOURCES = src/vtquery.cpp
vtshowkeys_SOURCES = src/vtutils.h src/vtshowkeys.cpp \
                     src/musl/wcwidth.cpp src/musl/nonspacing.h src/musl/wide.h src/musl/wcwidth_musl.h
vtsize_SOURCES = src/vtsize.c
unicodedata_SOURCES = src/unicodedata.cpp

dist_man1_MANS = man/toutf8.1 man/vtshowkeys.1 man/vtsize.1

# Installs the files in $PREFIX/share/$pkgname/
dist_pkgdata_DATA = data/UnicodeData.txt data/Blocks.txt
AM_CPPFLAGS = -DDATADIR=\"$(pkgdatadir)\"

RANLIB?=ranlib
CXXFLAGS+=-Wall
INSTALL?=install
MKDIRHIER?=mkdirhier

OBJECTS:=Hyphenator.o Language.o HyphenationTree.o HyphenationRule.o hyphenate.o

INSTALL_HEADERS:=Hyphenator.h Language.h HyphenationRule.h hyphenate.h

NAME:=libhyphenate
PREFIX?=$(DESTDIR)/usr

all : $(NAME).la src-all

.PHONY : src-all src-install

include $(wildcard Makefile.config)

LIBDIR?=$(PREFIX)/lib
INCLUDEDIR?=$(PREFIX)/include/libhyphenate
SHAREDIR?=$(PREFIX)/share/libhyphenate
DOCDIR?=$(PREFIX)/share/doc/libhyphenate

ifdef $(prefix)
   PREFIX:=$(prefix)
endif

CXXFLAGS+=`pkg-config --cflags glib-2.0` -g
LIBS+=`pkg-config --libs glib-2.0` 
CXXFLAGS+=-D LIBHYPHENATE_DEFAULT_PATH='"$(SHAREDIR)/patterns"'

.SECONDARY : $(OBJECTS:.o=.lo)

%.lo : %.c
	libtool --mode=compile $(CXX) $(CPPFLAGS) -c $<

%.lo : %.cpp
	libtool --mode=compile $(CXX) $(CXXFLAGS) -c $<

%.la :
	libtool --mode=link \
	 $(CXX) $(CPPFLAGS) -c -o $@ $^ -version-info $(VERSION) \
	    -rpath $(LIBDIR) $(LIBS)

libhyphenate.la : $(OBJECTS:.o=.lo)
libhyphenate.la : VERSION=2:0:1
libhyphen.la : $(OBJECTS:.o=.lo) hyphen.lo csutil.lo
libhyphen.la : VERSION=1:1:1

Hyphenator.lo : Hyphenator.cpp Language.h Hyphenator.h Makefile.config

clean : ; -rm -rf *.o *.lo *.la .libs

libhyphen-install : libhyphen.la
	$(INSTALL) -d $(DESTDIR)$(LIBDIR) $(DESTDIR)$(INCLUDEDIR)
	libtool --mode=install $(INSTALL) -m a=r libhyphen.la \
	    $(DESTDIR)$(LIBDIR)/$(NAME).la
	libtool --mode=finish $(LIBDIR)
	$(INSTALL) -m a=r hyphen.h $(DESTDIR)$(INCLUDEDIR)/..

install : all src-install
	$(INSTALL) -d $(DESTDIR)$(LIBDIR) $(DESTDIR)$(SHAREDIR) 
	$(INSTALL) -d $(DESTDIR)$(INCLUDEDIR) $(DESTDIR)$(SHAREDIR)/patterns
	$(INSTALL) -d $(DESTDIR)$(DOCDIR)
	libtool --mode=install $(INSTALL) -m a=r $(NAME).la \
	    $(DESTDIR)$(LIBDIR)/$(NAME).la
	libtool --mode=finish $(LIBDIR)
	$(INSTALL) -m a=r $(INSTALL_HEADERS) $(DESTDIR)$(INCLUDEDIR)

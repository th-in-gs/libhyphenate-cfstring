RANLIB?=ranlib
CXXFLAGS+=-fpic -O3
INSTALL?=install
MKDIRHIER?=mkdirhier

all: 
	$(MAKE) -C src

include $(wildcard src/Makefile.config)

LIBDIR?=$(PREFIX)/lib
INCLUDEDIR?=$(PREFIX)/include/libhyphenate
SHAREDIR?=$(PREFIX)/share/libhyphenate
DOCDIR?=$(PREFIX)/share/doc/libhyphenate
PATTERNDIR=$(SHAREDIR)/patterns

install: 
	$(MAKE) -C src install
	$(INSTALL) -d $(DESTDIR)$(SHAREDIR) $(DESTDIR)$(PATTERNDIR)
	$(INSTALL) -d $(DESTDIR)$(DOCDIR)
	$(INSTALL) $(wildcard share/patterns/*) $(DESTDIR)$(PATTERNDIR) 
	$(INSTALL) README* $(DESTDIR)$(DOCDIR)
	$(INSTALL) doc/* $(DESTDIR)$(DOCDIR)

clean:
	-rm -f src/Makefile.config
	$(MAKE) -C src clean

distclean: clean

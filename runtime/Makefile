# vi: set ts=8 shiftwidth=8 noexpandtab:
#
# VNPForth - Compiled native Forth for x86 Linux
# Copyright (C) 2005-2013  Simon Baldwin (simon_baldwin@yahoo.com)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

include ../Makefile.inc

# Define a standard method of compiling .ft files.  Since the modules in
# this directory generally go into a shared library, use -fPIC.  Some
# modules containing CODE words require this, in any case.
FORTHC		= ../compiler/forthc
FORTHFLAGS	= -g -fPIC -O -P -S -w
.SUFFIXES:
.SUFFIXES:	.ft .o
.ft.o:
	$(FORTHC) $(FORTHFLAGS) $<

OBJECTS = _dlmain.o cells.o stack.o dstack.o rstack.o memory.o logic.o \
	  maths.o compare.o coreio.o io.o floatio.o loop.o strings.o \
	  except.o tools.o alloc.o compat.o environ.o float.o procenv.o \
	  cclink.o syscall.o syscalls.o extsyscl.o errno.o perror.o

# List of source files built into the man page
MDOCSOURCES = _dlmain.ft cells.ft stack.ft dstack.ft rstack.ft memory.ft \
	      logic.ft maths.ft compare.ft coreio.ft io.ft floatio.ft \
	      loop.ft strings.ft except.ft tools.ft alloc.ft compat.ft \
	      environ.ft float.ft procenv.ft cclink.ft forthrt1.ft

SDOCSOURCES =	syscall.ft perror.ft syscalls.ft extsyscl.ft
EDOCSOURCES =	errno.ft
DOCSOURCES  =	$(MDOCSOURCES) $(SDOCSOURCES) $(EDOCSOURCES)

# List of source files built into the C header file
HSOURCES    =	$(DOCSOURCES)

default: all

# Build the runtime .o, and both the static and shared libraries
_dlmain.o: _dlmain.ft $(FORTHC)
	$(AWK) -vTIMESTAMP="`date '+%s'`"				\
		'{ gsub (/__TIMESTAMP__/, TIMESTAMP); print $$0 }'	\
		_dlmain.ft >_dlmain.pp
	$(FORTHC) $(FORTHFLAGS) _dlmain.pp
	rm -f _dlmain.pp

all: forthrt1.o libforth.a libforth.so libforth.3 forthlib.h

libforth.a: $(OBJECTS) $(FORTHC)
	rm -f libforth.a; ar -cr libforth.a $(OBJECTS)
	$(RANLIB) libforth.a

libforth.so: $(OBJECTS) $(FORTHC)
	$(CC) -m32 -nostdlib -shared -o libforth.so $(OBJECTS)

# Add the documentation to the man page
libforth.3: libdoc.awk libtabls.awk include.awk libforth.3.m4 $(DOCSOURCES)
	$(AWK) -f libdoc.awk $(DOCSOURCES) \
		| $(AWK) -f libtabls.awk >forthvariables
	$(AWK) -f libdoc.awk $(MDOCSOURCES) \
		| $(AWK) -vmode=word -f libtabls.awk >forthwords
	$(AWK) -f libdoc.awk $(SDOCSOURCES) $(EDOCSOURCES) \
		| $(AWK) -vmode=word -f libtabls.awk >extraforthwords
	$(AWK) -f include.awk <libforth.3.m4 >libforth.3
	rm -f forthwords extraforthwords forthvariables

# Build the forthlib.h C header from the sources
forthlib.h: libdoc.awk libnames.awk libheadr.awk libhpatc.awk include.awk \
	    forthlib.h.m4 $(HSOURCES)
	$(AWK) -f libdoc.awk $(HSOURCES) \
		| $(AWK) -f libnames.awk > fnames
	cat fnames | xargs $(FORTHC) -M -- > mnames
	$(AWK) -f libdoc.awk $(HSOURCES) \
		| $(AWK) -vfnames='fnames' -vmnames='mnames' -f libheadr.awk \
		| $(AWK) -vsymbol='v4_c_push' -vargs='(int)' -f libhpatc.awk \
		| $(AWK) -vsymbol='v4_c_pop'  -vtype='int'   -f libhpatc.awk \
		>cheader
	$(AWK) -f include.awk <forthlib.h.m4 >forthlib.h
	cat mnames | xargs $(FORTHC) -X -- > dnames
	diff -q fnames dnames
	rm -f cheader fnames mnames dnames

install: all
	$(INSTALL) -d $(includedir) $(libdir) $(mandir)/man3
	$(INSTALL_DATA) forthlib.h $(includedir)/forthlib.h
	$(INSTALL_DATA) libforth.a $(libdir)/libforth.a
	$(INSTALL_PROGRAM) libforth.so $(libdir)/libforth.so
	$(INSTALL_DATA) forthrt1.o $(libdir)/forthrt1.o
	$(INSTALL_DATA) libforth.3 $(mandir)/man3/libforth.3
	$(GZIP) -f -9 $(mandir)/man3/libforth.3

install-strip: all
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall:
	rm -f $(includedir)/forthlib.h
	rm -f $(libdir)/libforth.a
	rm -f $(libdir)/libforth.so
	rm -f $(libdir)/forthrt1.o
	rm -f $(mandir)/man3/libforth.3 $(mandir)/man3/libforth.3.gz

clean:
	$(MAKE) -C testsuite clean
	rm -f forthrt1.o libforth.a libforth.so *.s *.p *.o
	rm -f libforth.3 forthlib.h
	rm -f forthwords extraforthwords forthvariables
	rm -f cheader fnames mnames dnames
	rm -f _dlmain.pp
	rm -f core

check: all
	$(MAKE) -C testsuite check
clobber: clean
	$(MAKE) -C testsuite clobber
distclean: clean
	$(MAKE) -C testsuite distclean
maintainer-clean: distclean
	$(MAKE) -C testsuite maintainer-clean
dist:

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

include ../../Makefile.inc

# Define a standard method of compiling .ft files
FORTHC		= ../../compiler/forthc
FORTHFLAGS	= -g -O -P -S
.SUFFIXES:
.SUFFIXES:	.ft .o
.ft.o:
	$(FORTHC) $(FORTHFLAGS) $<

# Linker flags, and library dependencies
LFLAGS	= -L..
LIBS	= -lforth
FORTHRT	= -nostdlib -static ../forthrt1.o

LDEPS	= ../libforth.a ../forthrt1.o
LDEPD	= ../libforth.so

default: check

all:	testcore_s testcore_d testenv_s testenv_d

testcore_s: tester.o core.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o testcore_s tester.o core.o $(FORTHRT) $(LFLAGS) $(LIBS)

testcore_d: tester.o core.o $(LDEPD) $(FORTHC)
	$(CC) -m32 -g -o testcore_d tester.o core.o $(LFLAGS) $(LIBS)

testenv_s: environ.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o testenv_s environ.o $(FORTHRT) $(LFLAGS) $(LIBS)

testenv_d: environ.o $(LDEPD) $(FORTHC)
	$(CC) -m32 -g -o testenv_d environ.o $(LFLAGS) $(LIBS)

clean:
	rm -f testcore_s testcore_d testenv_s testenv_d
	rm -f core *.o *.s *.p

RUNTIME = LD_LIBRARY_PATH=..
check: all
	@echo "Test core stdin static" | $(RUNTIME) ./testcore_s
	@$(RUNTIME) ./testenv_s
	@echo "Test core stdin dynamic" | $(RUNTIME) ./testcore_d
	@$(RUNTIME) ./testenv_d

install:
install-strip:
uninstall:
clobber: clean
distclean: clean
maintainer-clean: distclean
dist:

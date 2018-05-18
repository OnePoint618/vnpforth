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

CFLAGS		= $(CEXTRA) $(DEBUG)

# Define a standard method of compiling .ft files.  Reconstruct the
# standard method of compiling .c files.
FORTHC		= ../compiler/forthc
FORTHFLAGS	= -g -O -P -S
.SUFFIXES:
.SUFFIXES:	.ft .c .o
.ft.o:
	$(FORTHC) $(FORTHFLAGS) $<
.c.o:
	$(CC) -I ../runtime -c $(CFLAGS) -m32 $<

# Linker flags, and library dependencies
LFLAGS	= -L../runtime
LIBS	= -lforth
FORTHRT	= -nostdlib -static ../runtime/forthrt1.o

LDEPS	= ../runtime/libforth.a ../runtime/forthrt1.o
LDEPD	= ../runtime/libforth.so

default: check

# Build all demos.  For demonstration purposes, we'll create static binaries
# where we can.
all: sleep printenv logname shell mv ln rm cat reimann features

sleep: sleep.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o sleep sleep.o $(FORTHRT) $(LFLAGS) $(LIBS)

printenv: printenv.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o printenv printenv.o $(FORTHRT) $(LFLAGS) $(LIBS)

logname: logname.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o logname logname.o $(LFLAGS) $(LIBS)

shell: shell.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o shell shell.o $(FORTHRT) $(LFLAGS) $(LIBS)

mv: mv.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o mv mv.o $(FORTHRT) $(LFLAGS) $(LIBS)

ln: mv
	rm -f ln; /bin/ln mv ln

rm: rm.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o rm rm.o $(FORTHRT) $(LFLAGS) $(LIBS)

cat: cat.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o cat cat.o $(FORTHRT) $(LFLAGS) $(LIBS)

reimann: reimann_.o reimann.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -I../runtime \
	      -g -o reimann reimann.o reimann_.o $(LFLAGS) $(LIBS)

features: features.o $(LDEPS) $(FORTHC)
	$(CC) -m32 -g -o features features.o $(FORTHRT) $(LFLAGS) $(LIBS)

clean:
	rm -f sleep printenv logname shell mv ln rm cat reimann features
	rm -f foo bar
	rm -f core *.o *.s *.p

RUNTIME = LD_LIBRARY_PATH=../runtime
check: all
	./sleep 1
	./printenv PATH
	$(RUNTIME) ./logname
	echo -e "/bin/uname -s\nexit" | ./shell && echo ""
	date >foo; ./mv foo bar
	./ln bar foo
	./cat bar foo
	./rm bar foo
	$(RUNTIME) ./reimann
	./features

install:
install-strip:
uninstall:
clobber: clean
distclean: clean
maintainer-clean: distclean
dist:

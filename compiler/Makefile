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

CXXFLAGS= -D_ISOC99_SOURCE -Wall -Wextra -pedantic $(CXXEXTRA) $(DEBUG)
LDFLAGS = $(LDEXTRA) $(DEBUG)
OBJECTS	= cmdline.o data.o dattable.o symbol.o symtable.o \
	  opcode.o optable.o mangler.o srcfile.o parser.o program.o \
	  compiler.o codegen.o optimize.o forth.o forth.tab.o

default: all
all: forthc

forthc: $(OBJECTS)
	$(CXX) $(LDFLAGS) -static -o forthc $(OBJECTS) $(LEXLIB)

cmdline.o:   cmdline.cc options.h cmdline.h util.h
codegen.o:   codegen.cc data.h dattable.h opcode.h operand.h register.h \
             stack.h symbol.h util.h optable.h options.h program.h symtable.h
compiler.o:  compiler.cc cmdline.h options.h mangler.h program.h \
             dattable.h optable.h symtable.h
data.o:      data.cc data.h dattable.h util.h
dattable.o:  dattable.cc data.h dattable.h
mangler.o:   mangler.cc mangler.h util.h
opcode.o:    opcode.cc data.h opcode.h operand.h register.h stack.h symbol.h \
             util.h optable.h
optable.o:   optable.cc opcode.h operand.h register.h stack.h symbol.h \
             util.h optable.h symtable.h
optimize.o:  optimize.cc mangler.h opcode.h operand.h register.h stack.h \
             symbol.h util.h optable.h
parser.o:    parser.cc data.h dattable.h opcode.h operand.h register.h \
             stack.h symbol.h util.h optable.h parser.h symtable.h
program.o:   program.cc cmdline.h options.h dattable.h optable.h parser.h \
             operand.h program.h symtable.h srcfile.h
srcfile.o:   srcfile.cc srcfile.h
symbol.o:    symbol.cc mangler.h symbol.h util.h symtable.h
symtable.o:  symtable.cc mangler.h symbol.h util.h symtable.h

forth.tab.cc: forth.output
forth.tab.hh: forth.output

forth.output: forth.y operand.h parser.h
	$(YACC) -v -d -o forth.tab.cc forth.y

forth.tab.o: forth.tab.cc parser.h operand.h

forth.cc: forth.l operand.h parser.h forth.tab.hh
	$(LEX) -i -o forth.cc forth.l

forth.o: forth.cc parser.h operand.h forth.tab.hh
	$(CXX) $(CXXFLAGS) -Wno-unused-function -c forth.cc

install: all
	$(INSTALL) -d $(bindir) $(mandir)/man1
	$(INSTALL_PROGRAM) forthc $(bindir)/forthc
	$(INSTALL_DATA) forthc.1 $(mandir)/man1/forthc.1
	$(GZIP) -f -9 $(mandir)/man1/forthc.1

install-strip: all
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall:
	rm -f $(bindir)/forthc
	rm -f $(mandir)/man1/forthc.1 $(mandir)/man1/forthc.1.gz

clean:
	$(MAKE) -C testsuite clean
	rm -f *.o forthc
	rm -f core gmon.out

dist: forth.tab.cc forth.tab.hh forth.output forth.cc

check: all
	$(MAKE) -C testsuite check
clobber: clean
	$(MAKE) -C testsuite clobber
distclean: clean
	$(MAKE) -C testsuite distclean
maintainer-clean: clean
	rm -f forth.tab.cc forth.tab.hh forth.output forth.cc
	$(MAKE) -C testsuite maintainer-clean

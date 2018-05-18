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

COMPILER_TESTS	= testsuite.ft
SPLIT_PREFIX	= compile_test_
REPORT_FILE	= testsuite.log
TESTER		= tester

default: check

check:
	$(AWK) -vprefix="$(SPLIT_PREFIX)" -f split.awk < $(COMPILER_TESTS)
	rm -f core $(REPORT_FILE)
	${SHELL} $(TESTER) $(REPORT_FILE) $(SPLIT_PREFIX)??

clean:
	rm -f $(REPORT_FILE) $(SPLIT_PREFIX)??
	rm -f core *.o *.p *.s

all:
install:
install-strip:
uninstall:
clobber: clean
distclean: clean
maintainer-clean: distclean
dist:

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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
#

# Normal descriptive preamble.
Summary:	VNPForth compiled native Forth for x86 Linux
Name:		vnpforth
Version:	1.5
Release:	1
Group:		Development/Languages
Copyright:	GPL
Packager:	Simon Baldwin <simon_baldwin@yahoo.com>
URL:		http://www.geocities.com/simon_baldwin/
Source:		ftp://metalab.unc.edu/pub/Linux/devel/lang/forth/vnpforth-1.5.tgz
BuildRoot:	/tmp/vnpforth-1.5

%description
VNPForth is an implementation of the Forth programming language for IA32
Linux systems.  It contains a Forth compiler capable of turning Forth programs
into standard object (.o) files, and a runtime library that offers most of
the expected ANS Forth core words.

# Straightforward preparation for building.
%prep
%setup

# To build, just run make.
%build
make

# Tweaked install.  Here we set "prefix" to the build root, suffixed with
# "/usr".  So, unlike our natural locations under "/usr/local", we install
# the RPM packaged version in "/usr", leaving "/usr/local" free for, well,
# the usual "/usr/local" stuff.
%install
make prefix="$RPM_BUILD_ROOT/usr" install

# Clean up our build root.
%clean
rm -rf $RPM_BUILD_ROOT

# List of packaged files.
%files
/usr/bin/forthc

/usr/include/forthlib.h

/usr/lib/libforth.a
/usr/lib/libforth.so
/usr/lib/forthrt1.o

/usr/man/man1/forthc.1.gz

/usr/man/man3/libforth.3.gz

%doc COPYING
%doc README
%doc demos/features.ft

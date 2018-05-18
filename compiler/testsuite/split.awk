#!/bin/awk -f
# vi: set ts=2 shiftwidth=2 expandtab:
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

# Extract non-comment lines and write each to a separate file.

# Initialize output file count and chars lookup.
BEGIN {
  count = 0
  chars = "abcdefghijklmnopqrstuvwxyz"
}

# Write each test line as a single file, prefix_[a-z][a-z].
$0 !~ /^$/ && $1 != "\\" {
  a = substr (chars, count / 26 + 1, 1)
  b = substr (chars, count % 26 + 1, 1)

  print $0 >prefix a b
  count++
}

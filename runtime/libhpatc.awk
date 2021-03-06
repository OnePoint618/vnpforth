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

# Patch the C header file for variables and words to force a given
# signature for a symbol.

# Check usage.
BEGIN {
  if (!symbol)
    {
      print ARGV[0] ": need definition for symbol" > "/dev/stderr"
      exit (1)
    }
}

# Alter the requested symbol for the corrected type and argument.
$1 == "extern" && $3 == symbol {
  newtype = type ? type : $2
  newargs = args ? args ";" : $4
  printf "extern %s %s %s\n", newtype, symbol, newargs
  next
}

# Just pass everything else through.
{
  print $0
}

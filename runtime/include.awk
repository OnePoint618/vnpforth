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

# Replicate just the include() function of m4.  Avoids the need for a
# full m4 binary.

# Catch 'include(...)' special lines
/^[ \t]*include\([^\)]*\)[ \t]*$/ {

  # Find the name of the file being included.
  file = $0
  sub(/^[ \t]*include\(/, "", file)
  sub(/\)[ \t]*$/, "", file)

  # Read in each line of the file, and print it out.
  while ((status=getline line <file) > 0)
    print line
  close (file)

  # Check for read or general getline error.
  if (status != 0)
    {
      printf ("Error reading file %s: %s\n", file, ERRNO) | "cat 1>&2"
      exit 1
    }
  next
}

# Pass unchanged all lines that don't look like 'include(...)'
{
  print $0
}

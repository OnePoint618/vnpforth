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

# Produce documentation suitable for processing into man pages from a
# Forth source file.

# Initialize the states, arrays, and indices.
BEGIN {
  VARIABLE_DECLARATION    = "variable"
  CREATE_DECLARATION      = "create"
  WORD_DECLARATION        = ":"
  CODEWORD_DECLARATION    = "code"

  VARIABLE_TAG            = "V"
  VARIABLE_SIZE_TAG       = "B"
  WORD_TAG                = "W"
  STACKDOCUMENTATION_TAG  = "S"
  DOCUMENTATION_TAG       = "D"
  END_TAG                 = "E"
}

# Ignore all blank lines in the file
/^[ \t]*$/ {
  next
}

# Handle every other line in the file individually.
{
  # Look for VARIABLE/CREATE declarations, and WORDs or CODEWORDs.  Items
  # can be hidden from doc by prefixing them with a "(...)" comment.
  if ($1 == VARIABLE_DECLARATION || $1 == CREATE_DECLARATION \
      || $1 == WORD_DECLARATION || $1 == CODEWORD_DECLARATION)
    {
      # Output the variable's name or word defined.  This is the token after
      # "VARIABLE", "CREATE", ":", or "CODEWORD".
      if ($1 == VARIABLE_DECLARATION || $1 == CREATE_DECLARATION)
        printf ("%s %s\n", VARIABLE_TAG, $2)
      else
        printf ("%s %s\n", WORD_TAG, $2)

      # If a variable, output size information.
      if ($1 == CREATE_DECLARATION)
        {
          if ($4 == "cells" || $4 == "chars")
            printf ("%s %s %s\n", VARIABLE_SIZE_TAG, $3, $4)
          else
            printf ("%s %s chars\n", VARIABLE_SIZE_TAG, $3)
        }
      else
        if ($1 == VARIABLE_DECLARATION)
          printf ("%s 1 cell\n", VARIABLE_SIZE_TAG)

      # Find any description, and output that too.  The description we are
      # interested in is inside (...)'s, and there may be more than one (...)
      # element to handle on the line.
      if (index ($0, "( ") != 0)
        {
          # Remove the first '( ' and the last ' )' parts.
          sub (/^[^\(]*\([ \t]/, "", $0)
          sub (/[ \t]\)[^\)]*$/, "", $0)

          # Split the remaining string on the ' ) ( ' pattern.
          count = split ($0, doclines, \
                         /[ \t]\)[ \t]+\([ \t]/ );

          # Print out the doclines array, one line per element, up to, but
          # excluding, the last line.  These are the stack effects
          # documentation.
          for (i = 1; i <= count - 1; i++)
            printf ("%s %s\n", STACKDOCUMENTATION_TAG, doclines[i]);

          # Look for evidence of stack effects documentation on the last
          # documentation element.
          line = doclines[count]
          if (match (line, /^[^,]*--[^,]*,[ \t]*/) > 0)
            {
              sline = substr (line, RSTART, RLENGTH)
              line  = substr (line, RLENGTH + 1)
              sub (/[ \t]*,[ \t]$/, "", sline)
              printf ("%s %s\n", STACKDOCUMENTATION_TAG, sline)
            }
          printf ("%s %s\n", DOCUMENTATION_TAG, line)
        }

      # End this documentation section.
      printf ("%s\n", END_TAG);
    }
}

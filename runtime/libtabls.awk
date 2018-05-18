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

# Output variables and words documentation from processed Forth source.

# Initialize the states, arrays, and indices.
BEGIN {
  IDLE                    = 0
  NAME                    = 1
  DESCRIPTION             = 2
  state                   = IDLE

  VARIABLE_TAG            = "V"
  WORD_TAG                = "W"
  STACKDOCUMENTATION_TAG  = "S"
  DOCUMENTATION_TAG       = "D"
  END_TAG                 = "E"

  # Set up for words or variables, based on the mode variable.
  if (mode == "word")
    {
      table_header = "Word"
      trigger_tag = WORD_TAG
    }
  else
    {
      table_header = "Variable"
      trigger_tag = VARIABLE_TAG
    }

  # Start a tabular output; two columns.
  printf (".TS\n");
  printf ("l l.\n");
  printf ("%s\tDescription\n", table_header);
  printf (".BR\n");
}

# Find a trigger tag; if found, note the name and continue.
$1 == trigger_tag {
  name = $2
  state = NAME
  next
}

# Handle each description line found for this element.
$1 == STACKDOCUMENTATION_TAG || $1 == DOCUMENTATION_TAG {
  if (state != IDLE)
    {
      sub (/^. */, "")
      gsub (/\t/, " ")

      # If the first line, output a full table entry, else omit name.
      if (state == NAME)
        printf ("\\&%s\t%s\n", name, $0)
      else
        printf ("\t%s\n", $0)
      state = DESCRIPTION
    }
  next
}

# At the end tag, just clean up.
$1 == END_TAG {
  state = IDLE
  next
}

# At end of output, terminate the table.
END {
  printf (".TE\n");
}

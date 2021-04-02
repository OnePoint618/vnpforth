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

# Output a C header file for variables and words from processed Forth source.

# Initialize the states, arrays, and indices.
BEGIN {
  IDLE                    = 0
  VARIABLE_NAME           = 1
  VARIABLE_SIZE           = 2
  WORD_NAME               = 3
  VARIABLE_DESCRIPTION    = 4
  WORD_DESCRIPTION        = 5
  state                   = IDLE

  VARIABLE_TAG            = "V"
  VARIABLE_SIZE_TAG       = "B"
  WORD_TAG                = "W"
  STACKDOCUMENTATION_TAG  = "S"
  DOCUMENTATION_TAG       = "D"
  END_TAG                 = "E"

  C_INTARRAY              = 0
  C_CHARARRAY             = 1

  # Read in each Forth name.
  while (getline fname <fnames != 0)
    {
      # Read the corresponding mangled name.
      if (getline mname <mnames <= 0)
        {
          print ARGV[0] ": error/eof on mnames" > "/dev/stderr"
          exit (1)
        }

      # Add the name to a lookup array of mangled names.
      mangled_names[fname] = mname
    }

  # Check we reached the precise end of the mangled names.
  if (getline mname <mnames != 0)
    {
      print ARGV[0] ": mismatch on mnames" > "/dev/stderr"
      exit (1)
    }

  # Close opened names files.
  close (fnames)
  close (mnames)
}

# Find either a variable or a word tag, and note the mangled name.
$1 == VARIABLE_TAG || $1 == WORD_TAG {

  # Save the Forth name of this item, and lookup the mangled name.
  forth_name = $2
  name = mangled_names[forth_name]

  # Initialize commentary to an empty string.
  commentary = ""

  # Specifically for a variable tag, note the length of the data.
  if ($1 == VARIABLE_TAG)
    {
      variable_type = C_INTARRAY
      variable_len  = 1
      state = VARIABLE_NAME
    }
  else
    state = WORD_NAME
  next
}

# Handle variable size descriptor records.
$1 == VARIABLE_SIZE_TAG {
  if (state == VARIABLE_NAME)
    {
      if ($3 == "cell" || $3 == "cells")
        variable_type = C_INTARRAY
      else
        variable_type = C_CHARARRAY
      variable_len = $2
    }
}

# Catch documentation records, for commentary. */
$1 == STACKDOCUMENTATION_TAG {
  if (state != IDLE)
    {
      sub(/^. /, "", $0)
      commentary = commentary " (" $0 ")"
      next
    }
}
$1 == DOCUMENTATION_TAG {
  if (state != IDLE)
    {
      sub(/^. /, "", $0)
      commentary = commentary " " $0
      next
    }
}

# At the end tag, output the header information, and clean up.
$1 == END_TAG {

  # Massage commentary and forth name, especially to replace /* and */
  sub (/^[ \t]*/, "", commentary)
  sub (/[ \t]*$/, "", commentary)
  gsub (/\/\*/, "/ *", commentary)
  gsub (/\*\//, "* /", commentary)
  gsub (/\/\*/, "/ *", forth_name)
  gsub (/\*\//, "* /", forth_name)

  # Wrap the definition in a #ifndef to allow each to be disabled.
  printf ("#ifndef NO_VNPFORTH_%s\n", name);

  # Print a brief description of the item, including non-mangled name.
  printf ("/* %s", forth_name)
  if (commentary != "")
    printf (": %s", commentary)
  printf (" */\n")

  # Print the actual declaration.
  if (state == VARIABLE_NAME)
    {
      if (variable_type == C_INTARRAY)
        {
          if (variable_len == 1)
            printf ("extern int %s;", name)
          else
            printf ("extern int %s[%d];", name, variable_len)
        }
      else
        {
          if (variable_type == C_CHARARRAY)
            printf ("extern char %s[%d];", name, variable_len)
        }
    }
  else
    {
      if (state == WORD_NAME)
        printf ("extern void %s (void);", name)
    }

  # End the #ifdef wrapping, and clean up.
  printf ("\n#endif\n\n");
  state = IDLE
  next
}

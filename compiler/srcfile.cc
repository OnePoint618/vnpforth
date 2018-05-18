// vi: set ts=2 shiftwidth=2 expandtab:
//
// VNPForth - Compiled native Forth for x86 Linux
// Copyright (C) 2005-2013  Simon Baldwin (simon_baldwin@yahoo.com)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <cerrno>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "srcfile.h"

// Destructor, closes any currently open stream.
SourceFile::~SourceFile ()
{
  close ();
}

// File opening and closing.
bool
SourceFile::open ()
{
  struct stat statbuf;
  if (stat (path_.c_str (), &statbuf) == -1)
    return false;

  if (!S_ISREG (statbuf.st_mode))
    {
      errno = ENOENT;
      return false;
    }

  close ();
  stream_ = fopen (path_.c_str (), "r");
  return !!stream_;
}

void
SourceFile::close ()
{
  if (stream_)
    {
      fclose (stream_);
      stream_ = 0;
    }
}

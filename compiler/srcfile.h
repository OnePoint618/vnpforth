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

#ifndef VNPFORTH_SOURCEFILE_H
#define VNPFORTH_SOURCEFILE_H

#include <cstdio>
#include <string>

// Encapsulation of stdio source file.
class SourceFile
{
public:
  inline
  SourceFile (const std::string & path)
    : path_ (path), stream_ (0) { }

  ~SourceFile ();
  bool open ();
  void close ();

  inline std::FILE *
  get_stream () const
  {
    return stream_;
  }

private:
  SourceFile (const SourceFile & source);
  SourceFile & operator= (const SourceFile & source);

  const std::string path_;
  std::FILE * stream_;
};

#endif

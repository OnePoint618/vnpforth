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

#ifndef VNPFORTH_UTIL_H
#define VNPFORTH_UTIL_H

#include <cctype>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

namespace {

// General purpose string converter.
template <class C>
  const std::string
  to_string (C x)
  {
    std::ostringstream outs;
    outs << x;
    return outs.str ();
  }

// Convert string to lowercase.
std::string
to_lower (const std::string & str)
{
  std::string ss = str;

  std::transform (ss.begin (), ss.end (), ss.begin (), std::ptr_fun (tolower));
  return ss;
}

// Trim leading and trailing string whitespace.
std::string
trim (const std::string & str)
{
  std::string ss = str;

  ss.erase (ss.find_last_not_of (" \t\r\n") + 1);
  size_t first = ss.find_first_not_of (" \t\r\n");
  if (first != ss.npos)
    ss = ss.substr (first);

  return ss;
}

} // namespace

#endif

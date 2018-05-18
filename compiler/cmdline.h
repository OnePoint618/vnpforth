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

#ifndef VNPFORTH_COMMANDLINE_H
#define VNPFORTH_COMMANDLINE_H

#include <set>
#include <string>
#include <vector>

#include "options.h"

// Command line options and arguments.
class CommandLine
{
public:
  CommandLine (int argc, char * const * argv);

  inline Options
  get_options () const
  {
    return options_;
  }

  inline std::string
  get_program_name () const
  {
    return program_name_;
  }

  inline std::vector<std::string>
  get_arguments () const
  {
    return arguments_;
  }

  inline const std::set<std::string> *
  get_definitions () const
  {
    return &definitions_;
  }

private:
  Options options_;

  std::string program_name_;
  std::vector<std::string> arguments_;
  std::set<std::string> definitions_;

  void usage ();
  void version ();
  void help ();
};

#endif

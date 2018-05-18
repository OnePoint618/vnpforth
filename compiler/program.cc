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
#include <cstring>
#include <ctime>
#include <iostream>
#include <string>

#include "cmdline.h"
#include "dattable.h"
#include "optable.h"
#include "options.h"
#include "parser.h"
#include "program.h"
#include "srcfile.h"
#include "symtable.h"

// Parse a source file into the program.
bool
Program::build (const std::string & source_path,
                const CommandLine & commandline)
{
  SourceFile source (source_path);
  if (!source.open ())
    {
      std::cerr << commandline.get_program_name () << ": "
                << source_path << ": " << strerror (errno) << std::endl;
      return false;
    }

  source_path_ = source_path;
  options_ = commandline.get_options ();

  datatable_.clear ();
  symtable_.clear ();
  optable_.clear ();

  Parser parser;
  const int status = parser.parse (source_path, source.get_stream (),
                                   &datatable_, &symtable_, &optable_,
                                   commandline.get_definitions (),
                                   options_.trace_parser ());

  optable_.unreachable_check (source_path);

  if (status)
    optable_.synthesize_main (symtable_);

  if (status && options_.optimize_code ())
    optable_.optimize_code (source_path);

  return status;
}

// Front end to assembly code generation.
void
Program::generate (std::ostream & outs) const
{
  generate_preamble (outs);
  datatable_.generate (outs);
  symtable_.generate (outs, options_);
  optable_.generate (outs, options_);
  generate_postamble (outs);
}

// Pretty-print function to create intermediate store listing.
void
Program::create_listing (std::ostream & outs) const
{
  const size_t wordlen = sizeof (void*);
  outs << "VNPForth 1.5 [build "
       << (wordlen == 8 ? "x86_64" : (wordlen == 4 ? "x86" : "Unknown"));

  std::tm tm;
  strptime (__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &tm);
  outs << '/' << mktime (&tm) << ']' << std::endl;

  outs << "Source: " << source_path_ << std::endl;
  std::time_t now;
  std::time (&now);
  outs << "Compiled: " << asctime (localtime (&now));
  outs << "Flags:"
       << (options_.include_debugging () ? " debug" : "")
       << (options_.include_profiling () ? " profile" : "")
       << (options_.weak_functions () ? " weak" : "")
       << (options_.position_independent () ? " PIC" : "")
       << (options_.optimize_code () ? " optimize" : "")
       << (options_.save_intermediate () ? " intermediate" : "")
       << (options_.save_assembly () ? " assembly" : "")
       << std::endl << std::endl;

  datatable_.create_listing (outs);
  symtable_.create_listing (outs);
  optable_.create_listing (outs);
}

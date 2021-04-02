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

#include <cstdlib>
#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <unistd.h>

#include "options.h"
#include "cmdline.h"
#include "util.h"

// Constructor parses and builds options flags and argument vector.  There
// must be at least one argument after flags, and a few combinations of
// flags are invalid.  Exits on invalid command options or arguments.
CommandLine::CommandLine (int argc, char * const * argv)
{
  program_name_ = argv[0];
  const std::string::size_type p = program_name_.find_last_of ('/');
  if (p != program_name_.npos)
    program_name_.erase (0, p + 1);

  if (argc == 2 && std::string (argv[1]) == "--help")
    {
      help ();
      std::exit (EXIT_SUCCESS);
    }

  int c;
  while ((c = getopt (argc, argv, ":gp::f:wOPSsMXD:U:#vh")) != -1)
    {
      switch (c)
        {
        case 'g':
          options_.debugging_flag_ = true;
          break;
        case 'p':
          if (optarg && std::string (optarg) != "g")
            {
              std::cerr << program_name_ << ": invalid option -- -p"
                        << optarg << std::endl;
              usage ();
              std::exit (EXIT_FAILURE);
            }
          options_.profiling_flag_ = true;
          break;
        case 'f':
          if (std::string (optarg) != "PIC" && std::string (optarg) != "pic")
            {
              std::cerr << program_name_ << ": invalid option -- -f"
                        << optarg << std::endl;
              usage ();
              std::exit (EXIT_FAILURE);
            }
          options_.PIC_flag_ = true;
          break;
        case 'w':
          options_.weak_flag_ = true;
          break;
        case 'O':
          options_.optimize_flag_ = true;
          break;
        case 'P':
          options_.intermediate_flag_ = true;
          break;
        case 's':
        case 'S':
          options_.assembly_flag_ = true;
          break;
        case 'M':
          options_.mangle_flag_ = true;
          break;
        case 'X':
          options_.demangle_flag_ = true;
          break;
        case 'D':
          definitions_.insert (to_lower (optarg));
          break;
        case 'U':
          definitions_.erase (to_lower (optarg));
          break;
        case '#':
          options_.trace_parser_flag_ = true;
          break;
        case 'v':
          version ();
          std::exit (EXIT_SUCCESS);
        case 'h':
          help ();
          std::exit (EXIT_SUCCESS);
        case ':':
        case '?':
          std::cerr << program_name_ << ": invalid option -- "
                    << static_cast<char>(optopt) << std::endl;
          usage ();
          std::exit (EXIT_FAILURE);
        default:
          usage ();
          std::exit (EXIT_FAILURE);
        }
    }

  if (optind >= argc)
    {
      std::cerr << program_name_ << ": no "
          << (options_.mangle_flag_
              || options_.demangle_flag_ ? "ids" : "input files") << std::endl;
      usage ();
      std::exit (EXIT_FAILURE);
    }

  if (options_.debugging_flag_ && options_.profiling_flag_)
    {
      std::cerr << program_name_
          << ": -g flag overrides -p flag; -p ignored" << std::endl;
      options_.profiling_flag_ = false;
    }

  if ((options_.mangle_flag_ || options_.demangle_flag_)
      && (options_.debugging_flag_ || options_.profiling_flag_
           || options_.PIC_flag_ || options_.optimize_flag_
           || options_.intermediate_flag_ || options_.assembly_flag_
           || !definitions_.empty ()))
    {
      if (options_.mangle_flag_)
        {
          std::cerr << program_name_
            << ": -M cannot be used with other flags; -M ignored" << std::endl;
          options_.mangle_flag_ = false;
        }

      if (options_.demangle_flag_)
        {
          std::cerr << program_name_
            << ": -X cannot be used with other flags; -X ignored" << std::endl;
          options_.demangle_flag_ = false;
        }
    }

  if (options_.mangle_flag_ && options_.demangle_flag_)
    {
      std::cerr << program_name_
          << ": -X cannot be used with -M; -X ignored" << std::endl;
      options_.demangle_flag_ = false;
    }

  arguments_.assign (argv + optind, argv + argc);
}

// Informational helpers.
void
CommandLine::help ()
{
  std::cerr << "Usage: " << program_name_
      << " [ options ] file [ file ... ]" << std::endl
      << "       " << program_name_ << " -M id [ id ... ]" << std::endl
      << "       " << program_name_ << " -X id [ id ... ]" << std::endl
      << std::endl
      << "Options:" << std::endl
      << " -g          Include debugging information in the output files"
      << std::endl
      << " -p,-pg      Include line profiling for gmon.out execution profile"
      << std::endl
      << " -w          Emit weak symbols for word definitions (for libraries)"
      << std::endl
      << " -fPIC,-fpic Generate position independent code, for shared libraries"
      << std::endl
      << " -O          Optimize generated code (modest optimization only)"
      << std::endl
      << " -P          Write out intermediate file (.p) during compilation"
      << std::endl
      << " -S,-s       Write out assembly language file (.s) during compilation"
      << std::endl
      << " -D<string>  Define string, for [ifdef] conditional compilation"
      << std::endl
      << " -U<string>  Undefine string defined previously with -D"
      << std::endl
      << " -M          Name-mangle each id given, for linking with 'C'"
      << std::endl
      << " -X          Name-demangle each id given, for linking with 'C'"
      << std::endl
      << " -v          Print compiler version number and exit"
      << std::endl
      << " -h,--help   Print this help message"
      << std::endl;
}

void
CommandLine::version ()
{
  const size_t wordlen = sizeof (void*);

  std::cout << std::endl << "VNPForth 1.5 ["
      << "build "
      << (wordlen == 8 ? "x86_64" : (wordlen == 4 ? "x86" : "unknown"))
      << '/' << STAMP << ']' << std::endl
      << "Copyright (C) 2005-2013 Simon Baldwin" << std::endl << std::endl
      << "This program comes with ABSOLUTELY NO WARRANTY; for details"
      << std::endl
      << "please see the file 'COPYING' supplied with the source code."
      << std::endl
      << "This is free software, and you are welcome to redistribute it"
      << std::endl
      << "under certain conditions; again, see 'COPYING' for details."
      << std::endl
      << "This program is released under the GNU General Public License."
      << std::endl;
}

void
CommandLine::usage ()
{
  std::cerr << "Try '" << program_name_
      << " --help' for more information." << std::endl;
}

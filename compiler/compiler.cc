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

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fstream>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>

#include "cmdline.h"
#include "mangler.h"
#include "program.h"

// Helpers for for_each, mangler and compiler.
namespace {

// Mangle or demangle the given name and print it.
void
mangle (const std::string & name)
{
  std::cout << Mangler::mangle (name) << std::endl;
}

void
demangle (const std::string & name)
{
  std::cout << Mangler::demangle (name) << std::endl;
}

// Program exit status.
int exit_status = EXIT_SUCCESS;

// Compile the given source file using command line options.
void
compile (const std::string source, const CommandLine * commandline)
{
  // Create a program from the source file and options.
  Program program;
  if (!program.build (source, *commandline))
    {
      exit_status = EXIT_FAILURE;
      return;
    }

  // Find the base name of the source.
  const std::string::size_type slash = source.find_last_of ('/');
  const std::string::size_type dot = source.find_last_of ('.');

  std::string base = source;
  if (dot < source.size () && (dot > slash || slash > source.size ()))
    base.erase (dot, base.size ());

  // Write an intermediate listing if requested.
  const Options options = commandline->get_options ();

  if (options.save_intermediate ())
    {
      const std::string & path = base + ".p";
      std::ofstream outs (path.c_str ());
      if (!outs)
        {
          std::cerr << commandline->get_program_name () << ": "
                    << path << ": " << std::strerror (errno) << std::endl;
          exit_status = EXIT_FAILURE;
          return;
        }
      program.create_listing (outs);
      outs.close ();
    }

  // Create and write assembly translation.
  const std::string & path = base + ".s";
  std::ofstream outs (path.c_str ());
  if (!outs)
    {
      std::cerr << commandline->get_program_name () << ": "
                << path << ": " << std::strerror (errno) << std::endl;
      exit_status = EXIT_FAILURE;
      return;
    }
  program.generate (outs);
  outs.close ();

  // Assemble into an object file.
  const std::string & as = std::string ("as --32 -o ") + base + ".o " + path;
  const int status = system (as.c_str ());
  if (status != EXIT_SUCCESS)
    {
      std::cerr << commandline->get_program_name () << ": "
                << path << ": " << as << ", status "
                << status << std::endl;
      exit_status = EXIT_FAILURE;
      return;
    }

  if (!options.save_assembly ())
    unlink (path.c_str ());
}

} // namespace

// Main program.
int main (int argc, char * const * argv)
{
  try
    {
      // If only mangling or demangling, name de/mangle each named argument.
      const CommandLine commandline = CommandLine (argc, argv);
      if (commandline.get_options ().mangle_only ())
        {
          std::vector<std::string> names = commandline.get_arguments ();
          std::for_each (names.begin (), names.end (), mangle);
          return EXIT_SUCCESS;
        }

      if (commandline.get_options ().demangle_only ())
        {
          std::vector<std::string> names = commandline.get_arguments ();
          std::for_each (names.begin (), names.end (), demangle);
          return EXIT_SUCCESS;
        }

      // Otherwise, compile each named source file.
      const std::vector<std::string> sources = commandline.get_arguments ();
      std::for_each (sources.begin (), sources.end (),
                     std::bind2nd (std::ptr_fun (compile), &commandline));
      return exit_status;
    }

  // Handle any exceptions thrown by the above.
  catch (std::string & s)
    {
      std::cerr << "Internal error: " << s << std::endl;
      return EXIT_FAILURE;
    }
  catch (std::bad_alloc)
    {
      std::cerr << "Internal error: heap memory exhausted" << std::endl;
      return EXIT_FAILURE;
    }
  catch (...)
    {
      std::cerr << "Internal error: unknown problem" << std::endl;
      return EXIT_FAILURE;
    }
}

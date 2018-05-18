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

#ifndef VNPFORTH_OPTIONS_H
#define VNPFORTH_OPTIONS_H

class CommandLine;

// Generalized control options and flags.
class Options
{
  friend class CommandLine;

public:
  inline
  Options ()
    : debugging_flag_ (false), profiling_flag_ (false), weak_flag_ (false),
      PIC_flag_ (false), optimize_flag_ (false), intermediate_flag_ (false),
      assembly_flag_ (false), mangle_flag_ (false), demangle_flag_ (false),
      trace_parser_flag_ (false) { }

  inline bool
  include_debugging () const
  {
    return debugging_flag_;
  }

  inline bool
  include_profiling () const
  {
    return profiling_flag_;
  }

  inline bool
  weak_functions () const
  {
    return weak_flag_;
  }

  inline bool
  position_independent () const
  {
    return PIC_flag_;
  }

  inline bool
  optimize_code () const
  {
    return optimize_flag_;
  }

  inline bool
  save_intermediate () const
  {
    return intermediate_flag_;
  }

  inline bool
  save_assembly () const
  {
    return assembly_flag_;
  }

  inline bool
  mangle_only () const
  {
    return mangle_flag_;
  }

  inline bool
  demangle_only () const
  {
    return demangle_flag_;
  }

  inline bool
  trace_parser () const
  {
    return trace_parser_flag_;
  }

private:
  bool debugging_flag_;
  bool profiling_flag_;
  bool weak_flag_;
  bool PIC_flag_;
  bool optimize_flag_;
  bool intermediate_flag_;
  bool assembly_flag_;
  bool mangle_flag_;
  bool demangle_flag_;
  bool trace_parser_flag_;
};

#endif

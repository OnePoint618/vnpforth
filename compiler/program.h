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

#ifndef VNPFORTH_PROGRAM_H
#define VNPFORTH_PROGRAM_H

#include <iostream>
#include <string>

#include "dattable.h"
#include "optable.h"
#include "symtable.h"

class CommandLine;

// Program, aggregate and facade for tables, populated by Parser.
class Program
{
public:
  Program () { }

  bool build (const std::string & source_path, const CommandLine & commandline);
  void create_listing (std::ostream & outs) const;

  inline std::string
  get_source_path () const
  {
    return source_path_;
  }

  inline const OpcodeTable &
  get_opcode_table_const () const
  {
    return optable_;
  }

  inline DataTable &
  get_data_table ()
  {
    return datatable_;
  }

  inline SymbolTable &
  get_symbol_table ()
  {
    return symtable_;
  }

  inline OpcodeTable &
  get_opcode_table ()
  {
    return optable_;
  }

  inline Options
  get_options () const
  {
    return options_;
  }

  void generate (std::ostream & outs) const;

private:
  Program (const Program & program);
  Program & operator= (const Program & program);

  void generate_preamble (std::ostream & outs) const;
  void generate_postamble (std::ostream & outs) const;

  std::string source_path_;

  DataTable datatable_;
  SymbolTable symtable_;
  OpcodeTable optable_;

  Options options_;
};

#endif

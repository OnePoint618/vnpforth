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

#ifndef VNPFORTH_OPCODETABLE_H
#define VNPFORTH_OPCODETABLE_H

#include <iostream>
#include <set>
#include <string>
#include <vector>

class Options;
class SymbolTable;
class Opcode;

// Opcode table, aggregates opcode pointers into a vector.  The table
// takes ownership of data objects, and deletes them in its destructor.
class OpcodeTable
{
public:
  OpcodeTable () { }
  ~OpcodeTable ();

  void add (Opcode * opcode);
  void create_listing (std::ostream & outs) const;
  void clear ();

  inline bool
  is_empty () const
  {
    return opcodes_.empty ();
  }

  void synthesize_main (const SymbolTable & symtable);
  void unreachable_check (const std::string & source_path) const;
  void optimize_code (const std::string & source_path);

  void generate (std::ostream & outs, const Options & options) const;

private:
  OpcodeTable (const OpcodeTable & table);
  OpcodeTable & operator= (const OpcodeTable & table);

  typedef std::vector<Opcode *> OpcodeTableStore;
  typedef OpcodeTableStore::const_iterator OpcodeTableIterator;
  typedef OpcodeTableStore::iterator OpcodeTableIterator_mutable;

  void replace_with (OpcodeTableIterator_mutable iter, Opcode * replacement);
  void replace_with_nop (OpcodeTableIterator_mutable iter);
  int remove_unreachable_code ();
  int remove_unnecessary_jumps ();
  int remove_useless_calls ();
  int inline_booleans ();
  int inline_small_functions ();
  int replace_adjacent_push_pop_pairs ();
  int relocate_suboptimal_labels ();
  int remove_unnecessary_labels ();

  OpcodeTableStore opcodes_;
  std::set<Opcode *> opcode_collection_;
};

#endif

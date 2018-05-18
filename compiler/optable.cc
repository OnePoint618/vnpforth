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
#include <functional>
#include <string>
#include <vector>

#include "opcode.h"
#include "optable.h"
#include "symtable.h"

// Delete all opcode objects in the table, and destructor.
namespace {

void
dispose (const Opcode * opcode)
{
  delete opcode;
}

} // namespace

void
OpcodeTable::clear ()
{
  std::for_each (opcode_collection_.begin (),
                 opcode_collection_.end (), dispose);
  opcodes_.clear ();
  opcode_collection_.clear ();
}

OpcodeTable::~OpcodeTable ()
{
  clear ();
}

// Add opcode objects sequentially to opcodes.  The optimizer may totally
// restructure and reorder the opcodes, so we maintain a separate record of
// everything, used for garbage collection on clearing/deletion.
void
OpcodeTable::add (Opcode * opcode)
{
  opcodes_.push_back (opcode);
  opcode_collection_.insert (opcode);
}

// Pretty-print to create the opcode table listing.
namespace {

void
print_out (const Opcode * opcode, std::ostream & outs)
{
  outs << opcode->create_list_entry ();

  if (opcode->is_enddefine_opcode ())
    outs << std::endl;
}

} // namespace

void
OpcodeTable::create_listing (std::ostream & outs) const
{
  if (opcodes_.empty ())
    return;

  outs << "Intermediate code:" << std::endl;
  std::for_each (opcodes_.begin (), opcodes_.end (),
                 std::bind2nd (std::ptr_fun (print_out), outs));
}

// Synthesize a main function from any opcodes not inside a definition.
void
OpcodeTable::synthesize_main (const SymbolTable & symtable)
{
  // Split opcodes into two sets; those inside definitions, and those not.
  OpcodeTableStore main_opcodes, function_opcodes;
  bool in_main = true;

  for (size_t i = 0; i < opcodes_.size (); ++i)
    {
      Opcode * opcode = opcodes_[i];

      if (opcode->is_define_opcode ())
        in_main = false;

      OpcodeTableStore * table = in_main ? &main_opcodes : &function_opcodes;
      table->push_back (opcode);

      if (opcode->is_enddefine_opcode ())
        in_main = true;
    }

  // Combine sets, appending opcodes not in definitions into a synthesized
  // main definition.  We expect the parser to have added the "main" symbol.
  if (!main_opcodes.empty ())
    {
      opcodes_.assign (function_opcodes.begin (), function_opcodes.end ());

      const Symbol * symbol = symtable.lookup ("main");
      const DefinableSymbol * main = symbol->is_definable_symbol ();

      const int first_line = main_opcodes.front ()->get_line ();
      new DefineOpcode (main, first_line, this);

      opcodes_.insert (opcodes_.end (),
                       main_opcodes.begin (), main_opcodes.end ());

      const int last_line = main_opcodes.back ()->get_line ();
      new LabelOpcode (Label (0), last_line, this);
      new EndDefineOpcode (main, last_line, this);
    }
}

// Unreachability check, finds occurrences of an unconditional jump that is
// followed by a non-no-op, non-label opcode.  The jump may have been inserted
// by codegen, so unreachability is not necessarily present in the source.
void
OpcodeTable::unreachable_check (const std::string & source_path) const
{
  for (OpcodeTableIterator iter = opcodes_.begin (); iter != opcodes_.end (); )
    {
      // Find the next unconditional jump.
      const OpcodeTableIterator first =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_jump_opcode));
      if (first == opcodes_.end ())
        break;

      // Find the non-no-op after the jump.
      const OpcodeTableIterator second =
          std::find_if (first + 1, opcodes_.end (),
                        std::not1 (std::mem_fun (&Opcode::is_noop_opcode)));
      if (second == opcodes_.end ())
        break;

      // If not a label, this code may be unreachable.
      const Opcode * next_opcode = *second;
      if (!next_opcode->is_label_opcode ())
        {
          const Opcode * jump_opcode = *first;

          std::cerr << source_path << ':' << next_opcode->get_line ()
                    << ": warning: possibly unreachable code, skipped by line "
                    << jump_opcode->get_line () << std::endl;
        }

      // Start the next scan after the jump just processed.
      iter = first + 1;
    }
}

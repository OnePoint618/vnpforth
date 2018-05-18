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
#include <cassert>
#include <cctype>
#include <climits>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>
#include <unistd.h>

#include "data.h"
#include "dattable.h"
#include "mangler.h"
#include "opcode.h"
#include "optable.h"
#include "options.h"
#include "program.h"
#include "register.h"
#include "stack.h"
#include "symbol.h"
#include "symtable.h"

// Code generator functions.  These belong to assorted classes, but are
// concentrated here to keep cpu-specific code in a single module.

// Stack and register definitions.
const Stack Stack::DATA = Stack (
    "D", Mangler::mangle ("_dpush"), Mangler::mangle ("_dpop"));
const Stack Stack::RETURN = Stack (
    "R", Mangler::mangle ("_rpush"), Mangler::mangle ("_rpop"));
const Stack Stack::FLOAT = Stack (
    "F", Mangler::mangle ("_fpush"), Mangler::mangle ("_fpop"));

const Register Register::R0 = Register ("0", "%eax");
const Register Register::R1 = Register ("1", "%edx");

// Convenience namespace for stab types.
namespace Stabtype { enum { VOID = 1, CELL = 2, UCELL = 3, CHAR = 4 }; }

// Debug constant, and hack to suppress unused arguments warnings.
namespace {

const int CELL_SIZE = sizeof (int);

inline void
args_unused (std::ostream & outs, const Options & options)
{
  outs << (options.include_debugging () ? "" : "");
}

inline void
args_unused (const Options & options)
{
  options.include_debugging ();
}

} // namespace

// Overall program generators.
void
Program::generate_preamble (std::ostream & outs) const
{
  outs << "\t.file\t" << '"' << source_path_ << '"' << std::endl
       << "\t.version\t" << '"' << "01.01" << '"' << std::endl
       << "forth_compiled.:" << std::endl;

  if (options_.include_profiling ())
    outs << "\tnop" << std::endl;

  // Debugger preamble identifies module, and sets up language variable types.
  if (options_.include_debugging ())
    {
      if (char * cwd = getcwd (NULL, 0))  // Linux extension to getcwd
        {
          outs << ".stabs\t" << '"' << cwd << '/' << '"' << ",100,0,0,Ltext0"
               << std::endl;
          std::free (cwd);
        }

      outs << ".stabs\t" << '"' << source_path_ << '"' << ",100,0,0,Ltext0"
           << std::endl
           << "\t.stabs\t" << '"' << "forth_compiled." << '"' << ",0x3c,0,0,0"
           << std::endl;

      outs << ".text" << std::endl
           << "Ltext0:" << std::endl;
      outs << ".stabs\t" << '"' << "void:t" << Stabtype::VOID << '='
           << Stabtype::VOID << ';' << '"' << ",128,0,0,0" << std::endl;
      outs << ".stabs\t" << '"' << "cell:t" << Stabtype::CELL << '=' << 'r'
           << Stabtype::CELL << ';' << INT_MIN << ';' << INT_MAX << ';'
           << '"' << ",128,0,0,0" << std::endl;
      outs << ".stabs\t" << '"' << "ucell:t" << Stabtype::UCELL << '=' << 'r'
           << Stabtype::CELL << ';' << 0 << ';'
           << static_cast<int>(UINT_MAX) << ';'
           << '"' << ",128,0,0,0" << std::endl;
      outs << ".stabs\t" << '"' << "char:t" << Stabtype::CHAR << '=' << 'r'
           << Stabtype::CHAR << ';' << 0 << ';' << CHAR_MAX << ';'
           << '"' << ",128,0,0,0" << std::endl;
    }
}

void
Program::generate_postamble (std::ostream & outs) const
{
  if (options_.include_debugging ())
    {
      outs << "\t.text" << std::endl
           << "\t.stabs\t" << '"' << '"' << ",100,0,0,.Letext" << std::endl
           << ".Letext:" << std::endl;
    }

  outs << "\t.ident\t " << '"' << "Forthc: Forth 1.5, "
       << __DATE__ << ' ' << __TIME__ << ", Linux" << '"' << std::endl;
}

// Read-only data generators.
void
DataTable::generate (std::ostream & outs) const
{
  if (!segments_.empty ())
    outs << ".section\t.rodata" << std::endl;

  // Write labels for synonyms, and generate data for only the first listed.
  for (DataTableIterator iter = segments_.begin ();
       iter != segments_.end (); ++iter)
    {
      const std::vector<const Data *> * elements = &iter->second;

      for (size_t i = 0; i < elements->size (); ++i)
        {
          const Data * data = elements->at (i);
          outs << ".LC" << data->get_id () << ':' << std::endl;
        }

      const Data * data = elements->at (0);
      data->generate (outs);
    }
}

namespace {

std::string
data_to_string (const std::vector<unsigned char> & chunk)
{
  std::ostringstream outs;
  outs.fill ('0');

  for (size_t i = 0; i < chunk.size (); ++i)
    {
      if (chunk[i] && isprint (chunk[i]))
        outs << chunk[i];
      else
        {
          outs << '\\' << std::oct << std::setw (3)
               << static_cast<int>(chunk[i]) << std::setw (0) << std::dec;
        }
    }

  return outs.str ();
}

} // namespace

void
Data::generate (std::ostream & outs) const
{
  const std::vector<unsigned char> & chunk = get_chunk ();

  outs << "\t.string " << '"'
       << data_to_string (chunk) << '"' << std::endl;
}

void
CountedStringData::generate (std::ostream & outs) const
{
  const std::vector<unsigned char> & chunk = get_chunk ();

  std::vector<unsigned char> size;
  size.push_back (static_cast<unsigned char>(chunk.size ()));

  outs << "\t.string " << '"' << data_to_string (size)
       << data_to_string (chunk) << '"' << std::endl;
}

// Variables and constants generators.
void
SymbolTable::generate (std::ostream & outs, const Options & options) const
{
  for (SymbolTableIterator iter = symbols_.begin ();
      iter != symbols_.end (); ++iter)
    {
      iter->second->generate (outs, options);
    }
}

void
VariableSymbol::generate (std::ostream & outs, const Options & options) const
{
  if (options.include_debugging ())
    {
      outs << ".stabs\t" << '"' << get_name ();

      if (units_ == CELL && size_ == 1)
        outs << ":G" << Stabtype::CELL;
      else
        {
          outs << ":Gar" << Stabtype::CELL << ";0;" << size_ - 1 << ';'
               << ((units_ == CELL) ? Stabtype::CELL : Stabtype::CHAR);
        }

      outs << '"' << ",40,0,0," << get_name () << std::endl;
    }

  const int bytes = (units_ == CELL) ? size_ * CELL_SIZE : size_;
  const int alignment = (bytes >= 32) ? 32 : ((units_ == CELL) ? CELL_SIZE : 1);

  outs << "\t.comm  " << get_name ()
       << ',' << bytes << ',' << alignment << std::endl;
}

void
ConstantSymbol::generate (std::ostream & outs, const Options & options) const
{
  if (options.include_debugging ())
    {
      outs << ".stabs\t" << '"' << get_name () << ":G"
           << Stabtype::CELL << '"'
           << ",40,0,0," << get_name () << std::endl;
    }

  outs << "\t.comm  " << get_name ()
       << ',' << CELL_SIZE << ',' << CELL_SIZE << std::endl;
}

void
Symbol::generate (std::ostream & outs, const Options & options) const
{
  args_unused (outs, options);
}

// Opcode generators.
namespace {

const Symbol * current_definition = 0;
int current_opcode_sequence = 0;

} // namespace

void
OpcodeTable::generate (std::ostream & outs, const Options & options) const
{
  current_definition = 0;
  current_opcode_sequence = 0;
  int current_line = -1;

  for (size_t i = 0; i < opcodes_.size (); ++i)
    {
      const Opcode * opcode = opcodes_[i];

      // Write opcode debugging data for line number if required.
      if (options.include_debugging () && current_line != opcode->get_line ())
        {
          // Don't write debugger stabs for pseudo-opcodes or assembly.
          const bool requires_debug = !(opcode->is_label_opcode ()
                                        || opcode->is_noop_opcode ()
                                        || opcode->is_define_opcode ()
                                        || opcode->is_enddefine_opcode ()
                                        || opcode->is_assembly_opcode ());

          if (requires_debug)
            {
              assert (current_definition);
              outs << ".stabn 68,0," << opcode->get_line () << ",.LM"
                   << current_opcode_sequence << "-"
                   << current_definition->get_name () << std::endl
                   << ".LM" << current_opcode_sequence << ':' << std::endl;
            }
        }

      // Generate opcode assembly.
      opcode->generate (outs, options);

      current_line = opcode->get_line ();
      ++current_opcode_sequence;
    }
}

void
CallOpcode::generate (std::ostream & outs, const Options & options) const
{
  outs << "\tcall " << symbol_->get_name ()
       << (options.position_independent () ? "@PLT" : "") << std::endl;
}

void
DefineOpcode::generate (std::ostream & outs, const Options & options) const
{
  // Function preamble.
  outs << ".text" << std::endl
       << "\t.align 4" << std::endl;

  const std::string & symbol_name = symbol_->get_name ();

  if (options.include_debugging ())
    {
      outs << ".stabs " << '"' << symbol_name << ":F" << Stabtype::VOID << '"'
           << ",36,0,0," << symbol_name << std::endl;
    }

  if (!symbol_->is_anonword_symbol ())
    {
      if (options.weak_functions ())
        outs << "\t.weak " << symbol_name << std::endl;
      else
        outs << ".globl " << symbol_name << std::endl;
    }

  outs << "\t.type\t" << symbol_name << ",@function" << std::endl
       << symbol_name << ':' << std::endl;

  if (options.include_debugging ())
    {
      outs << ".stabn 68,0," << get_line () << ",.LMa"
           << current_opcode_sequence << "-" << symbol_name << std::endl
           << ".LMa" << current_opcode_sequence << ':' << std::endl;
    }

  outs << "\tpush %ebp" << std::endl
       << "\tmov %esp,%ebp" << std::endl;

  // Preserve registers.  Careful reading of the Intel ABI reveals that it's
  // necessary only to preserve selected registers, and as we don't even use
  // all of them, just the following few suffice.
  if (symbol_->is_codeword_symbol ())
    {
      // Always push %ebx first, to match the non-codeword layout.
      outs << "\tpush %ebx" << std::endl
           << "\tpush %edi" << std::endl
           << "\tpush %esi" << std::endl;
    }
  else
    {
      // %ebx is unused in non-PIC compilations, but always push it here
      // anyway.  This makes the part of the stack frame used by stack
      // unwinding is same in both PIC and non-PIC forth words, and means
      // exceptions can handle both seamlessly.  Wastes a little execution
      // time and text space in non-PIC executables.
      outs << "\tpush %ebx" << std::endl;
    }

  const int symbol_id = symbol_->get_id ();

  if (options.position_independent ())
    {
      outs << "\tcall .Lpic" << symbol_id << std::endl
           << ".Lpic" << symbol_id << ':' << std::endl
           << "\tpop %ebx" << std::endl
           << "\tadd $_GLOBAL_OFFSET_TABLE_"
           << "+[.-.Lpic" << symbol_id << "],%ebx" << std::endl;

      // Write a label usable for PIC compilation flag checking.
      outs << "0:" << std::endl;
    }

  // Set up the library's argc, argv, and envp from the main function.
  if (symbol_name == "main")
    {
      static std::map<int, std::string> stacked;

      if (stacked.empty ())
        {
          stacked[8] = Mangler::mangle ("_argc");
          stacked[12] = Mangler::mangle ("_argv");
          stacked[16] = Mangler::mangle ("_envp");
        }

      for (int offset = 8; offset <= 16; offset += CELL_SIZE)
        {
          const std::string & symbol_name = stacked[offset];

          outs << "\tmov " << offset << "(%ebp),%eax" << std::endl;
          if (options.position_independent ())
            {
              outs << "\tmov "
                   << symbol_name << "@GOT(%ebx),%ecx" << std::endl
                   << "\tmov %eax,(%ecx)" << std::endl;
            }
          else
            outs << "\tmov %eax," << symbol_name << std::endl;
        }
    }

  if (options.include_profiling ())
    {
      outs << ".data" << std::endl
           << "\t.align 4" << std::endl
           << ".LP" << symbol_id << ':' << std::endl
           << "\t.long 0" << std::endl
           << ".text" << std::endl;

      outs << "\tpush %edx" << std::endl;
      if (options.position_independent ())
        {
          outs << "\tlea .LP" << symbol_id << "@GOTOFF(%ebx),%edx"
               << std::endl
               << "\tcall *mcount@GOT(%ebx)" << std::endl;
        }
      else
        {
          outs << "\tmov $.LP" << symbol_id << ",%edx" << std::endl
               << "\tcall mcount" << std::endl;
        }
      outs << "\tpop %edx" << std::endl;
    }

  if (options.include_debugging ())
    {
      outs << ".stabn 68,0," << get_line () << ",.LMb"
           << current_opcode_sequence << "-" << symbol_name << std::endl
           << ".LMb" << current_opcode_sequence << ':' << std::endl;

      outs << ".LBB" << symbol_id << ':' << std::endl;
    }

  if (symbol_->is_codeword_symbol ())
    outs << "#APP" << std::endl;

  current_definition = symbol_;
}

void
EndDefineOpcode::generate (std::ostream & outs, const Options & options) const
{
  if (symbol_->is_codeword_symbol ())
    outs << "#NO_APP" << std::endl;

  const std::string & symbol_name = symbol_->get_name ();
  const int symbol_id = symbol_->get_id ();

  if (options.include_debugging ())
    {
      outs << ".LBE" << symbol_id << ':' << std::endl;

      outs << ".stabn 68,0," << get_line () << ",.LM"
           << current_opcode_sequence << "-" << symbol_name << std::endl
           << ".LM" << current_opcode_sequence << ':' << std::endl;
    }

  // Restore registers.
  if (symbol_->is_codeword_symbol ())
    {
      outs << "\tpop %esi" << std::endl
           << "\tpop %edi" << std::endl
           << "\tpop %ebx" << std::endl;
    }
  else
    outs << "\tpop %ebx" << std::endl;

  // Function postamble.
  outs << "\tleave" << std::endl
       << "\tret" << std::endl;

  outs << ".Lfe" << symbol_id << ':' << std::endl
       << "\t.size\t " << symbol_name << ",.Lfe" << symbol_id << '-'
       << symbol_name << std::endl;

  if (options.include_debugging ())
    {
      outs << ".stabn\t192,0,0,.LBB" << symbol_id << '-'
           << symbol_name << std::endl
           << ".stabn\t224,0,0,.LBE" << symbol_id << '-'
           << symbol_name << std::endl
           << ".Lscope" << symbol_id << ':' << std::endl
           << ".stabs\t" << '"' << '"' << ",36,0,0,"
           << ".Lscope" << symbol_id << '-' << symbol_name << std::endl;
    }

  current_definition = 0;
}

void
LabelOpcode::generate (std::ostream & outs, const Options & options) const
{
  args_unused (options);
  outs << ".L" << label_.get_value () << ':' << std::endl;
}

void
AssemblyOpcode::generate (std::ostream & outs, const Options & options) const
{
  args_unused (options);
  outs << assembly_ << std::endl;
}

void
LoadValueOpcode::generate (std::ostream & outs, const Options & options) const
{
  const int value = value_.get_value ();

  if (options.optimize_code ()
      && (value == 1 || value == -1 || value == 0))
    {
      outs << "\txor " << reg_.get_cpu_name () << ','
           << reg_.get_cpu_name () << std::endl;
      switch (value)
        {
        case  1:
          outs << "\tinc " << reg_.get_cpu_name () << std::endl;
          break;
        case -1:
          outs << "\tdec " << reg_.get_cpu_name () << std::endl;
          break;
        case  0:
          break;
        }
    }
  else
    outs << "\tmov $" << value << ',' << reg_.get_cpu_name () << std::endl;
}

void
LoadRegisterOpcode::generate (std::ostream & outs,
                              const Options & options) const
{
  args_unused (options);
  outs << "\tmov " << with_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
}

void
LoadSymbolOpcode::generate (std::ostream & outs, const Options & options) const
{
  if (options.position_independent ())
    {
      outs << "\tmov " << symbol_->get_name () << "@GOT(%ebx),"
           << reg_.get_cpu_name () << std::endl;
    }
  else
    {
      outs << "\tlea " << symbol_->get_name () << ','
           << reg_.get_cpu_name () << std::endl;
    }
}

void
LoadSymbolIndirectOpcode::generate (std::ostream & outs,
                                    const Options & options) const
{
  if (options.position_independent ())
    {
      outs << "\tmov " << symbol_->get_name () << "@GOT(%ebx),%ecx"
           << std::endl
           << "\tmov (%ecx)," << reg_.get_cpu_name () << std::endl;
    }
  else
    {
      outs << "\tmov (" << symbol_->get_name () << "),"
           << reg_.get_cpu_name () << std::endl;
    }
}

void
LoadDataOpcode::generate (std::ostream & outs, const Options & options) const
{
  if (options.position_independent ())
    {
      outs << "\tlea .LC" << data_->get_id () << "@GOTOFF(%ebx),"
           << reg_.get_cpu_name () << std::endl;
    }
  else
    {
      outs << "\tlea .LC" << data_->get_id () << ','
           << reg_.get_cpu_name () << std::endl;
    }
}

void
StoreSymbolIndirectOpcode::generate (std::ostream & outs,
                                     const Options & options) const
{
  if (options.position_independent ())
    {
      outs << "\tmov " << symbol_->get_name () << "@GOT(%ebx),%ecx"
           << std::endl
           << "\tmov " << reg_.get_cpu_name () << ",(%ecx)" << std::endl;
    }
  else
    {
      outs << "\tmov " << reg_.get_cpu_name () << ",("
           << symbol_->get_name () << ')' << std::endl;
    }
}

void
PushOpcode::generate (std::ostream & outs, const Options & options) const
{
  const bool need_exchange = !(reg_.get_cpu_name () == "%eax");

  if (need_exchange)
    outs << "\txchg " << reg_.get_cpu_name () << ",%eax" << std::endl;

  outs << "\tcall " << stack_.get_push_function ()
       << (options.position_independent () ? "@PLT" : "") << std::endl;

  if (need_exchange)
    outs << "\txchg %eax," << reg_.get_cpu_name () << std::endl;
}

void
PopOpcode::generate (std::ostream & outs, const Options & options) const
{
  const bool need_exchange = !(reg_.get_cpu_name () == "%eax");

  if (need_exchange)
    outs << "\txchg " << reg_.get_cpu_name () << ",%eax" << std::endl;

  outs << "\tcall " << stack_.get_pop_function ()
       << (options.position_independent () ? "@PLT" : "") << std::endl;

  if (need_exchange)
    outs << "\txchg %eax," << reg_.get_cpu_name () << std::endl;
}

void
AddValueOpcode::generate (std::ostream & outs, const Options & options) const
{
  const int value = value_.get_value ();

  if (options.optimize_code ()
      && (value == 1 || value == -1 || value == 0))
    {
      switch (value)
        {
        case  1:
          outs << "\tinc " << reg_.get_cpu_name () << std::endl;
          break;
        case -1:
          outs << "\tdec " << reg_.get_cpu_name () << std::endl;
          break;
        case  0:
          break;
        }
    }
  else
    outs << "\tadd $" << value << ',' << reg_.get_cpu_name () << std::endl;
}

void
AddRegisterOpcode::generate (std::ostream & outs,
                             const Options & options) const
{
  args_unused (options);
  outs << "\tadd " << increment_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
}

void
SubtractValueOpcode::generate (std::ostream & outs,
                               const Options & options) const
{
  const int value = value_.get_value ();

  if (options.optimize_code ()
      && (value == 1 || value == -1 || value == 0))
    {
      switch (value)
        {
        case  1:
          outs << "\tdec " << reg_.get_cpu_name () << std::endl;
          break;
        case -1:
          outs << "\tinc " << reg_.get_cpu_name () << std::endl;
          break;
        case  0:
          break;
        }
    }
  else
    outs << "\tsub $" << value << ',' << reg_.get_cpu_name () << std::endl;
}

void
SubtractRegisterOpcode::generate (std::ostream & outs,
                                  const Options & options) const
{
  args_unused (options);
  outs << "\tsub " << decrement_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
}

void
JumpOpcode::generate (std::ostream & outs, const Options & options) const
{
  args_unused (options);
  outs << "\tjmp .L" << label_.get_value () << std::endl;
}

void
JumpZeroOpcode::generate (std::ostream & outs, const Options & options) const
{
  args_unused (options);
  outs << "\ttest " << reg_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
  outs << "\tje .L" << label_.get_value () << std::endl;
}

void
JumpNonZeroOpcode::generate (std::ostream & outs,
                             const Options & options) const
{
  args_unused (options);
  outs << "\ttest " << reg_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
  outs << "\tjne .L" << label_.get_value () << std::endl;
}

void
JumpGreaterZeroOpcode::generate (std::ostream & outs,
                                 const Options & options) const
{
  args_unused (options);
  outs << "\ttest " << reg_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
  outs << "\tjg .L" << label_.get_value () << std::endl;
}

void
JumpLessZeroOpcode::generate (std::ostream & outs,
                              const Options & options) const
{
  args_unused (options);
  outs << "\ttest " << reg_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
  outs << "\tjl .L" << label_.get_value () << std::endl;
}

void
JumpGreaterEqualZeroOpcode::generate (std::ostream & outs,
                                      const Options & options) const
{
  args_unused (options);
  outs << "\ttest " << reg_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
  outs << "\tjge .L" << label_.get_value () << std::endl;
}

void
JumpLessEqualZeroOpcode::generate (std::ostream & outs,
                                   const Options & options) const
{
  args_unused (options);
  outs << "\ttest " << reg_.get_cpu_name () << ','
       << reg_.get_cpu_name () << std::endl;
  outs << "\tjle .L" << label_.get_value () << std::endl;
}

void
JumpEqualOpcode::generate (std::ostream & outs, const Options & options) const
{
  args_unused (options);
  outs << "\tcmp " << reg1_.get_cpu_name () << ','
       << reg2_.get_cpu_name () << std::endl;
  outs << "\tje .L" << label_.get_value () << std::endl;
}

void
JumpNotEqualOpcode::generate (std::ostream & outs,
                              const Options & options) const
{
  args_unused (options);
  outs << "\tcmp " << reg1_.get_cpu_name () << ','
       << reg2_.get_cpu_name () << std::endl;
  outs << "\tjne .L" << label_.get_value () << std::endl;
}

void
NoOpOpcode::generate (std::ostream & outs, const Options & options) const
{
  args_unused (outs, options);
}

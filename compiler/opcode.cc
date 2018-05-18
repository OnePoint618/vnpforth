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

#include <iomanip>
#include <sstream>
#include <string>

#include "data.h"
#include "opcode.h"
#include "optable.h"
#include "util.h"

// Opcode constructor, auto-adds to the given opcode table, if any.
Opcode::Opcode (int line, OpcodeTable * optable)
  : line_ (line), optimizes_ (0)
{
  if (optable)
    optable->add (this);
}

// General and front-end opcode pretty printing function.
const std::string
Opcode::create_list_entry () const
{
  std::ostringstream outs;

  outs << std::setw (6) << line_ << std::setw (0) << " "
       << create_list_specific () << std::endl;
  if (optimizes_)
    {
      outs << std::setw (11) << " " << std::setw (0)
           << "optimizes "
           << optimizes_->create_list_specific () << std::endl;
    }

  return outs.str ();
}

// Pretty printing functions for specific opcodes.
const std::string
CallOpcode::create_list_specific () const
{
  return std::string ("  call Sym_") + to_string (symbol_->get_id ());
}

const std::string
DefineOpcode::create_list_specific () const
{
  return std::string ("function Sym_") + to_string (symbol_->get_id ()) + " {";
}

const std::string
EndDefineOpcode::create_list_specific () const
{
  return std::string ("}");
}

const std::string
LabelOpcode::create_list_specific () const
{
  return std::string ("label Label_") + to_string (label_.get_value ());
}

const std::string
AssemblyOpcode::create_list_specific () const
{
  return std::string ("  asm (") + assembly_ + ")";
}

const std::string
LoadValueOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " = " + to_string (value_.get_value ());
}

const std::string
LoadRegisterOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " = Reg_" + with_.get_name ();
}

const std::string
LoadSymbolOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " = Sym_" + to_string (symbol_->get_id ());
}

const std::string
LoadSymbolIndirectOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " = *(Sym_" + to_string (symbol_->get_id ()) + ")";
}

const std::string
LoadDataOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " = Data_" + to_string (data_->get_id ());
}

const std::string
StoreSymbolIndirectOpcode::create_list_specific () const
{
  return std::string ("  *(Sym_") + to_string (symbol_->get_id ()) + ")"
         + " = Reg_" + reg_.get_name ();
}

const std::string
PushOpcode::create_list_specific () const
{
  return std::string ("  Stack_") + stack_.get_name ()
         + " = Reg_" + reg_.get_name ();
}

const std::string
PopOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " = Stack_" + stack_.get_name ();
}

const std::string
AddValueOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " += " + to_string (value_.get_value ());
}

const std::string
AddRegisterOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " += Reg_" + increment_.get_name ();
}

const std::string
SubtractValueOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " -= " + to_string (value_.get_value ());
}

const std::string
SubtractRegisterOpcode::create_list_specific () const
{
  return std::string ("  Reg_") + reg_.get_name ()
         + " -= Reg_" + decrement_.get_name ();
}

const std::string
JumpOpcode::create_list_specific () const
{
  return std::string ("  goto Label_") + to_string (label_.get_value ());
}

const std::string
JumpZeroOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg_.get_name ()
         + " == 0 goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpNonZeroOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg_.get_name ()
         + " != 0 goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpGreaterZeroOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg_.get_name ()
         + " > 0 goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpLessZeroOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg_.get_name ()
         + " < 0 goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpGreaterEqualZeroOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg_.get_name ()
         + " >= 0 goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpLessEqualZeroOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg_.get_name ()
         + " <= 0 goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpEqualOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg1_.get_name ()
         + " == Reg_" + reg2_.get_name ()
         + " goto Label_" + to_string (label_.get_value ());
}

const std::string
JumpNotEqualOpcode::create_list_specific () const
{
  return std::string ("  if Reg_") + reg1_.get_name ()
         + " != Reg_" + reg2_.get_name ()
         + " goto Label_" + to_string (label_.get_value ());
}

const std::string
NoOpOpcode::create_list_specific () const
{
  return std::string ("  no-op");
}

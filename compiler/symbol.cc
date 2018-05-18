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

#include "mangler.h"
#include "symbol.h"
#include "symtable.h"
#include "util.h"

// Static symbol id sequence and instance counter.
int Symbol::count = 0;
int Symbol::instances = 0;

// Ostream inserter for variable symbol enumerations.
namespace {

std::ostream &
operator<< (std::ostream & outs, VariableSymbol::Units units)
{
  switch (units)
    {
    case VariableSymbol::CELL:
      outs << "Cell";
      break;
    case VariableSymbol::CHARACTER:
      outs << "Char";
      break;
    }
  return outs;
}

} // namespace

// Symbol constructor, mangles and auto-adds to the given symbol table, and
// destructor, decrements instances and resets count if necessary.
Symbol::Symbol (const std::string & name,
                int line, const std::string & type, SymbolTable & symtable)
  : name_ (Mangler::mangle (name)), id_ (count++), line_ (line), type_ (type)
{
  ++instances;
  symtable.add (this);
}

Symbol::~Symbol ()
{
  if (--instances == 0)
    count = 0;
}

// Pretty-print functions to create symbol table listing.
const std::string
Symbol::create_list_entry () const
{
  std::ostringstream outs;

  outs << std::setw (6) << get_line () << std::setw (0)
       << " " << "Sym_" << get_id () << " "
       << get_type () << " " << '"' << get_name () << '"' << std::endl;

  return outs.str ();
}

const std::string
VariableSymbol::create_list_entry () const
{
  std::ostringstream outs;

  outs << std::setw (6) << get_line () << std::setw (0)
       << " " << "Sym_" << get_id () << " "
       << get_type () << " " << units_;
  if (size_ > 1)
    outs << "[" << size_ << "]";
  outs << ' ' << '"' << get_name () << '"' << std::endl;

  return outs.str ();
}

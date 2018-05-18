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
#include <functional>
#include <string>

#include "mangler.h"
#include "symbol.h"
#include "symtable.h"

typedef std::pair<const std::string, const Symbol *> SymTablePair;

// Delete all symbol objects in the table, and destructor.
namespace {

void
dispose (SymTablePair pair)
{
  delete pair.second;
}

} // namespace

void
SymbolTable::clear ()
{
  std::for_each (symbols_.begin (), symbols_.end (), dispose);
  symbols_.clear ();
}

SymbolTable::~SymbolTable ()
{
  clear ();
}

// Add and lookup symbols; name lookup mangles before searching, as symbol
// names are always mangled on symbol construction.
void
SymbolTable::add (const Symbol * symbol)
{
  assert (symbols_.find (symbol->get_name ()) == symbols_.end ());

  symbols_[symbol->get_name ()] = symbol;
}

const Symbol *
SymbolTable::lookup (const std::string & name) const
{
  const SymbolTableIterator iter = symbols_.find (Mangler::mangle (name));
  if (iter == symbols_.end ())
    return 0;

  return iter->second;
}

// Pretty-print to create the symbol table listing.
namespace {

void
print_out (SymTablePair pair, std::ostream & outs)
{
  outs << pair.second->create_list_entry ();
}

} // namespace

void
SymbolTable::create_listing (std::ostream & outs) const
{
  if (symbols_.empty ())
    return;

  outs << "Symbols:" << std::endl;
  std::for_each (symbols_.begin (), symbols_.end (),
                 std::bind2nd (std::ptr_fun (print_out), outs));
  outs << std::endl;
}

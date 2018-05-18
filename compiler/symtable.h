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

#ifndef VNPFORTH_SYMBOLTABLE_H
#define VNPFORTH_SYMBOLTABLE_H

#include <iostream>
#include <map>
#include <string>

class Options;
class Symbol;

// Symbol table, aggregates symbol pointers into a map.  The map takes
// ownership of symbols, and deletes them in its destructor.
class SymbolTable
{
public:
  SymbolTable () { }
  ~SymbolTable ();

  void add (const Symbol * symbol);
  const Symbol * lookup (const std::string & name) const;
  void create_listing (std::ostream & outs) const;
  void clear ();

  inline bool
  is_empty () const
  {
    return symbols_.empty ();
  }

  void generate (std::ostream & outs, const Options & options) const;

private:
  SymbolTable (const SymbolTable & table);
  SymbolTable & operator= (const SymbolTable & table);

  typedef std::map<const std::string, const Symbol *> SymbolTableStore;
  typedef SymbolTableStore::const_iterator SymbolTableIterator;

  SymbolTableStore symbols_;
};

#endif

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

#ifndef VNPFORTH_DATATABLE_H
#define VNPFORTH_DATATABLE_H

#include <iostream>
#include <map>
#include <vector>

class Data;

// Data table, aggregates data pointers into a vector.  The vector takes
// ownership of data objects, and deletes them in its destructor.
class DataTable
{
public:
  DataTable () { }
  ~DataTable ();

  void add (Data * data);
  void create_listing (std::ostream & outs) const;
  void clear ();

  inline bool
  is_empty () const
  {
    return segments_.empty ();
  }

  void generate (std::ostream & outs) const;

private:
  DataTable (const DataTable & table);
  DataTable & operator= (const DataTable & table);

  typedef std::map<std::string, std::vector<const Data *> > DataTableStore;
  typedef DataTableStore::const_iterator DataTableIterator;

  DataTableStore segments_;
};

#endif

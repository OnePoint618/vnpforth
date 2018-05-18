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

#include "data.h"
#include "dattable.h"

typedef std::pair<const std::string,
                  std::vector<const Data *> > DataTablePair;

// Delete all data objects in the table, and destructor.
namespace {

void
dispose (const Data * data)
{
  delete data;
}

void
dispose_vector (DataTablePair pair)
{
  std::for_each (pair.second.begin (), pair.second.end (), dispose);
  pair.second.clear ();
}

} // namespace

void
DataTable::clear ()
{
  std::for_each (segments_.begin (), segments_.end (), dispose_vector);
  segments_.clear ();
}

DataTable::~DataTable ()
{
  clear ();
}

void
DataTable::add (Data * data)
{
  const std::string & signature = data->get_signature ();

  const DataTableIterator iter = segments_.find (signature);
  if (iter != segments_.end ())
    data->set_synonym_of (iter->second[0]);

  segments_[signature].push_back (data);
}

// Pretty-print to create the symbol table listing.
namespace {

void
print_out (const Data * data, std::ostream & outs)
{
  outs << data->create_list_entry ();
}

void
print_out_vector (DataTablePair pair, std::ostream & outs)
{
  std::for_each (pair.second.begin (), pair.second.end (),
                 std::bind2nd (std::ptr_fun (print_out), outs));
}

} // namespace

void
DataTable::create_listing (std::ostream & outs) const
{
  if (segments_.empty ())
    return;

  outs << "Data:" << std::endl;
  std::for_each (segments_.begin (), segments_.end (),
                 std::bind2nd (std::ptr_fun (print_out_vector), outs));
  outs << std::endl;
}

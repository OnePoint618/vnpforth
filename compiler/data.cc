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

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include "data.h"
#include "dattable.h"
#include "util.h"

// Static data id sequence and instance counter.
int Data::count = 0;
int Data::instances = 0;

// Data constructor, auto-adds to the given data table, and destructor to
// decrement instances and potentially reset count.
Data::Data (size_t size, void const * pod,
            int line, const std::string & type, DataTable & datatable)
  : id_ (count++), chunk_ (size), line_ (line), type_ (type), synonym_of_ (0)
{
  const unsigned char * p = reinterpret_cast<const unsigned char *>(pod);
  chunk_.assign (p, p + size);
  ++instances;
  datatable.add (this);
}

Data::~Data ()
{
  if (--instances == 0)
    count = 0;
}

// Generate a unique signature for the contained data.
const std::string
Data::get_signature () const
{
  const std::vector<unsigned char> & chunk = get_chunk ();

  std::ostringstream outs;

  // Include the type.  Different types cannot share data segments because
  // their code generation is not identical.
  outs << get_type ();

  for (size_t i = 0; i < chunk.size (); ++i)
    outs << std::hex << std::setw (2) << static_cast<int>(chunk[i]);

  return outs.str ();
}

// Pretty-print functions to create data table listing.
const std::string
Data::create_list_data () const
{
  const std::vector<unsigned char> & chunk = get_chunk ();

  std::ostringstream outs;
  for (size_t i = 0; i < chunk.size (); ++i)
    {
      if (isprint (chunk[i]))
        outs << chunk[i];
      else
        outs << "\\0" << std::oct << static_cast<int>(chunk[i]);
    }

  return outs.str ();
}

const std::string
Data::create_list_entry () const
{
  std::ostringstream outs;

  outs << std::setw (6) << line_ << std::setw (0)
       << " " << "Data_" << get_id () << " ";
  if (synonym_of_)
    outs << "synonym Data_" << synonym_of_->get_id () << std::endl;
  else
    {
      outs << type_ << "[" << chunk_.size () << "] "
           << '"' << create_list_data () << '"' << std::endl;
    }

  return outs.str ();
}

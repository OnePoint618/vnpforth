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

#ifndef VNPFORTH_DATA_H
#define VNPFORTH_DATA_H

#include <string>
#include <vector>

class DataTable;

// Abstract data class, holds untyped binary data and some housekeeping,
// including a sequential id for each object created.
class Data
{
public:
  virtual ~Data ();

  inline int
  get_id () const
  {
    return id_;
  }

  inline void
  set_synonym_of (const Data * synonym_of)
  {
    synonym_of_ = synonym_of;
  }

  const std::string create_list_entry () const;
  virtual const std::string create_list_data () const;

  const std::string get_signature () const;

  virtual void generate (std::ostream & outs) const;

protected:
  Data (size_t size, void const * pod,
        int line, const std::string & type, DataTable & datatable);

  inline const std::string
  get_type () const
  {
    return type_;
  }

  inline const std::vector<unsigned char> &
  get_chunk () const
  {
    return chunk_;
  }

private:
  static int count;
  static int instances;

  const int id_;
  std::vector<unsigned char> chunk_;
  const int line_;
  const std::string type_;
  const Data * synonym_of_;
};

// Concrete string data classes.
class CountedStringData: public Data
{
public:
  CountedStringData (size_t size,
                     void const * pod, int line, DataTable & datatable)
    : Data (size, pod, line, "cstring", datatable) { }

  void generate (std::ostream & outs) const;
};

class StringData: public Data
{
public:
  StringData (size_t size, void const * pod, int line, DataTable & datatable)
    : Data (size, pod, line, "string", datatable) { }
};

#endif

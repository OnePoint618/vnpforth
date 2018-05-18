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

#ifndef VNPFORTH_REGISTER_H
#define VNPFORTH_REGISTER_H

#include <string>

// Register class, defines two registers, R0 and R1.
class Register
{
public:
  inline const std::string
  get_name () const
  {
    return name_;
  }

  inline const std::string
  get_cpu_name () const
  {
    return cpu_name_;
  }

  inline bool
  equals (const Register & reg) const
  {
    return name_ == reg.name_;
  }

  static const Register R0;
  static const Register R1;

private:
  inline
  Register (const std::string & name, const std::string & cpu_name)
    : name_ (name), cpu_name_ (cpu_name) { }

  const std::string name_;
  const std::string cpu_name_;
};

#endif

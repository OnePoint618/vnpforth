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

#ifndef VNPFORTH_STACK_H
#define VNPFORTH_STACK_H

#include <string>

// Stack class, defines three stacks, and their push and pop functions.
class Stack
{
public:
  inline const
  std::string get_name () const
  {
    return name_;
  }

  inline const std::string
  get_push_function () const
  {
    return push_function_;
  }

  inline const std::string
  get_pop_function () const
  {
    return pop_function_;
  }

  inline bool
  equals (const Stack & stack) const
  {
    return name_ == stack.name_;
  }

  static const Stack DATA;
  static const Stack RETURN;
  static const Stack FLOAT;

private:
  inline
  Stack (const std::string & name,
         const std::string & push_function, const std::string & pop_function)
    : name_ (name),
      push_function_ (push_function), pop_function_ (pop_function) { }

  const std::string name_;
  const std::string push_function_;
  const std::string pop_function_;
};

#endif

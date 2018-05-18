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

#ifndef VNPFORTH_PARSER_H
#define VNPFORTH_PARSER_H

#include <cstdio>
#include <iostream>
#include <list>
#include <set>
#include <stack>
#include <string>
#include <utility>

#include "operand.h"

class DataTable;
class SymbolTable;
class OpcodeTable;
class CommandLine;
class DefinableSymbol;

// Parser, builds a program from source.
class Parser
{
public:
  static const int MAX_ALLOWED_ERRORS;

  inline
  Parser ()
    : dattable_ (0), symtable_ (0), optable_ (0), line_number_ (0),
      error_count_ (0), current_definition_ (0), label_sequence_ (0),
      exit_label_ (0), implicit_main_ (false) { }

  bool parse (const std::string & source_path,
              std::FILE * stream, DataTable * dattable,
              SymbolTable * symtable, OpcodeTable * optable,
              const std::set<std::string> * definitions, bool trace_parser);

  // Parser terminals, public to be callable by non-OO yacc/lex modules.
  inline void
  next_line ()
  {
    ++line_number_;
  }

  inline bool
  excessive_errors () const
  {
    return error_count_ >= MAX_ALLOWED_ERRORS;
  }

  std::ostream & parse_error ();
  bool cond_state ();
  void f_cond_ifdef (const std::string & s, bool is_ifundef);
  void f_cond_else ();
  void f_cond_then_endif (bool is_endif);
  void f_end_of_file ();
  void f_definedvariable (const std::string & s);
  void f_definedarray (const std::string & s,
                       const std::string & ssize, const std::string & type);
  void f_definedarray (const std::string & s, const std::string & ssize);
  void f_definedconstant (const std::string & s);
  void f_definedfconstant (const std::string & s);
  void f_definedword (const std::string & s);
  void f_nonameword ();
  void f_semicolon ();
  void f_exit ();
  void f_pstring (const std::string & s);
  void f_cstring (const std::string & s);
  void f_astring (const std::string & s);
  void f_dotparenstring (const std::string & s);
  void f_word (const std::string & s);
  void f_object_address (const std::string & s);
  void f_char_literal (const std::string & s);
  void f_integer_literal (const std::string & s);
  void f_float_literal (const std::string & s);
  void f_string_literal (const std::string & s);
  void f_if ();
  void f_then_endif ();
  void f_else ();
  void f_do ();
  void f_query_do ();
  void f_loop ();
  void f_plus_loop ();
  void f_leave ();
  void f_query_leave ();
  void f_begin ();
  void f_until ();
  void f_while ();
  void f_repeat_again ();
  void f_case ();
  void f_of ();
  void f_endof ();
  void f_endcase ();
  void f_recurse ();
  void f_codeword (const std::string & s);
  void f_code (const std::string & s);
  void f_end_code ();

private:
  void parse_setup ();
  void parse_cleanup ();

  std::ostream & parse_warning ();

  std::string source_path_;
  DataTable * dattable_;
  SymbolTable * symtable_;
  OpcodeTable * optable_;
  const std::set<std::string> * definitions_;

  int line_number_;
  int error_count_;

  const DefinableSymbol * current_definition_;

  int label_sequence_;
  int exit_label_;
  std::stack<Label> label_stack_;
  std::stack<Label> break_stack_;
  std::list<std::pair<bool, int> > cond_stack_;

  bool implicit_main_;
  void detect_main ();
};

#endif

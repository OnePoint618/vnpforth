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
#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include <sstream>
#include <stack>
#include <string>
#include <utility>

#include "data.h"
#include "dattable.h"
#include "opcode.h"
#include "operand.h"
#include "optable.h"
#include "parser.h"
#include "symbol.h"
#include "symtable.h"
#include "util.h"

// Stub yywrap() to avoid linking libfl.
extern "C" int yywrap (void) { return 1; }

// Global declaration of pointer to active parser, linked to by yacc/lex
// modules, and external declaration of the bits of yacc/lex we call/use.
Parser * parser_ = 0;
extern void yyrestart (std::FILE *);
extern int yyparse ();
extern const char * yylval;
extern int yydebug;

// Static count of max tolerable errors before giving up.
const int Parser::MAX_ALLOWED_ERRORS = 20;

// Front-end parser function, driver for yacc/lex code.
bool
Parser::parse (const std::string & source_path,
               std::FILE * stream, DataTable * dattable,
               SymbolTable * symtable, OpcodeTable * optable,
               const std::set<std::string> * definitions, bool trace_parser)
{
  source_path_ = source_path;

  dattable_ = dattable;
  symtable_ = symtable;
  optable_ = optable;
  definitions_ = definitions;

  parse_setup ();

  parser_ = this;
  yyrestart (stream);
  yydebug = trace_parser;
  const int parse_status = yyparse ();
  parser_ = 0;
  yydebug = 0;

  parse_cleanup ();

  dattable_ = 0;
  symtable_ = 0;
  optable_ = 0;
  definitions_ = 0;

  return (parse_status == 0) && (error_count_ == 0);
}

// Parser error reporting, called from yacc/lex or internally only.
std::ostream &
Parser::parse_error ()
{
  ++error_count_;
  std::cerr << source_path_ << ':';
  if (parser_ && yylval)
    std::cerr << line_number_ << ": error: near '" << trim (yylval) << "', ";
  else
    std::cerr << " error: ";
  return std::cerr;
}

std::ostream &
Parser::parse_warning ()
{
  std::cerr << source_path_ << ':';
  if (parser_ && yylval)
    std::cerr << line_number_ << ": warning: near '" << trim (yylval) << "', ";
  else
    std::cerr << " warning: ";
  return std::cerr;
}

void
Parser::parse_setup ()
{
  line_number_ = 1;
  error_count_ = 0;
  current_definition_ = 0;

  label_sequence_ = 1;
  exit_label_ = 0;

  implicit_main_ = false;
}

void
Parser::parse_cleanup ()
{
  if (error_count_ == 0)
    {
      if (!(label_stack_.empty ()
            && break_stack_.empty () && cond_stack_.empty ()))
        {
          parse_error ()
              << "internal error: parse stacks not emptied" << std::endl;
        }

      if (current_definition_)
        {
          parse_error ()
              << "internal error: definition not exited" << std::endl;
        }
      if (exit_label_)
        {
          parse_error ()
              << "internal error: definition end missed" << std::endl;
        }

      if (dattable_->is_empty ()
          && symtable_->is_empty () && optable_->is_empty ())
        {
          parse_warning ()
              << "file contained no symbols or code" << std::endl;
        }
    }
}

// Strto[u]l wrappers, explicitly overflow values that fit long but not int
// and add support for non-decimal literal prefixes.
namespace {

long
to_long (bool negative, const std::string & str, int base)
{
  errno = 0;
  const std::string repr = std::string (negative ? "-" : "") + str;
  return std::strtol (repr.c_str (), 0, base);
}

int
to_int (const std::string & str)
{
  const char * ss = str.c_str ();

  const bool negative = (ss[0] == '-');
  ss += (ss[0] == '-' || ss[0] == '+') ? 1 : 0;

  long value = 0;
  switch (ss[0])
    {
    case '%': value = to_long (negative, ss + 1, 2); break;
    case '$': value = to_long (negative, ss + 1, 16); break;
    case '&':
    case '#': value = to_long (negative, ss + 1, 10); break;
    case '0':
      switch (ss[1])
        {
        case 'b': case 'B': value = to_long (negative, ss + 2, 2); break;
        case 'o': case 'O': value = to_long (negative, ss + 2, 8); break;
        case 'x': case 'X': value = to_long (negative, ss + 2, 16); break;
        default: value = to_long (negative, ss, 10); break;
        }
        break;
    default: value = to_long (negative, ss, 10); break;
    }

  errno = value > INT_MAX || value < INT_MIN ? ERANGE : errno;
  return value;
}

unsigned long
to_ulong (bool negative, const std::string & str, int base)
{
  errno = 0;
  const std::string repr = std::string (negative ? "-" : "") + str;
  return std::strtoul (repr.c_str (), 0, base);
}

unsigned int
to_uint (const std::string & str)
{
  const char * ss = str.c_str ();

  const bool negative = (ss[0] == '-');
  ss += (ss[0] == '-' || ss[0] == '+') ? 1 : 0;

  unsigned long value = 0;
  switch (ss[0])
    {
    case '%': value = to_ulong (negative, ss + 1, 2); break;
    case '$': value = to_ulong (negative, ss + 1, 16); break;
    case '&':
    case '#': value = to_ulong (negative, ss + 1, 10); break;
    case '0':
      switch (ss[1])
        {
        case 'b': case 'B': value = to_ulong (negative, ss + 2, 2); break;
        case 'o': case 'O': value = to_ulong (negative, ss + 2, 8); break;
        case 'x': case 'X': value = to_ulong (negative, ss + 2, 16); break;
        default: value = to_ulong (negative, ss, 10); break;
        }
        break;
    default: value = to_ulong (negative, ss, 10); break;
    }

  errno = value > UINT_MAX ? ERANGE : errno;
  return value;
}

} // namespace

// Detect implicit main function.
void
Parser::detect_main ()
{
  if (current_definition_ || implicit_main_)
    return;

  if (const Symbol * symbol = symtable_->lookup ("main"))
    {
      parse_error () << "'main' both defined and implicit"
          << ", from line " << symbol->get_line () << std::endl;
    }
  else
    new WordSymbol ("main", line_number_, *symtable_);

  implicit_main_ = true;
}

// Return the current conditional state.  Parsing is suspended if there are
// any false conditionals in the stack.
namespace {

bool
cond_flag (std::pair<bool, int> pair)
{
  return pair.first;
}

} // namespace

bool
Parser::cond_state ()
{
  const size_t count = std::count_if (cond_stack_.begin (),
                                      cond_stack_.end (), cond_flag);
  return count == cond_stack_.size ();
}

// Parse terminal handlers, all called from yacc/lex only.
void
Parser::f_cond_ifdef (const std::string & s, bool is_ifundef)
{
  std::istringstream ins (s);

  std::string junk, ss;
  ins >> junk >> ss;

  ss = to_lower (ss);
  const bool is_defined = (definitions_->find (ss) != definitions_->end ());
  const bool condition = is_defined ^ is_ifundef;
  cond_stack_.push_back (std::make_pair (condition, line_number_));
}

void
Parser::f_cond_else ()
{
  if (cond_stack_.empty ())
    {
      parse_error () << "parse error, unexpected [ELSE]" << std::endl;
      return;
    }

  cond_stack_.back ().first = !cond_stack_.back ().first;
}

void
Parser::f_cond_then_endif (bool is_endif)
{
  if (cond_stack_.empty ())
    {
      const std::string s = is_endif ? "[ENDIF]" : "[THEN]";

      parse_error () << "parse error, unexpected " << s << std::endl;
      return;
    }

  cond_stack_.pop_back ();
}

void
Parser::f_end_of_file ()
{
  if (!cond_stack_.empty ())
    {
      parse_error () << "unterminated [IFDEF] or [IFNDEF]"
          << ", from line " << cond_stack_.back ().second << std::endl;
    }
}

void
Parser::f_definedvariable (const std::string & s)
{
  if (const Symbol * symbol = symtable_->lookup (s))
    {
      if (symbol->is_external_symbol ())
        {
          parse_error () << "symbol '" << s << "' already implicitly external"
              << ", from line " << symbol->get_line () << std::endl;
        }
      else
        {
          parse_error () << "multiple symbol definition '" << s
              << "', from line " << symbol->get_line () << std::endl;
        }
      return;
    }

  new VariableSymbol (s, line_number_, *symtable_);
}

void
Parser::f_definedarray (const std::string & s,
                        const std::string & ssize, const std::string & type)
{
  std::istringstream ins (s);

  std::string ss, sssize, stype;
  ins >> ss;

  if (const Symbol * symbol = symtable_->lookup (ss))
    {
      if (symbol->is_external_symbol ())
        {
          parse_error () << "symbol '" << ss << "' already implicitly external"
              << ", from line " << symbol->get_line () << std::endl;
        }
      else
        {
          parse_error () << "multiple symbol definition '" << ss
              << "', from line " << symbol->get_line () << std::endl;
        }
      return;
    }

  const int size = to_int (ssize);
  if (errno == ERANGE)
    {
      std::istringstream ins2 (ssize);
      ins2 >> sssize;

      parse_error ()
          << "array size '" << sssize << "' out of range" << std::endl;
      return;
    }
  if (size <= 0)
    {
      parse_error ()
          << "array '" << ss << "' cannot have size zero or less" << std::endl;
      return;
    }

  std::istringstream ins2 (type);
  ins2 >> stype;
  stype = to_lower (stype);

  if (stype != "chars" && stype != "cells")
    {
      parse_error ()
          << "parse error, expecting 'CHARS' or 'CELLS'" << std::endl;
      return;
    }

  if (stype == "chars")
    new CharacterArraySymbol (ss, size, line_number_, *symtable_);
  else
    new CellArraySymbol (ss, size, line_number_, *symtable_);
}

void
Parser::f_definedarray (const std::string & s, const std::string & ssize)
{
  f_definedarray (s, ssize, "chars");
}

void
Parser::f_definedconstant (const std::string & s)
{
  if (const Symbol * symbol = symtable_->lookup (s))
    {
      if (symbol->is_external_symbol ())
        {
          parse_error () << "symbol '" << s << "' already implicitly external"
              << ", from line " << symbol->get_line () << std::endl;
        }
      else
        {
          parse_error () << "multiple symbol definition '" << s
              << "', from line " << symbol->get_line () << std::endl;
        }
      return;
    }

  const Symbol * symbol = new IntConstantSymbol (s, line_number_, *symtable_);
  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new StoreSymbolIndirectOpcode (Register::R0, symbol, line_number_, optable_);
}

void
Parser::f_definedfconstant (const std::string & s)
{
  if (const Symbol * symbol = symtable_->lookup (s))
    {
      if (symbol->is_external_symbol ())
        {
          parse_error () << "symbol '" << s << "' already implicitly external"
              << ", from line " << symbol->get_line () << std::endl;
        }
      else
        {
          parse_error () << "multiple symbol definition '" << s
              << "', from line " << symbol->get_line () << std::endl;
        }
      return;
    }

  const Symbol * symbol = new FloatConstantSymbol (s, line_number_, *symtable_);
  detect_main ();
  new PopOpcode (Register::R0, Stack::FLOAT, line_number_, optable_);
  new StoreSymbolIndirectOpcode (Register::R0, symbol, line_number_, optable_);
}

void
Parser::f_definedword (const std::string & s)
{
  bool is_definable = true;
  if (const Symbol * symbol = symtable_->lookup (s))
    {
      if (symbol->is_external_symbol ())
        {
          parse_error () << "symbol '" << s << "' already implicitly external"
              << ", from line " << symbol->get_line () << std::endl;
        }
      else
        {
          parse_error () << "multiple symbol definition '" << s
              << "', from line " << symbol->get_line () << std::endl;
        }
      is_definable = false;
    }

  exit_label_ = label_sequence_;
  label_stack_.push (Label (label_sequence_++));

  if (!is_definable)
    return;

  DefinableSymbol * symbol = new WordSymbol (s, line_number_, *symtable_);
  current_definition_ = symbol;
  new DefineOpcode (symbol, line_number_, optable_);
}

void
Parser::f_nonameword ()
{
  exit_label_ = label_sequence_;
  label_stack_.push (Label (label_sequence_++));

  DefinableSymbol * symbol = new AnonwordSymbol (line_number_, *symtable_);
  current_definition_ = symbol;
  new DefineOpcode (symbol, line_number_, optable_);
}

void
Parser::f_semicolon ()
{
  assert (!label_stack_.empty ());
  const Label label = label_stack_.top ();
  label_stack_.pop ();

  exit_label_ = 0;

  if (!current_definition_)
    return;

  new LabelOpcode (label, line_number_, optable_);
  new EndDefineOpcode (current_definition_, line_number_, optable_);

  const Symbol * symbol = current_definition_;
  current_definition_ = 0;

  if (symbol->is_anonword_symbol ())
    {
      detect_main ();
      new LoadSymbolOpcode (Register::R0, symbol, line_number_, optable_);
      new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
    }
}

void
Parser::f_exit ()
{
  detect_main ();
  new JumpOpcode (Label (exit_label_), line_number_, optable_);
}

void
Parser::f_pstring (const std::string & s)
{
  const CallableSymbol * csymbol;
  if (const Symbol * symbol = symtable_->lookup ("_type"))
    {
      csymbol = symbol->is_callable_symbol ();
      if (!csymbol)
        {
          parse_error () << "'_TYPE' is defined as a variable"
              << "', from line " << symbol->get_line () << std::endl;
        }
    }
  else
    csymbol = new ExternalSymbol ("_type", line_number_, *symtable_);

  std::string ss = s;
  ss.erase (ss.begin (), ss.begin () + 3);
  ss.erase (ss.end () - 1);
  const Data * data = new StringData (ss.size (),
                                      ss.c_str (), line_number_, *dattable_);

  detect_main ();
  new LoadDataOpcode (Register::R0, data, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  new LoadValueOpcode (Register::R0,
                       Value (ss.size ()), line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  new CallOpcode (csymbol, line_number_, optable_);
}

void
Parser::f_cstring (const std::string & s)
{
  std::string ss = s;
  ss.erase (ss.begin (), ss.begin () + 3);
  ss.erase (ss.end () - 1);

  if (ss.size () > UCHAR_MAX)
    {
      parse_error () << "string is longer than "
          << UCHAR_MAX << " characters" << std::endl;
      return;
    }

  const Data * data = new CountedStringData (ss.size (), ss.c_str (),
                                             line_number_, *dattable_);

  detect_main ();
  new LoadDataOpcode (Register::R0, data, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
}

void
Parser::f_astring (const std::string & s)
{
  const CallableSymbol * csymbol;
  if (const Symbol * symbol = symtable_->lookup ("_abortquote"))
    {
      csymbol = symbol->is_callable_symbol ();
      if (!csymbol)
        {
          parse_error () << "'_ABORTQUOTE' is defined as a variable"
              << "', from line " << symbol->get_line () << std::endl;
        }
    }
  else
    csymbol = new ExternalSymbol ("_abortquote", line_number_, *symtable_);

  std::string ss = s;
  ss.erase (ss.begin (), ss.begin () + 7);
  ss.erase (ss.end () - 1);
  ss.append (1, '\n');
  const Data * data = new StringData (ss.size (),
                                      ss.c_str (), line_number_, *dattable_);

  Label no_abort_label = Label (label_sequence_++);

  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new JumpZeroOpcode (Register::R0,
                      no_abort_label, line_number_, optable_);

  new LoadDataOpcode (Register::R0, data, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  new LoadValueOpcode (Register::R0,
                       Value (ss.size ()), line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  new CallOpcode (csymbol, line_number_, optable_);

  new LabelOpcode (no_abort_label, line_number_, optable_);
}

void
Parser::f_dotparenstring (const std::string & s)
{
  std::string ss = s;
  ss.erase (ss.begin (), ss.begin () + 3);
  ss.erase (ss.end () - 1);

  std::cerr << source_path_ << ':'
            << line_number_ << ": note: " << ss << std::endl;
}

void
Parser::f_word (const std::string & s)
{
  detect_main ();

  const Symbol * symbol = symtable_->lookup (s);
  if (!symbol)
    symbol = new ExternalSymbol (s, line_number_, *symtable_);

  if (symbol->is_variable_symbol ())
    {
      new LoadSymbolOpcode (Register::R0, symbol, line_number_, optable_);
      new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
      return;
    }

  if (symbol->is_intconstant_symbol ())
    {
      new LoadSymbolIndirectOpcode (Register::R0,
                                    symbol, line_number_, optable_);
      new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
      return;
    }

  if (symbol->is_floatconstant_symbol ())
    {
      new LoadSymbolIndirectOpcode (Register::R0,
                                    symbol, line_number_, optable_);
      new PushOpcode (Register::R0, Stack::FLOAT, line_number_, optable_);
      return;
    }

  const CallableSymbol * csymbol = symbol->is_callable_symbol ();
  if (csymbol)
    {
      new CallOpcode (csymbol, line_number_, optable_);
      return;
    }

  parse_error () << "internal error: invalid symbol type" << std::endl;
}

void
Parser::f_object_address (const std::string & s)
{
  detect_main ();

  std::istringstream ins (s);

  std::string junk, ss;
  ins >> junk >> ss;

  const Symbol * symbol = symtable_->lookup (ss);
  if (!symbol)
    symbol = new ExternalSymbol (ss, line_number_, *symtable_);

  new LoadSymbolOpcode (Register::R0, symbol, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
}

void
Parser::f_char_literal (const std::string & s)
{
  std::istringstream ins (s);

  std::string junk, literal;
  if (s[0] == '\'')
    literal = s.substr (1, 1);
  else
    ins >> junk >> literal;

  if (literal.size () > 1)
    {
      parse_warning ()
          << "char literal '" << literal << "' >1 character" << std::endl;
    }

  detect_main ();
  new LoadValueOpcode (Register::R0,
                       Value (literal[0]), line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
}

void
Parser::f_integer_literal (const std::string & s)
{
  if (INT_MIN != (1 << 31))
    parse_error () << "internal error: int is not four bytes" << std::endl;

  union {
    int i;
    unsigned int u;
  } literal;

  literal.i = to_int (s);
  if (errno == ERANGE)
    {
      if (s[0] == '-')
        {
          parse_error ()
              << "integer literal '" << s << "' out of range" << std::endl;
          return;
        }
      literal.u = to_uint (s);
      if (errno == ERANGE)
        {
          parse_error ()
              << "integer literal '" << s << "' out of range" << std::endl;
          return;
        }
    }

  detect_main ();
  new LoadValueOpcode (Register::R0,
                       Value (literal.i), line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
}

void
Parser::f_float_literal (const std::string & s)
{
  if (sizeof (float) != sizeof (int))
    parse_error () << "internal error: float and int sizes differ" << std::endl;

  union {
    int i;
    float f;
  } literal;

  errno = 0;
  literal.f = std::strtof (s.c_str (), 0);
  if (errno == ERANGE)
    {
      parse_error ()
          << "float literal '" << s << "' out of range" << std::endl;
      return;
    }

  detect_main ();
  new LoadValueOpcode (Register::R0,
                       Value (literal.i), line_number_, optable_);
  new PushOpcode (Register::R0, Stack::FLOAT, line_number_, optable_);
}

void
Parser::f_string_literal (const std::string & s)
{
  std::string ss = s;
  ss.erase (ss.begin (), ss.begin () + 3);
  ss.erase (ss.end () - 1);
  const Data * data = new StringData (ss.size (),
                                      ss.c_str (), line_number_, *dattable_);

  detect_main ();
  new LoadDataOpcode (Register::R0, data, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  new LoadValueOpcode (Register::R0,
                       Value (ss.size ()), line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);
}

void
Parser::f_if ()
{
  label_stack_.push (Label (label_sequence_++));

  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new JumpZeroOpcode (Register::R0,
                      label_stack_.top (), line_number_, optable_);
}

void
Parser::f_then_endif ()
{
  assert (!label_stack_.empty ());
  detect_main ();
  new LabelOpcode (label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();
}

void
Parser::f_else ()
{
  assert (!label_stack_.empty ());
  Label over_label = label_stack_.top ();
  label_stack_.pop ();

  label_stack_.push (Label (label_sequence_++));

  detect_main ();
  new JumpOpcode (label_stack_.top (), line_number_, optable_);
  new LabelOpcode (over_label, line_number_, optable_);
}

void
Parser::f_do ()
{
  detect_main ();
  new PopOpcode (Register::R1, Stack::DATA, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));

  new PushOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R1, Stack::RETURN, line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));

  new LabelOpcode (label_stack_.top (), line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));
  break_stack_.push (label_stack_.top ());
}

void
Parser::f_query_do ()
{
  detect_main ();
  new PopOpcode (Register::R1, Stack::DATA, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));

  new JumpEqualOpcode (Register::R0, Register::R1,
                       label_stack_.top (), line_number_, optable_);

  new PushOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R1, Stack::RETURN, line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));

  new LabelOpcode (label_stack_.top (), line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));
  break_stack_.push (label_stack_.top ());
}

void
Parser::f_loop ()
{
  assert (!label_stack_.empty ());
  Label end_label = label_stack_.top ();
  label_stack_.pop ();

  detect_main ();
  new PopOpcode (Register::R1, Stack::RETURN, line_number_, optable_);
  new AddValueOpcode (Register::R1, Value (1), line_number_, optable_);

  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);

  new PushOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R1, Stack::RETURN, line_number_, optable_);

  assert (!label_stack_.empty ());
  new SubtractRegisterOpcode (Register::R1,
                              Register::R0, line_number_, optable_);
  new JumpNonZeroOpcode (Register::R1,
                         label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();

  new LabelOpcode (end_label, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);

  assert (!label_stack_.empty ());
  new LabelOpcode (label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();

  assert (!break_stack_.empty ());
  break_stack_.pop ();
}

void
Parser::f_plus_loop ()
{
  assert (!label_stack_.empty ());
  Label end_label = label_stack_.top ();
  label_stack_.pop ();

  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new PopOpcode (Register::R1, Stack::RETURN, line_number_, optable_);
  new AddRegisterOpcode (Register::R1, Register::R0, line_number_, optable_);

  // Alternate handling depending on loop direction.
  Label reverse_label = Label (label_sequence_++);
  new JumpLessZeroOpcode (Register::R0, reverse_label, line_number_, optable_);

  // Forwards loop: continue if rs0 - rs1 < 0
  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R1, Stack::RETURN, line_number_, optable_);

  assert (!label_stack_.empty ());
  new SubtractRegisterOpcode (Register::R1,
                              Register::R0, line_number_, optable_);
  new JumpLessZeroOpcode (Register::R1,
                          label_stack_.top (), line_number_, optable_);
  new JumpOpcode (end_label, line_number_, optable_);

  // Reverse loop: continue if rs0 - rs1 >= 0
  new LabelOpcode (reverse_label, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PushOpcode (Register::R1, Stack::RETURN, line_number_, optable_);

  assert (!label_stack_.empty ());
  new SubtractRegisterOpcode (Register::R1,
                              Register::R0, line_number_, optable_);
  new JumpGreaterEqualZeroOpcode (Register::R1,
                                  label_stack_.top (), line_number_, optable_);

  label_stack_.pop ();

  new LabelOpcode (end_label, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::RETURN, line_number_, optable_);

  assert (!label_stack_.empty ());
  new LabelOpcode (label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();

  assert (!break_stack_.empty ());
  break_stack_.pop ();
}

void
Parser::f_leave ()
{
  if (break_stack_.empty ())
    {
      parse_error () << "'LEAVE' invalid outside a loop" << std::endl;
      return;
    }

  detect_main ();
  new JumpOpcode (break_stack_.top (), line_number_, optable_);
}

void
Parser::f_query_leave ()
{
  if (break_stack_.empty ())
    {
      parse_error () << "'?LEAVE' invalid outside a loop" << std::endl;
      return;
    }

  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new JumpNonZeroOpcode (Register::R0,
                         break_stack_.top (), line_number_, optable_);
}

void
Parser::f_begin ()
{
  detect_main ();
  label_stack_.push (Label (label_sequence_++));
  new LabelOpcode (label_stack_.top (), line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));
  break_stack_.push (label_stack_.top ());
}

void
Parser::f_until ()
{
  assert (!label_stack_.empty ());
  Label end_label = label_stack_.top ();
  label_stack_.pop ();

  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  assert (!label_stack_.empty ());
  new JumpZeroOpcode (Register::R0,
                      label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();

  new LabelOpcode (end_label, line_number_, optable_);

  assert (!break_stack_.empty ());
  break_stack_.pop ();
}

void
Parser::f_while ()
{
  if (break_stack_.empty ())
    {
      parse_error () << "'WHILE' invalid outside a loop" << std::endl;
      return;
    }

  detect_main ();
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new JumpZeroOpcode (Register::R0,
                      break_stack_.top (), line_number_, optable_);
}

void
Parser::f_repeat_again ()
{
  assert (!label_stack_.empty ());
  Label end_label = label_stack_.top ();
  label_stack_.pop ();

  detect_main ();
  assert (!label_stack_.empty ());
  new JumpOpcode (label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();

  new LabelOpcode (end_label, line_number_, optable_);

  assert (!break_stack_.empty ());
  break_stack_.pop ();
}

void
Parser::f_case ()
{
  detect_main ();
  label_stack_.push (Label (label_sequence_++));
  break_stack_.push (label_stack_.top ());
}

void
Parser::f_of ()
{
  detect_main ();
  new PopOpcode (Register::R1, Stack::DATA, line_number_, optable_);
  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
  new PushOpcode (Register::R0, Stack::DATA, line_number_, optable_);

  label_stack_.push (Label (label_sequence_++));

  new JumpNotEqualOpcode (Register::R0, Register::R1,
                          label_stack_.top (), line_number_, optable_);
}

void
Parser::f_endof ()
{
  detect_main ();
  assert (!break_stack_.empty ());
  new JumpOpcode (break_stack_.top (), line_number_, optable_);

  assert (!label_stack_.empty ());
  new LabelOpcode (label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();
}

void
Parser::f_endcase ()
{
  detect_main ();

  assert (!label_stack_.empty ());
  new LabelOpcode (label_stack_.top (), line_number_, optable_);
  label_stack_.pop ();

  assert (!break_stack_.empty ());
  break_stack_.pop ();

  new PopOpcode (Register::R0, Stack::DATA, line_number_, optable_);
}

void
Parser::f_recurse ()
{
  if (!current_definition_)
    {
      parse_error () << "'RECURSE' invalid outside a definition" << std::endl;
      return;
    }

  detect_main ();
  new CallOpcode (current_definition_, line_number_, optable_);
}

void
Parser::f_codeword (const std::string & s)
{
  if (const Symbol * symbol = symtable_->lookup (s))
    {
      if (symbol->is_external_symbol ())
        {
          parse_error () << "symbol '" << s << "' already implicitly external"
              << ", from line " << symbol->get_line () << std::endl;
        }
      else
        {
          parse_error () << "multiple symbol definition '" << s
              << "', from line " << symbol->get_line () << std::endl;
        }
      return;
    }

  DefinableSymbol * symbol = new CodewordSymbol (s, line_number_, *symtable_);
  current_definition_ = symbol;
  new DefineOpcode (symbol, line_number_, optable_);
}

void
Parser::f_code (const std::string & s)
{
  static const std::string & comments = "\\#";

  std::string ss = s;

  for (std::string::size_type i = 0; i < ss.size (); ++i)
    {
      if (comments.find (ss.at (i)) != comments.npos)
        {
          ss.erase (i);
          break;
        }
      if (ss.at (i) == '"')
        {
          for (++i; i < ss.size () && ss.at (i) != '"'; ++i)
            {
              if (ss.at (i) == '\\')
                ++i;
            }
        }
    }
  ss.erase (ss.find_last_not_of (" \t") + 1);

  if (!ss.empty ())
    new AssemblyOpcode (ss, line_number_, optable_);
}

void
Parser::f_end_code ()
{
  if (current_definition_)
    {
      new EndDefineOpcode (current_definition_, line_number_, optable_);
      current_definition_ = 0;
    }
}

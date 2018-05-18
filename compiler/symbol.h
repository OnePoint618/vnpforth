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

#ifndef VNPFORTH_SYMBOL_H
#define VNPFORTH_SYMBOL_H

#include <string>

#include "mangler.h"
#include "util.h"

class SymbolTable;
class Options;
class VariableSymbol;
class ConstantSymbol;
class IntConstantSymbol;
class FloatConstantSymbol;
class CallableSymbol;
class ExternalSymbol;
class DefinableSymbol;
class WordSymbol;
class AnonwordSymbol;
class CodewordSymbol;

// Abstract symbol class, assigns a unique sequential id on construction.
class Symbol
{
public:
  virtual ~Symbol ();

  inline const std::string
  get_name () const
  {
    return name_;
  }

  inline int
  get_id () const
  {
    return id_;
  }

  inline int
  get_line () const
  {
    return line_;
  }

  virtual const std::string create_list_entry () const;

  virtual void generate (std::ostream & outs, const Options & options) const;

  virtual inline const VariableSymbol *
  is_variable_symbol () const
  {
    return 0;
  }

  virtual inline const IntConstantSymbol *
  is_intconstant_symbol () const
  {
    return 0;
  }

  virtual inline const FloatConstantSymbol *
  is_floatconstant_symbol () const
  {
    return 0;
  }

  virtual inline const CallableSymbol *
  is_callable_symbol () const
  {
    return 0;
  }

  virtual inline const ExternalSymbol *
  is_external_symbol () const
  {
    return 0;
  }

  virtual inline const DefinableSymbol *
  is_definable_symbol () const
  {
    return 0;
  }

  virtual inline const WordSymbol *
  is_word_symbol () const
  {
    return 0;
  }

  virtual inline const AnonwordSymbol *
  is_anonword_symbol () const
  {
    return 0;
  }

  virtual inline const CodewordSymbol *
  is_codeword_symbol () const
  {
    return 0;
  }

protected:
  Symbol (const std::string & name,
          int line, const std::string & type, SymbolTable & symtable);

  inline const std::string
  get_type () const
  {
    return type_;
  }

  static int count;
  static int instances;

private:
  const std::string name_;
  const int id_;
  const int line_;
  const std::string type_;
};

// Concrete symbol classes.
class VariableSymbol: public Symbol
{
public:
  VariableSymbol (const std::string & name,
                  int line, SymbolTable & symtable)
    : Symbol (name, line, "variable", symtable), size_ (1), units_ (CELL) { }

  enum Units { CHARACTER, CELL };

  const std::string create_list_entry () const;

  void generate (std::ostream & outs, const Options & options) const;

  inline const VariableSymbol *
  is_variable_symbol () const
  {
    return this;
  }

protected:
  VariableSymbol (const std::string & name,
                  size_t size, Units units, int line, SymbolTable & symtable)
    : Symbol (name, line, "variable", symtable),
      size_ (size), units_ (units) { }

private:
  const size_t size_;
  const Units units_;
};

class CellArraySymbol: public VariableSymbol
{
public:
  CellArraySymbol (const std::string & name,
                   size_t size, int line, SymbolTable & symtable)
    : VariableSymbol (name, size, CELL, line, symtable) { }
};

class CharacterArraySymbol: public VariableSymbol
{
public:
  CharacterArraySymbol (const std::string & name,
                        size_t size, int line, SymbolTable & symtable)
    : VariableSymbol (name, size, CHARACTER, line, symtable) { }
};

// Abstract symbol for constants.
class ConstantSymbol: public Symbol
{
public:
  void generate (std::ostream & outs, const Options & options) const;

protected:
  ConstantSymbol (const std::string & name,
                  int line, const std::string & type, SymbolTable & symtable)
      : Symbol (name, line, type, symtable) { }
};

// Concrete constant symbols.
class IntConstantSymbol: public ConstantSymbol
{
public:
  IntConstantSymbol (const std::string & name,
                     int line, SymbolTable & symtable)
    : ConstantSymbol (name, line, "constant", symtable) { }

  inline const IntConstantSymbol *
  is_intconstant_symbol () const
  {
    return this;
  }
};

class FloatConstantSymbol: public ConstantSymbol
{
public:
  FloatConstantSymbol (const std::string & name,
                       int line, SymbolTable & symtable)
    : ConstantSymbol (name, line, "fconstant", symtable) { }

  inline const FloatConstantSymbol *
  is_floatconstant_symbol () const
  {
    return this;
  }
};

// Abstract symbol for objects suitable for function calls.
class CallableSymbol: public Symbol
{
protected:
  CallableSymbol (const std::string & name,
                  int line, const std::string & type, SymbolTable & symtable)
    : Symbol (name, line, type, symtable) { }

  inline const CallableSymbol *
  is_callable_symbol () const
  {
    return this;
  }
};

// Concrete callable symbol classes.
class ExternalSymbol: public CallableSymbol
{
public:
  ExternalSymbol (const std::string & name, int line, SymbolTable & symtable)
    : CallableSymbol (name, line, "external", symtable) { }

  inline const ExternalSymbol *
  is_external_symbol () const
  {
    return this;
  }
};

class DefinableSymbol: public CallableSymbol
{
protected:
  DefinableSymbol (const std::string & name,
                   int line, const std::string & type, SymbolTable & symtable)
    : CallableSymbol (name, line, type, symtable) { }

  inline const DefinableSymbol *
  is_definable_symbol () const
  {
    return this;
  }
};

class WordSymbol: public DefinableSymbol
{
public:
  WordSymbol (const std::string & name, int line, SymbolTable & symtable)
    : DefinableSymbol (name, line, "word", symtable) { }

  inline const WordSymbol *
  is_word_symbol () const
  {
    return this;
  }
};

class AnonwordSymbol: public DefinableSymbol
{
public:
  AnonwordSymbol (int line, SymbolTable & symtable)
    : DefinableSymbol (std::string ("__")
                       + Mangler::mangle ("") + to_string (count),
                       line, "anonword", symtable) { }

  inline const AnonwordSymbol *
  is_anonword_symbol () const
  {
    return this;
  }
};

class CodewordSymbol: public DefinableSymbol
{
public:
  CodewordSymbol (const std::string & name, int line, SymbolTable & symtable)
    : DefinableSymbol (name, line, "codeword", symtable) { }

  inline const CodewordSymbol *
  is_codeword_symbol () const
  {
    return this;
  }
};

#endif

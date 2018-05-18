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

#ifndef VNPFORTH_OPCODE_H
#define VNPFORTH_OPCODE_H

#include <string>

#include "operand.h"
#include "register.h"
#include "stack.h"
#include "symbol.h"

class Data;
class OpcodeTable;
class Options;
class CallOpcode;
class BranchingOpcode;
class JumpOpcode;
class NoOpOpcode;
class LabelOpcode;
class PushOpcode;
class PopOpcode;
class DefineOpcode;
class EndDefineOpcode;
class AssemblyOpcode;

// Abstract opcode class, includes line number, and a pointer to any other
// opcode inserted to optimize this one away.
class Opcode
{
public:
  virtual ~Opcode () { }

  const std::string create_list_entry () const;

  inline void
  set_optimizes (Opcode * optimizes)
  {
    // This may drop an existing pointer to an optimizing opcode.  However,
    // all opcodes are collected by the opcode table and deleted by it on
    // clear or destruction, so this is not a leak.
    optimizes_ = optimizes;
  }

  inline int
  get_line () const
  {
    return line_;
  }

  virtual void generate (std::ostream & outs,
                         const Options & options) const = 0;

  virtual inline const CallOpcode *
  is_call_opcode () const
  {
    return 0;
  }

  virtual inline const BranchingOpcode *
  is_branching_opcode () const
  {
    return 0;
  }

  virtual inline const JumpOpcode *
  is_jump_opcode () const
  {
    return 0;
  }

  virtual inline const NoOpOpcode *
  is_noop_opcode () const
  {
    return 0;
  }

  virtual inline const LabelOpcode *
  is_label_opcode () const
  {
    return 0;
  }

  virtual inline const PushOpcode *
  is_push_opcode () const
  {
    return 0;
  }

  virtual inline const PopOpcode *
  is_pop_opcode () const
  {
    return 0;
  }

  virtual inline const DefineOpcode *
  is_define_opcode () const
  {
    return 0;
  }

  virtual inline const EndDefineOpcode *
  is_enddefine_opcode () const
  {
    return 0;
  }

  virtual inline const AssemblyOpcode *
  is_assembly_opcode () const
  {
    return 0;
  }

protected:
  Opcode (int line, OpcodeTable * optable = 0);

  virtual const std::string create_list_specific () const = 0;

private:
  static int count;
  static int instances;

  const int line_;
  const Opcode * optimizes_;
};

// Function definition and calls, and label opcode.
class CallOpcode: public Opcode
{
public:
  CallOpcode (const CallableSymbol * symbol,
              int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), symbol_ (symbol) { }

  inline const Symbol *
  get_symbol () const
  {
    return symbol_;
  }

  void generate (std::ostream & outs, const Options & options) const;

  inline const CallOpcode *
  is_call_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const Symbol * symbol_;
};

class DefineOpcode: public Opcode
{
public:
  DefineOpcode (const DefinableSymbol * symbol,
                int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), symbol_ (symbol) { }

  inline const Symbol *
  get_symbol () const
  {
    return symbol_;
  }

  void generate (std::ostream & outs, const Options & options) const;

  inline const DefineOpcode *
  is_define_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const Symbol * symbol_;
};

class EndDefineOpcode: public Opcode
{
public:
  EndDefineOpcode (const DefinableSymbol * symbol,
                   int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), symbol_ (symbol) { }

  void generate (std::ostream & outs, const Options & options) const;

  inline const EndDefineOpcode *
  is_enddefine_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const Symbol * symbol_;
};

class LabelOpcode: public Opcode
{
public:
  LabelOpcode (const Label & label, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), label_ (label) { }

  inline const Label &
  get_label () const
  {
    return label_;
  }

  void generate (std::ostream & outs, const Options & options) const;

  inline const LabelOpcode *
  is_label_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const Label label_;
};

// Assembly language opcode.
class AssemblyOpcode: public Opcode
{
public:
  AssemblyOpcode (const std::string & assembly,
                  int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), assembly_ (assembly) { }

  ~AssemblyOpcode () { }

  void generate (std::ostream & outs, const Options & options) const;

  inline const AssemblyOpcode *
  is_assembly_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const std::string assembly_;
};

// Register load and store.
class LoadValueOpcode: public Opcode
{
public:
  LoadValueOpcode (const Register & reg,
                   const Value value, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), value_ (value) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Value value_;
};

class LoadRegisterOpcode: public Opcode
{
public:
  LoadRegisterOpcode (const Register & reg, const Register & with,
                      int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), with_ (with) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Register with_;
};

class LoadSymbolOpcode: public Opcode
{
public:
  LoadSymbolOpcode (const Register & reg,
                    const Symbol * symbol, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), symbol_ (symbol) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Symbol * symbol_;
};

class LoadSymbolIndirectOpcode: public Opcode
{
public:
  LoadSymbolIndirectOpcode (const Register & reg, const Symbol * symbol,
                            int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), symbol_ (symbol) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Symbol * symbol_;
};

class LoadDataOpcode: public Opcode
{
public:
  LoadDataOpcode (const Register & reg,
                  const Data * data, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), data_ (data) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Data * data_;
};

class StoreSymbolIndirectOpcode: public Opcode
{
public:
  StoreSymbolIndirectOpcode (const Register & reg, const Symbol * symbol,
                             int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), symbol_ (symbol) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Symbol * symbol_;
};

class PushOpcode: public Opcode
{
public:
  PushOpcode (const Register & reg,
              const Stack & stack, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), stack_ (stack) { }

  inline const Register &
  get_register () const
  {
    return reg_;
  }

  inline const Stack &
  get_stack () const
  {
    return stack_;
  }

  void generate (std::ostream & outs, const Options & options) const;

  inline const PushOpcode *
  is_push_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Stack stack_;
};

class PopOpcode: public Opcode
{
public:
  PopOpcode (const Register & reg,
             const Stack & stack, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), stack_ (stack) { }

  inline const Register &
  get_register () const
  {
    return reg_;
  }

  inline const Stack &
  get_stack () const
  {
    return stack_;
  }

  void generate (std::ostream & outs, const Options & options) const;

  inline const PopOpcode *
  is_pop_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Stack stack_;
};

// Add and subtract.
class AddValueOpcode: public Opcode
{
public:
  AddValueOpcode (const Register & reg,
                  const Value value, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), value_ (value) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Value value_;
};

class AddRegisterOpcode: public Opcode
{
public:
  AddRegisterOpcode (const Register & reg, const Register & increment,
                     int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), increment_ (increment) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Register increment_;
};

class SubtractValueOpcode: public Opcode
{
public:
  SubtractValueOpcode (const Register & reg,
                       const Value value, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), value_ (value) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Value value_;
};

class SubtractRegisterOpcode: public Opcode
{
public:
  SubtractRegisterOpcode (const Register & reg, const Register & decrement,
                          int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), reg_ (reg), decrement_ (decrement) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
  const Register decrement_;
};

// Branches.
class BranchingOpcode: public Opcode
{
public:
  inline const BranchingOpcode *
  is_branching_opcode () const
  {
    return this;
  }

  inline const Label &
  get_label () const
  {
    return label_;
  }

protected:
  BranchingOpcode (const Label & label, int line, OpcodeTable * optable = 0)
    : Opcode (line, optable), label_ (label) { }

  const Label label_;
};

class JumpOpcode: public BranchingOpcode
{
public:
  JumpOpcode (const Label & label, int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable) { }

  void generate (std::ostream & outs, const Options & options) const;

  inline const JumpOpcode *
  is_jump_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
};

class JumpZeroOpcode: public BranchingOpcode
{
public:
  JumpZeroOpcode (const Register & reg,
                  const Label & label, int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg_ (reg) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
};

class JumpNonZeroOpcode: public BranchingOpcode
{
public:
  JumpNonZeroOpcode (const Register & reg,
                     const Label & label, int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg_ (reg) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
};

class JumpGreaterZeroOpcode: public BranchingOpcode
{
public:
  JumpGreaterZeroOpcode (const Register & reg, const Label & label,
                         int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg_ (reg) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
};

class JumpLessZeroOpcode: public BranchingOpcode
{
public:
  JumpLessZeroOpcode (const Register & reg, const Label & label,
                      int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg_ (reg) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
};

class JumpGreaterEqualZeroOpcode: public BranchingOpcode
{
public:
  JumpGreaterEqualZeroOpcode (const Register & reg, const Label & label,
                              int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg_ (reg) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
};

class JumpLessEqualZeroOpcode: public BranchingOpcode
{
public:
  JumpLessEqualZeroOpcode (const Register & reg, const Label & label,
                           int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg_ (reg) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg_;
};

class JumpEqualOpcode: public BranchingOpcode
{
public:
  JumpEqualOpcode (const Register & reg1, const Register & reg2,
                   const Label & label, int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg1_ (reg1), reg2_ (reg2) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg1_;
  const Register reg2_;
};

class JumpNotEqualOpcode: public BranchingOpcode
{
public:
  JumpNotEqualOpcode (const Register & reg1, const Register & reg2,
                      const Label & label, int line, OpcodeTable * optable = 0)
    : BranchingOpcode (label, line, optable), reg1_ (reg1), reg2_ (reg2) { }

  void generate (std::ostream & outs, const Options & options) const;

protected:
  const std::string create_list_specific () const;
private:
  const Register reg1_;
  const Register reg2_;
};

// No-op, used for optimizing out opcodes.
class NoOpOpcode: public Opcode
{
public:
  NoOpOpcode (int line, OpcodeTable * optable = 0)
    : Opcode (line, optable) { }

  void generate (std::ostream & outs, const Options & options) const;

  inline const NoOpOpcode *
  is_noop_opcode () const
  {
    return this;
  }

protected:
  const std::string create_list_specific () const;
};

#endif

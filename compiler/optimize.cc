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
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "mangler.h"
#include "opcode.h"
#include "operand.h"
#include "optable.h"
#include "register.h"
#include "stack.h"

// Definition of "small" function, limit on optimization iterations, and
// pre-defined mangled names for true/false inlining.
namespace {

const int SMALL_FUNCTION_OPCODE_LIMIT = 10;
const int ITERATIONS_LIMIT = 32;

static const std::string MANGLED_TRUE = Mangler::mangle ("TRUE");
static const std::string MANGLED_FALSE = Mangler::mangle ("FALSE");

} // namespace

// Convenience functions to replace an opcode with something else.
void
OpcodeTable::replace_with (OpcodeTableIterator_mutable iter,
                           Opcode * replacement)
{
  replacement->set_optimizes (*iter);
  *iter = replacement;
  opcode_collection_.insert (replacement);
}

void
OpcodeTable::replace_with_nop (OpcodeTableIterator_mutable iter)
{
  const Opcode * opcode = *iter;
  replace_with (iter, new NoOpOpcode (opcode->get_line ()));
}

// Remove unreachable code that can never be executed.
int
OpcodeTable::remove_unreachable_code ()
{
  int optimizations = 0;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); )
    {
      // Find the next unconditional jump.
      const OpcodeTableIterator_mutable first =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_jump_opcode));
      if (first == opcodes_.end ())
        break;

      // Replace any non-no-op between here and the next label with a no-op.
      // Remember to break on definition end.
      for (iter = first + 1; iter != opcodes_.end (); ++iter)
        {
          const Opcode * opcode = *iter;

          if (opcode->is_label_opcode () || opcode->is_enddefine_opcode ())
            break;
          else if (!opcode->is_noop_opcode ())
            {
              replace_with_nop (iter);
              ++optimizations;
            }
        }
    }

    return optimizations;
}

// Replace jumps over nothing or to the next instruction with a no-op.
int
OpcodeTable::remove_unnecessary_jumps ()
{
  int optimizations = 0;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); )
    {
      // Find the next jump, conditional or otherwise.
      const OpcodeTableIterator_mutable first =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_branching_opcode));
      if (first == opcodes_.end ())
        break;

      // Find the non-no-op after the jump.
      const OpcodeTableIterator_mutable second =
          std::find_if (first + 1, opcodes_.end (),
                        std::not1 (std::mem_fun (&Opcode::is_noop_opcode)));
      if (second == opcodes_.end ())
        break;

      const BranchingOpcode * jump = (*first)->is_branching_opcode ();
      const LabelOpcode * label = (*second)->is_label_opcode ();

      // If we found a label and it matches the target of the jump we
      // can optimize away the jump.
      if (label && jump->get_label ().equals (label->get_label ()))
        {
          replace_with_nop (first);
          ++optimizations;
        }

      iter = second;
    }

  return optimizations;
}

// Replace calls to empty functions with a no-op.
int
OpcodeTable::remove_useless_calls ()
{
  int optimizations = 0;

  // Identify definitions that contain no executable opcodes.
  std::set<const Symbol *> empty_functions;
  const Symbol * current_definition = 0;
  bool has_executable_opcodes = false;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      const Opcode * opcode = *iter;

      if (opcode->is_define_opcode ())
        {
          current_definition = opcode->is_define_opcode ()->get_symbol ();
          has_executable_opcodes = false;
        }
      else if (opcode->is_enddefine_opcode ())
        {
          if (!has_executable_opcodes)
            empty_functions.insert (current_definition);

          current_definition = 0;
        }
      else if (current_definition)
        {
          const bool is_executable = !(opcode->is_label_opcode ()
                                       || opcode->is_noop_opcode ());
          has_executable_opcodes |= is_executable;
        }
    }

  // Replace calls to any empty definition with a no-op.
  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); )
    {
      // Find the next call opcode.
      const OpcodeTableIterator_mutable call_iter =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_call_opcode));
      if (call_iter == opcodes_.end ())
        break;

      const CallOpcode * call = (*call_iter)->is_call_opcode ();
      const Symbol * symbol = call->get_symbol ();

      // If the call is to an empty definition, replace it with a no-op.
      if (empty_functions.find (symbol) != empty_functions.end ())
        {
          replace_with_nop (call_iter);
          ++optimizations;
        }

      iter = call_iter + 1;
    }

  return optimizations;
}

// Inline any 'true' or 'false' booleans.  Eliminates a call just to obtain
// a constant 0 or -1.
int
OpcodeTable::inline_booleans ()
{
  int optimizations = 0;

  // Build a new opcode vector with inlining.  Add a nop optimizing out
  // the boolean as an indicator we can find in the intermediate output file.
  OpcodeTableStore inlined;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      Opcode * opcode = *iter;

      const CallOpcode * call = opcode->is_call_opcode ();

      if (call)
        {
          const Symbol * symbol = call->get_symbol ();
          const std::string name = symbol->get_name ();

          if (name == MANGLED_TRUE || name == MANGLED_FALSE)
            {
              // Important: after replace_with_nop, opcode != *iter.
              replace_with_nop (iter);
              inlined.push_back (*iter);

              Opcode * op;
              op = new LoadValueOpcode (Register::R0,
                                        name == MANGLED_TRUE
                                                ? Value (-1) : Value (0),
                                        opcode->get_line ());
              opcode_collection_.insert (op);
              inlined.push_back (op);
              op = new PushOpcode (Register::R0,
                                   Stack::DATA, opcode->get_line ());
              opcode_collection_.insert (op);
              inlined.push_back (op);

              ++optimizations;
            }
          else
            inlined.push_back (opcode);
        }
      else
        inlined.push_back (opcode);
    }

  // Replace the old opcode vector with the new one.
  if (optimizations)
    opcodes_.assign (inlined.begin (), inlined.end ());

  return optimizations;
}

// Inline any small straight-line functions.
int
OpcodeTable::inline_small_functions ()
{
  int optimizations = 0;

  // Identify definitions that are straight-line basic-block only, and
  // count their executable opcodes.
  std::set<const Symbol *> inline_candidates;
  const Symbol * current_definition = 0;
  int executable_opcodes = 0;
  bool is_not_candidate = false;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      const Opcode * opcode = *iter;

      if (opcode->is_define_opcode ())
        {
          current_definition = opcode->is_define_opcode ()->get_symbol ();
          executable_opcodes = 0;
          is_not_candidate = false;
        }
      else if (opcode->is_enddefine_opcode ())
        {
          if (!is_not_candidate)
            inline_candidates.insert (current_definition);

          current_definition = 0;
        }
      else if (current_definition)
        {
          const bool is_branching = opcode->is_branching_opcode ();
          is_not_candidate |= is_branching;

          const bool is_assembly = opcode->is_assembly_opcode ();
          is_not_candidate |= is_assembly;

          const bool is_executable = !(opcode->is_label_opcode ()
                                       || opcode->is_noop_opcode ());
          executable_opcodes += is_executable ? 1 : 0;
          is_not_candidate |= executable_opcodes > SMALL_FUNCTION_OPCODE_LIMIT;
        }
    }

  // Build splices for small definitions.
  std::map<const Symbol *, OpcodeTableStore> splices;
  current_definition = 0;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      Opcode * opcode = *iter;

      if (opcode->is_define_opcode ())
        {
          const Symbol * defining = opcode->is_define_opcode ()->get_symbol ();
          if (inline_candidates.find (defining) != inline_candidates.end ())
            current_definition = defining;
        }
      else if (opcode->is_enddefine_opcode ())
        {
          current_definition = 0;
        }
      else if (current_definition)
        {
          const bool is_executable = !(opcode->is_label_opcode ()
                                       || opcode->is_noop_opcode ());
          if (is_executable)
            splices[current_definition].push_back (opcode);
        }
    }

  // Build a new opcode vector with inlining.  Add a nop optimizing out
  // the call as an indicator we can find in the intermediate output file.
  OpcodeTableStore inlined;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      Opcode * opcode = *iter;

      const CallOpcode * call = opcode->is_call_opcode ();

      if (call && splices.find (call->get_symbol ()) != splices.end ())
        {
          // Important: after replace_with_nop, opcode != *iter.
          replace_with_nop (iter);
          inlined.push_back (*iter);

          const Symbol * symbol = call->get_symbol ();
          inlined.insert (inlined.end (),
                          splices[symbol].begin (), splices[symbol].end ());

          ++optimizations;
        }
      else
        inlined.push_back (opcode);
    }

  // Replace the old opcode vector with the new one.
  if (optimizations)
    opcodes_.assign (inlined.begin (), inlined.end ());

  return optimizations;
}

// Replace amenable adjacent push-pop pairs with a shorter representation.
int
OpcodeTable::replace_adjacent_push_pop_pairs ()
{
  int optimizations = 0;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); )
    {
      // Find the next push opcode.
      const OpcodeTableIterator_mutable first =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_push_opcode));
      if (first == opcodes_.end ())
        break;

      // Find the non-no-op opcode following this one.
      const OpcodeTableIterator_mutable second =
          std::find_if (first + 1, opcodes_.end (),
                        std::not1 (std::mem_fun (&Opcode::is_noop_opcode)));
      if (second == opcodes_.end ())
        break;

      const PushOpcode * push = (*first)->is_push_opcode ();
      const PopOpcode * pop = (*second)->is_pop_opcode ();

      // If we have a push-pop pair then there may be something to optimize.
      // Replace if the pop pops the stack just pushed.
      if (pop && push->get_stack ().equals (pop->get_stack ()))
        {
          // Either eliminate or transform into register to register move.
          // Always eliminate the second opcode of the pair.
          Opcode * opcode;
          if (push->get_register ().equals (pop->get_register ()))
            opcode = new NoOpOpcode (push->get_line ());
          else
            opcode = new LoadRegisterOpcode (pop->get_register (),
                                             push->get_register (),
                                             push->get_line ());
          replace_with (first, opcode);
          replace_with_nop (second);

          ++optimizations;
        }

      iter = second;
    }

  return optimizations;
}

// Relocate any suboptimal labels to more optimal positions.
int
OpcodeTable::relocate_suboptimal_labels ()
{
  int optimizations = 0;

  // Find labels whose directly following opcode is an unconditional goto.
  // Handle only forwards branches to avoid possible infinite loop issues.
  // These are the common cases, arising from conditionals and loop leave.
  std::map<int, int> relocations;
  std::map<int, int> label_lines;
  const LabelOpcode * current_label = 0;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      Opcode * opcode = *iter;

      if (opcode->is_label_opcode ())
        {
          current_label = opcode->is_label_opcode ();

          const int value = current_label->get_label ().get_value ();
          label_lines[value] = opcode->get_line ();
        }

      else if (opcode->is_jump_opcode ())
        {
          if (current_label) {
            const JumpOpcode * jump_opcode = opcode->is_jump_opcode ();
            const Label & label = jump_opcode->get_label ();

            // Pick up only forwards branches.
            const int value = current_label->get_label ().get_value ();
            if (label.get_value () > value)
              relocations[value] = label.get_value ();
          }

          current_label = 0;
        }

      else if (!opcode->is_noop_opcode ())
        current_label = 0;
    }

  // Advance each suboptimal label as far forwards as possible.
  for (std::map<int, int>::const_iterator iter = relocations.begin ();
       iter != relocations.end (); ++iter)
    {
      const int value = iter->first;
      const int relocation = iter->second;

      int forwards = relocation;
      while (relocations.find (forwards) != relocations.end ())
        forwards = relocations[forwards];

      relocations[value] = forwards;
    }

  // Build splices to relocate these labels.
  std::map<int, OpcodeTableStore> splices;
  for (std::map<int, int>::const_iterator iter = relocations.begin ();
       iter != relocations.end (); ++iter)
    {
      const int value = iter->first;
      const int relocation = iter->second;

      Opcode * opcode = new LabelOpcode (Label (value), label_lines[value]);
      opcode_collection_.insert (opcode);
      splices[relocation].push_back (opcode);
    }

  // Build a new opcode vector with suboptimal labels relocated.  Leave a
  // no-op marker at the old location.
  OpcodeTableStore relocated;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); ++iter)
    {
      Opcode * opcode = *iter;

      const LabelOpcode * label = opcode->is_label_opcode ();
      if (label)
        {
          const int value = label->get_label ().get_value ();
          bool is_suboptimal_or_spliced = false;

          if (relocations.find (value) != relocations.end ())
            {
              // Important: after replace_with_nop, opcode != *iter.
              replace_with_nop (iter);
              relocated.push_back (*iter);
              is_suboptimal_or_spliced = true;
            }

          if (splices.find (value) != splices.end ())
            {
              relocated.insert (relocated.end (),
                                splices[value].begin (), splices[value].end ());
              relocated.push_back (opcode);
              is_suboptimal_or_spliced = true;
            }

          if (is_suboptimal_or_spliced)
            ++optimizations;
          else
            relocated.push_back (opcode);
        }
      else
        relocated.push_back (opcode);
    }

  // Replace the old opcode vector with the new one.
  if (optimizations)
    opcodes_.assign (relocated.begin (), relocated.end ());

  return optimizations;
}

// Remove any labels for which there is no jump.
int
OpcodeTable::remove_unnecessary_labels ()
{
  int optimizations = 0;

  // Identify all currently used labels.
  std::set<int> used_labels;

  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); )
    {
      // Find the next branching opcode.
      const OpcodeTableIterator_mutable first =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_branching_opcode));
      if (first == opcodes_.end ())
        break;

      const Opcode * opcode = *first;
      const BranchingOpcode * branch = opcode->is_branching_opcode ();

      const Label & label = branch->get_label ();
      used_labels.insert (label.get_value ());

      iter = first + 1;
    }

  // Optimize away any labels not referenced.
  for (OpcodeTableIterator_mutable iter = opcodes_.begin ();
       iter != opcodes_.end (); )
    {
      // Find the next label opcode.
      const OpcodeTableIterator_mutable first =
          std::find_if (iter, opcodes_.end (),
                        std::mem_fun (&Opcode::is_label_opcode));
      if (first == opcodes_.end ())
        break;

      const Opcode * opcode = *first;
      const LabelOpcode * label_op = opcode->is_label_opcode ();

      // If no branch uses it, replace it with a no-op.
      const Label & label = label_op->get_label ();
      if (used_labels.find (label.get_value ()) == used_labels.end ())
        {
          replace_with_nop (first);
          ++optimizations;
        }

      iter = first + 1;
    }

  return optimizations;
}

// Run all optimizers in sequence, and iterate until no more optimizations.
void
OpcodeTable::optimize_code (const std::string & source_path)
{
  int is_optimizing = 0;
  int iterations = 0;
  do
    {
      is_optimizing = remove_unreachable_code ()
                      + remove_unnecessary_jumps ()
                      + remove_useless_calls ()
                      + inline_booleans ()
                      + inline_small_functions ()
                      + replace_adjacent_push_pop_pairs ()
                      + relocate_suboptimal_labels ()
                      + remove_unnecessary_labels ();
      ++iterations;
    }
  while (is_optimizing && iterations < ITERATIONS_LIMIT);

  if (iterations >= ITERATIONS_LIMIT)
    {
      std::cerr << source_path
                << ": warning: optimization limit reached after "
                << ITERATIONS_LIMIT << " iterations" << std::endl;
    }
}

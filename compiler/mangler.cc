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
#include <cctype>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

#include "mangler.h"
#include "util.h"

// High water mark and entities.
namespace {

const size_t HIGH_WATER_MARK = 4096 - 32;

typedef std::map<char, std::string> EntityTable;
typedef EntityTable::const_iterator EntityTableIterator;

const EntityTable &
get_entities ()
{
  static EntityTable entities;

  if (entities.empty ())
    {
      entities['`'] = "Backtick";
      entities['~'] = "Tilde";
      entities['!'] = "Store";
      entities['@'] = "Fetch";
      entities['#'] = "Hash";
      entities['$'] = "Dollar";
      entities['%'] = "Percent";
      entities['^'] = "Caret";
      entities['&'] = "Amp";
      entities['*'] = "Star";
      entities['('] = "Paren";
      entities[')'] = "Closeparen";
      entities['-'] = "Minus";
      entities['='] = "Equals";
      entities['+'] = "Plus";
      entities['['] = "Leftbracket";
      entities[']'] = "Rightbracket";
      entities['{'] = "Leftbrace";
      entities['}'] = "Rightbrace";
      entities['|'] = "Or";
      entities[';'] = "Semicolon";
      entities[':'] = "Colon";
      entities['"'] = "Quote";
      entities['\''] = "Tick";
      entities[','] = "Comma";
      entities['.'] = "Dot";
      entities['/'] = "Slash";
      entities['<'] = "Less";
      entities['>'] = "Greater";
      entities['?'] = "Question";
      entities['\\'] = "Backslash";
    }

  return entities;
}

} // namespace

// Low-level character manglers.
namespace {

bool
transform_simple (char c, std::string * mangled)
{
  if (c == '_' || isdigit (c) || isalpha (c))
    {
      mangled->append (mangled->empty () && isdigit (c) ? "z" : "");
      mangled->push_back (c);
      return true;
    }
  return false;
}

bool
transform_entity (char c, std::string * mangled)
{
  const EntityTable & entities = get_entities ();

  const EntityTableIterator iter = entities.find (c);
  if (iter != entities.end ())
    {
      mangled->append (iter->second);
      return true;
    }
  return false;
}

void
transform_hex (char c, std::string * mangled)
{
  std::ostringstream outs;
  outs.fill ('0');
  outs << 'x' << std::hex << std::uppercase << std::setw (2)
       << static_cast<unsigned int>(static_cast<unsigned char>(c));
  mangled->append (outs.str ());
}

void
transform (char c, std::string * mangled)
{
  if (mangled->size () > HIGH_WATER_MARK)
    return;

  if (transform_simple (c, mangled))
    return;

  mangled->append (mangled->empty () ? "" : "_");

  if (transform_entity (c, mangled))
    return;

  transform_hex (c, mangled);
}

} // namespace

// Static name mangler function.  If s starts with "__" do only restricted
// mangling, omitting conversion to lowercase and the added "v4_" prefix.
const std::string
Mangler::mangle (const std::string & s)
{
  if (s == "_start" || s == "main")
    return s;

  if (s == "__")
    return "";

  const bool is_restricted = s.size () > 2 && s.substr (0, 2) == "__";

  std::string m = s;
  if (is_restricted)
    m.erase (m.begin (), m.begin () + 2);
  else
    m = to_lower (m);

  std::string mangled = "";
  std::for_each (m.begin (), m.end (),
                 std::bind2nd (std::ptr_fun (transform), &mangled));

  if (is_restricted)
    return mangled;
  else
    return std::string ("v4_") + mangled;
}

// Low-level demanglers.
namespace {

bool
untransform_hex (std::string * s, std::string * demangled)
{
  static const std::string & hex = "0123456789ABCDEF";

  if (s->size () < 4 || s->substr (0, 2) != "_x")
    return false;

  if (hex.find (s->at (2)) != hex.npos && hex.find (s->at (3)) != hex.npos)
    {
      demangled->append (1, hex.find (s->at (2)) << 4 | hex.find (s->at (3)));
      s->erase (s->begin (), s->begin () + 4);
      return true;
    }
  return false;
}

bool
untransform_entity (std::string * s, std::string * demangled)
{
  const EntityTable & entities = get_entities ();

  for (EntityTableIterator iter = entities.begin ();
       iter != entities.end (); ++iter)
    {
      if (s->find (std::string ("_") + iter->second) == 0)
        {
          demangled->append (1, iter->first);
          s->erase (s->begin (),
                    s->begin () + std::string (iter->second).size () + 1);
          return true;
        }
    }
  return false;
}

void
untransform_simple (std::string * s, std::string * demangled)
{
  const bool is_first_character = demangled->empty ();

  if (is_first_character)
    {
      if (s->size () > 1 && s->at (0) == 'z' && isdigit (s->at (1)))
        s->erase (s->begin ());
    }

  demangled->append (1, s->at (0));
  s->erase (s->begin ());
}

void
untransform (std::string * s, std::string * demangled)
{
  std::string ss = *s;

  ss.insert (0, demangled->empty () ? "_" : "");

  if (untransform_hex (&ss, demangled) || untransform_entity (&ss, demangled))
    {
      s->assign (ss);
      return;
    }

  untransform_simple (s, demangled);
}

} // namespace

// Static name demangler function.  Mangling and demangling are not symmetrical
// operations, so demangle(mangle(s)) may not equal s.
const std::string
Mangler::demangle (const std::string & s)
{
  if (s == "_start" || s == "main")
    return s;

  if (s == "v4_")
    return "";

  if (s.size () < 4 || s.substr (0, 3) != "v4_")
    return std::string ("__") + s;

  std::string ss = s.substr (3);
  std::string demangled;

  while (!ss.empty ())
    untransform (&ss, &demangled);

  return demangled;
}

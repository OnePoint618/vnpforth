%{
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

#include <iomanip>
#include <iostream>
#include <sstream>

#include "parser.h"

#define YYSTYPE char *
#include "forth.tab.hh"

// External definition of active parser for non-OO yacc/lex code.
extern Parser * parser_;

#define RETURN_WITH_LVALUE(X) \
    do { yylval = yytext; return (X); } while (0)

namespace {

std::ostream &
yycharacter (std::ostream & outs)
{
  std::ostringstream ss;
  ss.fill ('0');
  ss << "0x" << std::hex << std::setw (2)
     << static_cast<int>(static_cast<unsigned char>(*yytext)) \
     << std::dec;
  outs << ss.str ();
  return outs;
}

} // namespace

%}

DIGIT  [0-9]
ODIGIT [0-7]
HDIGIT [0-9A-Fa-f]
BDIGIT [0-1]
FCHAR  [!-\377]
SPACE  [ \t\r]

%x code1 code2 code3 comment

%%

<*>"\n"                           parser_->next_line (); REJECT;

[-+]?{DIGIT}+                     RETURN_WITH_LVALUE(INTEGER_LITERAL);
[-+]?0x{HDIGIT}+                  RETURN_WITH_LVALUE(INTEGER_LITERAL);
[-+]?0o{ODIGIT}+                  RETURN_WITH_LVALUE(INTEGER_LITERAL);
[-+]?0b{BDIGIT}+                  RETURN_WITH_LVALUE(INTEGER_LITERAL);

[-+]?&{DIGIT}+                    RETURN_WITH_LVALUE(INTEGER_LITERAL);
[-+]?#{DIGIT}+                    RETURN_WITH_LVALUE(INTEGER_LITERAL);
[-+]?${HDIGIT}+                   RETURN_WITH_LVALUE(INTEGER_LITERAL);
[-+]?%{BDIGIT}+                   RETURN_WITH_LVALUE(INTEGER_LITERAL);

[-+]?{DIGIT}+("."{DIGIT}*)?[Ee][-+]?{DIGIT}* {
                                  RETURN_WITH_LVALUE(FLOAT_LITERAL); }

[-+]?{DIGIT}+"."{DIGIT}*          RETURN_WITH_LVALUE(FLOAT_LITERAL);

'{SPACE}+{FCHAR}+                 RETURN_WITH_LVALUE(OBJECT_ADDRESS);
"["'"]"{SPACE}+{FCHAR}+           RETURN_WITH_LVALUE(OBJECT_ADDRESS);

S\"" "[^\"\n]*\"                  RETURN_WITH_LVALUE(STRING_LITERAL);
C\"" "[^\"\n]*\"                  RETURN_WITH_LVALUE(CSTRING);
"."\"" "[^\"\n]*\"                RETURN_WITH_LVALUE(PSTRING);
ABORT\"" "[^\"\n]*\"              RETURN_WITH_LVALUE(ASTRING);

".("{SPACE}[^)\n]*")"             RETURN_WITH_LVALUE(DOTPARENSTRING);

CHAR{SPACE}+{FCHAR}+              RETURN_WITH_LVALUE(CHAR_LITERAL);
"["CHAR"]"{SPACE}+{FCHAR}+        RETURN_WITH_LVALUE(CHAR_LITERAL);

'{FCHAR}'?                        RETURN_WITH_LVALUE(CHAR_LITERAL);

:NONAME                           RETURN_WITH_LVALUE(NONAME);
:                                 RETURN_WITH_LVALUE(':');
;                                 RETURN_WITH_LVALUE(';');
EXIT                              RETURN_WITH_LVALUE(EXIT);

FVARIABLE                         RETURN_WITH_LVALUE(FVARIABLE);
VARIABLE                          RETURN_WITH_LVALUE(VARIABLE);
CREATE                            RETURN_WITH_LVALUE(CREATE);
ALLOT                             RETURN_WITH_LVALUE(ALLOT);
FCONSTANT                         RETURN_WITH_LVALUE(FCONSTANT);
CONSTANT                          RETURN_WITH_LVALUE(CONSTANT);

IF                                RETURN_WITH_LVALUE(IF);
ELSE                              RETURN_WITH_LVALUE(ELSE);
THEN                              RETURN_WITH_LVALUE(THEN);
ENDIF                             RETURN_WITH_LVALUE(ENDIF);

DO                                RETURN_WITH_LVALUE(DO);
"?"DO                             RETURN_WITH_LVALUE(QUERY_DO);
LOOP                              RETURN_WITH_LVALUE(LOOP);
"+"LOOP                           RETURN_WITH_LVALUE(PLUS_LOOP);
LEAVE                             RETURN_WITH_LVALUE(LEAVE);
"?"LEAVE                          RETURN_WITH_LVALUE(QUERY_LEAVE);

BEGIN                             RETURN_WITH_LVALUE(BEGIN_);
UNTIL                             RETURN_WITH_LVALUE(UNTIL);
AGAIN                             RETURN_WITH_LVALUE(AGAIN);
WHILE                             RETURN_WITH_LVALUE(WHILE);
REPEAT                            RETURN_WITH_LVALUE(REPEAT);

CASE                              RETURN_WITH_LVALUE(CASE);
OF                                RETURN_WITH_LVALUE(OF);
ENDOF                             RETURN_WITH_LVALUE(ENDOF);
ENDCASE                           RETURN_WITH_LVALUE(ENDCASE);

RECURSE                           RETURN_WITH_LVALUE(RECURSE);

"["IFDEF"]"                       RETURN_WITH_LVALUE(COND_IFDEF);
"["IFUNDEF"]"                     RETURN_WITH_LVALUE(COND_IFUNDEF);
"["ELSE"]"                        RETURN_WITH_LVALUE(COND_ELSE);
"["THEN"]"                        RETURN_WITH_LVALUE(COND_THEN);
"["ENDIF"]"                       RETURN_WITH_LVALUE(COND_ENDIF);

CODE                              BEGIN(code1); RETURN_WITH_LVALUE(CODE);
<code1>{
  {SPACE}+
  {FCHAR}+                        BEGIN(code2); RETURN_WITH_LVALUE(WORD);
}
<code2>{
  {SPACE}+
  "\\"{SPACE}[^\n]*\n             BEGIN(code3); parser_->next_line ();
  "\\"\n                          BEGIN(code3); parser_->next_line ();
  "("{SPACE}[^)]*")"
  "\n"                            BEGIN(code3);
  .                               { BEGIN(INITIAL);
                                    parser_->parse_error ()
                                      << "expected newline here" << std::endl; }
}
<code3>{
  "\n"
  END-CODE                        BEGIN(INITIAL); RETURN_WITH_LVALUE(END_CODE);
  [^\r\n]+                        RETURN_WITH_LVALUE(ASM);
}
<code1,code2,code3>\r
<code1,code2,code3>.              { BEGIN(INITIAL);
                                    parser_->parse_error ()
                                        << "unknown character '"
                                        << yycharacter <<  "'" << std::endl; }
<code1,code2,code3><<EOF>>        { BEGIN(INITIAL);
                                    parser_->parse_error ()
                                        << "unexpected EOF in CODE...ENDCODE"
                                        << std::endl;
                                    yyterminate (); }

{SPACE}+
"\\"\n                            parser_->next_line ();
"\\"{SPACE}[^\n]*\n               parser_->next_line ();
"("{SPACE}                        BEGIN(comment);
<comment>{
  ")"                             BEGIN(INITIAL);
  "\n"
  .
}
<comment><<EOF>>                  { BEGIN(INITIAL);
                                    parser_->parse_error ()
                                        << "unexpected EOF in comment"
                                        << std::endl;
                                    yyterminate (); }
"\n"

{FCHAR}+                          RETURN_WITH_LVALUE(WORD);

.                                 { parser_->parse_error ()
                                        << "unknown character '"
                                        << yycharacter <<  "'" << std::endl; }

%%

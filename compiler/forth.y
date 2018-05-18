%{
// vi: set ts=2 shiftwidth=2 expandtab:
//
// VNPForth - Compiled native Forth for x86 Linux
// Copyright (C) 2005-2013 Simon Baldwin (simon_baldwin@yahoo.com)
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
//

#include <iostream>

#include "parser.h"

#define YYSTYPE char *
#define YYERROR_VERBOSE 1
extern int yylex ();

// External definition of active parser for non-OO yacc/lex code.
extern Parser * parser_;

// Macro to cause yyparse to abort if the maximum number of allowable errors
// has been exceeded.  This is an icky thing to have to do, but the closed
// nature of yyparse seems to leave little option - even a setjmp/longjmp is
// not an improvement.  And it doesn't even trigger until we hit a valid
// token, so we may well have a lot more errors than the maximum allowed by
// the time that happens.  All in all, rather unpleasant.
#define MNE                                               \
  do {                                                    \
    if (parser_->excessive_errors ())                     \
      {                                                   \
        parser_->parse_error ()                           \
            << "too many errors, giving up" << std::endl; \
        YYABORT;                                          \
      }                                                   \
  } while (0)

// Macro to conditionally suppress calling the parser terminal handler when
// conditional processing indicates that the code being parsed is inactive.
#define P(TERMINAL_HANDLER)                               \
  do {                                                    \
    if (parser_->cond_state ())                           \
      {                                                   \
        TERMINAL_HANDLER;                                 \
      }                                                   \
  } while (0)

// yyerror function links to the parser's error reporting.
namespace {

void yyerror (const char * message)
{
  parser_->parse_error () << message << std::endl;
}

} // namespace

%}

%debug
%token VARIABLE FVARIABLE CONSTANT FCONSTANT
%token CREATE ALLOT
%token NONAME EXIT
%token IF THEN ENDIF ELSE
%token DO QUERY_DO "?DO" LOOP PLUS_LOOP "+LOOP" LEAVE QUERY_LEAVE "?LEAVE"
%token BEGIN_ "BEGIN" UNTIL WHILE REPEAT AGAIN
%token CASE OF ENDOF ENDCASE
%token RECURSE
%token PSTRING "print string"
%token CSTRING "counted string"
%token ASTRING "abort string"
%token DOTPARENSTRING "compilation comment"
%token INTEGER_LITERAL "integer"
%token STRING_LITERAL "literal string"
%token FLOAT_LITERAL "float"
%token CHAR_LITERAL "char literal"
%token OBJECT_ADDRESS "address"
%token WORD "word"
%token CODE ASM "assembler string" END_CODE "END-CODE"
%token COND_IFDEF "[IFDEF]" COND_IFUNDEF "[IFUNDEF]"
%token COND_ELSE "[ELSE]" COND_THEN "[THEN]" COND_ENDIF "[ENDIF]"
%token END 0 "end of file"

%%
program:          elements END { MNE; parser_->f_end_of_file (); }
                ;

elements:
                | elements element
                ;

element:          definition
                | declaration
                | statement
                | error WORD { MNE; yyerrok; }
                ;

declaration:      VARIABLE WORD { MNE; P(parser_->f_definedvariable ($2)); }
                | FVARIABLE WORD { MNE; P(parser_->f_definedvariable ($2)); }
                | CONSTANT WORD { MNE; P(parser_->f_definedconstant ($2)); }
                | FCONSTANT WORD { MNE; P(parser_->f_definedfconstant ($2)); }
                | CREATE WORD INTEGER_LITERAL WORD ALLOT { MNE;
                                      P(parser_->f_definedarray ($2, $3, $4)); }
                | CREATE WORD INTEGER_LITERAL ALLOT { MNE;
                                      P(parser_->f_definedarray ($2, $3)); }
                ;

statements:
                | statements statement
                ;

definition:       ':' definedword statements semicolon_
                | noname statements semicolon_
                | CODE codeword codes end_code_
                ;
semicolon_:       ';' { MNE; P(parser_->f_semicolon ()); }
                ;
definedword:      WORD { MNE; P(parser_->f_definedword ($1)); }
                ;
noname:           NONAME { MNE; P(parser_->f_nonameword ()); }
                ;
codeword:         WORD { MNE; P(parser_->f_codeword ($1)); }
                ;
codes:            code
                | codes code
                ;
code:             ASM { MNE; P(parser_->f_code ($1)); }
                ;
end_code_:        END_CODE { MNE; P(parser_->f_end_code ()); }
                ;

statement:        conditional
                | if_statement
                | do_loop
                | do_leave
                | begin_loop
                | begin_while
                | case_statement
                | recurse
                | EXIT { MNE; P(parser_->f_exit ()); }
                | PSTRING { MNE; P(parser_->f_pstring ($1)); }
                | CSTRING { MNE; P(parser_->f_cstring ($1)); }
                | ASTRING { MNE; P(parser_->f_astring ($1)); }
                | DOTPARENSTRING { MNE; P(parser_->f_dotparenstring ($1)); }
                | INTEGER_LITERAL { MNE; P(parser_->f_integer_literal ($1)); }
                | FLOAT_LITERAL { MNE; P(parser_->f_float_literal ($1)); }
                | STRING_LITERAL { MNE; P(parser_->f_string_literal ($1)); }
                | CHAR_LITERAL { MNE; P(parser_->f_char_literal ($1)); }
                | OBJECT_ADDRESS { MNE; P(parser_->f_object_address ($1)); }
                | ALLOT { MNE; P(parser_->f_word ($1)); }
                | WORD  { MNE; P(parser_->f_word ($1)); }
                ;

conditional:      COND_IFDEF WORD { MNE; parser_->f_cond_ifdef ($1, false); }
                | COND_IFUNDEF WORD { MNE; parser_->f_cond_ifdef ($1, true); }
                | COND_THEN { MNE; parser_->f_cond_then_endif (false); }
                | COND_ENDIF { MNE; parser_->f_cond_then_endif (true); }
                | COND_ELSE { MNE; parser_->f_cond_else (); }
                ;

if_statement:     if_ statements then_endif_
                | if_ statements else_ statements then_endif_
                ;
if_:              IF { MNE; P(parser_->f_if ()); }
                ;
then_endif_:      THEN { MNE; P(parser_->f_then_endif ()); }
                | ENDIF { MNE; P(parser_->f_then_endif ()); }
                ;
else_:            ELSE { MNE; P(parser_->f_else ()); }
                ;

do_loop:          do_ statements loop_
                | do_ statements plus_loop_
                ;
do_:              DO { MNE; P(parser_->f_do ()); }
                | QUERY_DO { MNE; P(parser_->f_query_do ()); }
                ;
loop_:            LOOP { MNE; P(parser_->f_loop ()); }
                ;
plus_loop_:       PLUS_LOOP { MNE; P(parser_->f_plus_loop ()); }
                ;
do_leave:         LEAVE { MNE; P(parser_->f_leave ()); }
                | QUERY_LEAVE { MNE; P(parser_->f_query_leave ()); }
                ;

begin_loop:       begin_ statements until_
                | begin_ statements repeat_again_
                ;
begin_:           BEGIN_ { MNE; P(parser_->f_begin ()); }
                ;
until_:           UNTIL { MNE; P(parser_->f_until ()); }
                ;
repeat_again_:    REPEAT { MNE; P(parser_->f_repeat_again ()); }
                | AGAIN { MNE; P(parser_->f_repeat_again ()); }
                ;
begin_while:      WHILE { MNE; P(parser_->f_while ()); }
                ;

case_statement:   case_ casebody statements endcase_
                ;
casebody:         caseclause
                | casebody caseclause
                ;
caseclause:       statements of_ statements endof_
                ;
case_:            CASE { MNE; P(parser_->f_case ()); }
                ;
endcase_:         ENDCASE { MNE; P(parser_->f_endcase ()); }
                ;
of_:              OF { MNE; P(parser_->f_of ()); }
                ;
endof_:           ENDOF { MNE; P(parser_->f_endof ()); }
                ;

recurse:          RECURSE { MNE; P(parser_->f_recurse ()); }
                ;
%%
